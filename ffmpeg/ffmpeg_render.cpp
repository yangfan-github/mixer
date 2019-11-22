#include "ffmpeg_render.h"

void get_pps_sps(uint8_t* buf, uint32_t size, bool is264, uint32_t& offset, uint32_t& len)
{
    len = 0;
    struct NAL
    {
        uint32_t offfset;
        uint32_t len;
    };
    NAL _nals[1024];
    if (size < 8 || buf == NULL)
        return;
    uint32_t idx = 0;

    bool foundFirstStartCode = false;
    memset(_nals, 0, sizeof(_nals));
    for (uint32_t i = 4; i < size && idx < sizeof(_nals)/sizeof(NAL); ++i)
    {
        if (0x01 == buf[i-1] && 0x00 == buf[i-2] && 0x00 == buf[i-3])
        {
            if (false == foundFirstStartCode)
            {
                foundFirstStartCode = true;
                _nals[idx].offfset = i;
                idx++;
                continue;
            }
            _nals[idx].offfset = i;
            _nals[idx-1].len = i - _nals[idx-1].offfset - (buf[i-4] == 0 ?4:3);
            ++idx;
        }
    }
    if (idx > 0)
        _nals[idx-1].len = size - _nals[idx-1].offfset;

    static const uint8_t AVCPPS = 0x8;
    static const uint8_t AVCSPS = 0x7;
    static const uint8_t HEVCPPS = 34;
    static const uint8_t HEVCSPS = 33;
    static const uint8_t HEVCVPS = 32;
    for (int i = 1; i < 1024 && _nals[i].len != 0; ++i)
    {
        uint8_t pps, sps;
        uint8_t rsh = 0;
        pps = AVCPPS;
        sps = AVCSPS;
        uint8_t mark = 0x1f;
        if (false == is264)
        {
            rsh = 1;
            pps = HEVCPPS;
            sps = HEVCSPS;
            mark = 0x3f;
        }
        if(((buf[_nals[i].offfset] >> rsh) & mark) == pps // pps
                && ((buf[_nals[i-1].offfset] >> rsh) & mark) == sps) // sps
        {
            if (is264 == false)
            {
                 if (i >= 2
                         && ((buf[_nals[i-2].offfset] >> rsh & mark) == HEVCVPS))
                 {
                     uint32_t o = _nals[i].offfset+_nals[i].len;
                     len =  o - _nals[i-2].offfset;
                     offset = _nals[i-2].offfset;
                 }
            }
            else
            {
                uint32_t o = _nals[i].offfset+_nals[i].len;
                len =  o - _nals[i-1].offfset;
                offset = _nals[i-1].offfset;
            }
        }
    }
    if(0 < len)
    {
        if(3 <= offset && 0 == buf[offset-3])
        {
            offset -= 3;
            len += 3;
        }
        else
        {
            offset -= 2;
            len += 2;
        }
    }
}

ffmpeg_render::stream::stream(ffmpeg_render* render)
:input_pin(render)
,_render(render)
,_avstream(nullptr)
,_ctxBSF(nullptr)
{
    memset(&_pkt_out,0,sizeof(_pkt_out));
}

ffmpeg_render::stream::~stream()
{
    close();
}

ret_type ffmpeg_render::stream::deliver(frame_ptr frame)
{
    ret_type rt;
    if(nullptr != frame)
    {
        if(false == _mt->get_global_header() && true == _render->_is_global_header)
        {
            if(AV_CODEC_ID_H264 == _avstream->codec->codec_id || AV_CODEC_ID_HEVC == _avstream->codec->codec_id)
            {
                if(nullptr == _avstream->codec->extradata || 0 == _avstream->codec->extradata_size)
                {
                    uint32_t offset = 0 ,size = 0;
                    uint8_t* buf = (uint8_t*)frame->get_buf();
                    uint32_t len = frame->get_len();
                    get_pps_sps(buf,len,AV_CODEC_ID_H264 == _avstream->codec->codec_id,offset,size);
                    if(0 < size)
                    {
                        JCHK(nullptr != (_avstream->codec->extradata = (uint8_t*)av_realloc(_avstream->codec->extradata,size)),rc_new_fail);
                        memcpy(_avstream->codec->extradata,buf+offset,size);
                        _avstream->codec->extradata_size = size;
                        JIF(_mt->set_extra_data(buf+offset,size))
                    }
                    else
                    {
                        TRACE(dump::warn,FORMAT_STR("ffmpeg render url:%1% stream[%2%] frame[DTS:%3%] can not get extra data",
                            %_render->_ctxFormat->filename
                            %_avstream->index
                            %frame->_info.dts))
                        return rc_ok;
                    }
                }
            }
        }
    }
    JIF(input_pin::deliver(frame))
    return _render->process();
}

ret_type ffmpeg_render::stream::open()
{
    if(false == is_connect())
        return rc_ok;

    if(nullptr == _avstream)
    {
        JCHK(_mt,rc_state_invalid)
        AVCodecID id = (AVCodecID)_mt->get_sub();
        JCHK(AV_CODEC_ID_NONE != id,rc_state_invalid);
        JCHK(_avstream = avformat_new_stream(_render->_ctxFormat,nullptr),rc_fail);
        _avstream->codec->codec_type = (AVMediaType)_mt->get_major();
        _avstream->codec->codec_id = id;
        _avstream->id = _render->_ctxFormat->nb_streams;

        if(AVMEDIA_TYPE_VIDEO == _avstream->codec->codec_type)
        {
            //int64_t duration;
            int ratioX = 0,ratioY = 0;
            _avstream->codec->pix_fmt = (AVPixelFormat)_mt->get_video_format();
            _avstream->codec->width = _mt->get_video_width();
            _avstream->codec->height = _mt->get_video_height();
            ratioX = _mt->get_video_ratioX();
            ratioY = _mt->get_video_ratioY();
            _render->_ctxFormat->oformat->video_codec = _avstream->codec->codec_id;

            if(0 == ratioX)
                ratioX = _avstream->codec->width;
            if(0 == ratioY)
                ratioY = _avstream->codec->height;

            unsigned int g = gcd(ratioX,ratioY);
            ratioX /= g;
            ratioY /= g;

            unsigned int sar_x = ratioX * _avstream->codec->height;
            unsigned int sar_y = ratioY * _avstream->codec->width;

            g = gcd(sar_x,sar_y);
            _avstream->codec->sample_aspect_ratio.num = sar_x/g;
            _avstream->codec->sample_aspect_ratio.den = sar_y/g;

            _avstream->sample_aspect_ratio = _avstream->codec->sample_aspect_ratio;

            _avstream->r_frame_rate.num = FRAME_TIMEBASE.den;
            _avstream->r_frame_rate.den = _mt->get_video_duration();
            _avstream->codec->framerate = _avstream->r_frame_rate;
        }
        else if(AVMEDIA_TYPE_AUDIO == _avstream->codec->codec_type)
        {
            _avstream->codec->sample_fmt = (AVSampleFormat)_mt->get_audio_format();
            _avstream->codec->channels = _mt->get_audio_channel();
            _avstream->codec->channel_layout = _mt->get_audio_channel_layout();
            _avstream->codec->sample_rate = _mt->get_audio_sample_rate();
            _avstream->codec->frame_size = _mt->get_audio_frame_size();

            _render->_ctxFormat->oformat->audio_codec = _avstream->codec->codec_id;
        }
        else
            return rc_param_invalid;

        _avstream->time_base = FRAME_TIMEBASE;
        _avstream->codec->time_base = _avstream->time_base;

        int extra_size = _mt->get_extra_size();
        uint8_t* extra_data = _mt->get_extra_data();

        if(NULL != extra_data && 0 < extra_size)
        {
            JCHK(NULL != (_avstream->codec->extradata = (uint8_t*)av_realloc(_avstream->codec->extradata,extra_size)),rc_new_fail);
            memcpy(_avstream->codec->extradata,extra_data,extra_size);
            _avstream->codec->extradata_size = extra_size;
        }
        else
        {
            if(NULL != _avstream->codec->extradata)
            {
                av_free(_avstream->codec->extradata);
                _avstream->codec->extradata = NULL;
            }
            _avstream->codec->extradata_size = 0;
        }


        if(true == _render->_is_global_header)
        {
            _avstream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
            _avstream->codec->flags2 &= ~AV_CODEC_FLAG2_LOCAL_HEADER;
            if(false == _mt->get_global_header())
            {
                if(AV_CODEC_ID_H264 == _avstream->codec->codec_id)
                {

                }
                else if(AV_CODEC_ID_HEVC == _avstream->codec->codec_id)
                {

                }
                else if(AV_CODEC_ID_AAC == _avstream->codec->codec_id)
                {
                    JCHK(_ctxBSF = av_bitstream_filter_init("aac_adtstoasc"),rc_fail);
                }
            }
        }
        else
        {
            _avstream->codec->flags &= ~CODEC_FLAG_GLOBAL_HEADER;
            _avstream->codec->flags2 |= AV_CODEC_FLAG2_LOCAL_HEADER;

            if(true == _mt->get_global_header())
            {
                if(AV_CODEC_ID_H264 == _avstream->codec->codec_id)
                {
                    JCHK(_ctxBSF = av_bitstream_filter_init("h264_mp4toannexb"),rc_fail);
                }
                else if(AV_CODEC_ID_HEVC == _avstream->codec->codec_id)
                {
                    JCHK(_ctxBSF = av_bitstream_filter_init("hevc_mp4toannexb"),rc_fail);
                }
            }
        }
        memset(&_pkt_out,0,sizeof(_pkt_out));
        _pkt_out.dts = AV_NOPTS_VALUE;
        _pkt_out.pts = AV_NOPTS_VALUE;
    }
    return rc_ok;
}

ret_type ffmpeg_render::stream::convert(AVPacket& pkt,frame_ptr frame)
{
    ret_type rt = rc_ok;
    if(frame)
    {
        JIF(convert_frame_to_packet(pkt,&_avstream->time_base,frame))
        pkt.stream_index = _avstream->index;
        if(pkt.dts <= _pkt_out.dts)
        {
            ++pkt.dts;
            ++pkt.pts;
        }
        _pkt_out.dts = pkt.dts;
        _pkt_out.pts = frame->_info.dts;

        if(nullptr != _ctxBSF)
        {
            if(AVMEDIA_TYPE_VIDEO == _avstream->codec->codec_type)
            {
                if(nullptr != _pkt_out.data)
                {
                    av_free(_pkt_out.data);
                    _pkt_out.data = nullptr;
                }
                _pkt_out.size = 0;

                char err[AV_ERROR_MAX_STRING_SIZE] = {0};
                int ret = av_bitstream_filter_filter(_ctxBSF,
                    _avstream->codec,
                    nullptr,
                    &_pkt_out.data,
                    &_pkt_out.size,
                    pkt.data,
                    pkt.size,
                    pkt.flags & AV_PKT_FLAG_KEY);

                if(0 > ret)
                {
                    _pkt_out.data = nullptr;
                    _pkt_out.size = 0;
                    JCHKM(0 <= ret,rc_fail,FORMAT_STR("ffmpeg render url:%1% stream[%2%] frame[DTS:%3%] add header fail msg:%4%",
                        %_render->_ctxFormat->filename
                        %_avstream->index
                        %pkt.dts
                        %av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))
                }
                else
                {
                    if(_pkt_out.data != pkt.data)
                    {
                        pkt.data = _pkt_out.data;
                        pkt.size = _pkt_out.size;
                    }
                    else
                    {
                        _pkt_out.data = nullptr;
                        _pkt_out.size = 0;
                    }
                }
            }
            else if(AVMEDIA_TYPE_AUDIO == _avstream->codec->codec_type)
            {
                char err[AV_ERROR_MAX_STRING_SIZE] = {0};
                int ret = av_bitstream_filter_filter(_ctxBSF,_avstream->codec,nullptr,
                    &pkt.data,&pkt.size,pkt.data,pkt.size,frame->_info.flag & MEDIA_FRAME_FLAG_SYNCPOINT);
                JCHKM(0 <= ret,rc_fail,FORMAT_STR("ffmpeg render url:%1%  stream[%2%] aac frame[DTS:%3%] romove header fail,msg:%4%",
                    %_render->_ctxFormat->filename
                    %_avstream->index
                    %frame->_info.dts
                    %av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))
            }
        }
    }
    return rt;
}

void ffmpeg_render::stream::close()
{
    if(nullptr != _pkt_out.data)
    {
        av_free(_pkt_out.data);
        _pkt_out.data = nullptr;
    }
    _pkt_out.size = 0;
	if(nullptr != _ctxBSF)
	{
		av_bitstream_filter_close(_ctxBSF);
		_ctxBSF = nullptr;
	}
	if(nullptr != _avstream)
	{
        if(nullptr != _avstream->codec->extradata)
        {
            av_free(_avstream->codec->extradata);
            _avstream->codec->extradata = nullptr;
        }
        _avstream->codec->extradata_size = 0;
        _avstream = nullptr;
	}
}

ffmpeg_render::ffmpeg_render()
:_ctxFormat(nullptr)
,_is_global_header(false)
,_is_image(false)
,_is_header(false)
,_is_eof(false)
,_time(MEDIA_FRAME_NONE_TIMESTAMP)
{
    //ctor
}

ffmpeg_render::~ffmpeg_render()
{
    //dtor
    close();
}
CLS_INFO_DEFINE(media_render,ffmpeg_render,MAKE_VERSION(0,0,0,0))

uint32_t ffmpeg_render::cls_priority(const char* info,media_type* mt_input,media_type* mt_output)
{
    return 1;
}

ret_type ffmpeg_render::set_media_type(input_pin* pin,media_ptr mt)
{
    return rc_ok;
}

input_pin_ptr ffmpeg_render::create_pin(media_ptr mt)
{
    input_pin_ptr pin;
    JCHKR(mt,rc_param_invalid,pin)

    std::shared_ptr<stream> strm(new stream(this));
    JCHKR(strm,rc_new_fail,pin)

    JCHKR(IS_OK(strm->set_media_type(mt)),rc_param_invalid,pin)
    _streams.insert(_streams.end(),strm);
    pin.reset(strm.get(),pin_deleter<input_pin>(shared_from_this()));
    return pin;
}

ret_type ffmpeg_render::open(const string& url)
{
    JCHK(!url.empty(),rc_param_invalid)

    close();

    ret_type rt = rc_ok;

    const char* format = nullptr;

    std::vector<std::string> values;
    if(parse_url(url,values))
    {
        string protocol = values[us_protocol];
        transform(protocol.begin(),protocol.end(),protocol.begin(),::tolower);
        if(protocol == "rtmp")
            format = "flv";
    }

    int ret;
    char err[AV_ERROR_MAX_STRING_SIZE] = {0};
    JCHKM(0 <= (ret = avformat_alloc_output_context2(&_ctxFormat,nullptr,format,url.c_str())),
        rc_param_invalid,FORMAT_STR("ffmpeg muxer create url:[%1%] fail,%2%",%url
        %av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))
    _is_global_header = 0 != (AVFMT_GLOBALHEADER & _ctxFormat->oformat->flags);
    _is_image = 0 == strcmp(_ctxFormat->oformat->name,"image2");
    if(true == _is_image)
    {
        _ctxFormat->oformat->video_codec = av_guess_codec(_ctxFormat->oformat,nullptr,url.c_str(),nullptr,AVMEDIA_TYPE_VIDEO);
    }
    else if(0 == strcmp(_ctxFormat->oformat->name,"flv") ||
        0 == strcmp(_ctxFormat->oformat->name,"mp4") ||
        0 == strcmp(_ctxFormat->oformat->name,"mpegts"))
    {
        _ctxFormat->oformat->video_codec = AV_CODEC_ID_H264;
        _ctxFormat->oformat->audio_codec = AV_CODEC_ID_AAC;
    }

    JCHKM(0 <= (ret = avio_open2(&_ctxFormat->pb,url.c_str(),AVIO_FLAG_WRITE,nullptr,nullptr)),rc_param_invalid,
        FORMAT_STR("ffmpeg muxer connect url:[%1%] fail,msg::%2%",%url.c_str()%av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))
    av_dump_format(_ctxFormat,0,url.c_str(),1);

    for(StreamIt it = _streams.begin() ; it != _streams.end() ; ++it)
    {
        JIF((*it)->open())
    }
    _is_eof = false;
    _url = url;
    return rt;
}

ret_type ffmpeg_render::process()
{
    ret_type rt = rc_ok;
    while(true)
    {
        std::shared_ptr<stream> strm;
        frame_ptr frame;
        for(StreamIt it=_streams.begin() ; it != _streams.end() ; ++it)
        {
            frame_ptr frame_temp;
            std::shared_ptr<stream> strm_temp = *it;
            if(strm_temp->is_connect())
            {
                if(strm_temp->peek(frame_temp))
                {
                    if(!frame || frame_temp->_info.dts < frame->_info.dts)
                    {
                        frame = frame_temp;
                        strm = strm_temp;
                    }
                }
                else if(false == strm_temp->eof())
                    return rt;
            }
        }
        if(frame)
        {
            JIF(write(strm.get(),frame))
            JCHK(strm->pop(),rc_fail);
            _time = frame->_info.dts;
        }
        else
        {
            _is_eof = true;
            break;
        }
    }
    return rt;
}

ret_type ffmpeg_render::write(stream* strm,frame_ptr frame)
{
    ret_type rt;
    if(nullptr == _ctxFormat)
    {
        JIF(open(_url))
    }

    char err[AV_ERROR_MAX_STRING_SIZE] = {0};
    if(false == _is_header)
    {
		int ret = avformat_write_header(_ctxFormat,nullptr);
        JCHKM(0 <= ret,rc_fail,
            FORMAT_STR("ffmpeg muxer write file[%1%] header fail,msg:%2%",
            %_ctxFormat->filename
            %av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))
        _is_header = true;
    }

    AVPacket pkt;
    av_init_packet(&pkt);
    JIF(strm->convert(pkt,frame))

    if(pkt.dts > pkt.pts)
    {
        media_ptr mt = strm->get_media_type();
        TRACE(dump::warn,FORMAT_STR("stream[%1%:%2%-%3%] write packet PTS:%4% < DTS:%5%",
            %strm->_avstream->index%mt->get_major_name()
            %mt->get_sub_name()%pkt.pts%pkt.dts))
        pkt.pts = pkt.dts;
    }

    int ret = av_write_frame(_ctxFormat,&pkt);

    if(0 > ret)
    {
        close();
        JCHKM(0 <= ret,rc_fail,
            FORMAT_STR("ffmpeg muxer stream:%1% write packet DTS:%2% PTS:%3% fail:%4%",
            %pkt.stream_index%frame->_info.dts%frame->_info.pts
            %av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))
    }

    return rt;
}

void ffmpeg_render::close()
{
    if(nullptr != _ctxFormat)
    {
        if(true == _is_header)
        {
            int ret;
            char err[AV_ERROR_MAX_STRING_SIZE] = {0};
            ret = av_write_trailer(_ctxFormat);
            if(0 != ret)
            {
                TRACE(dump::warn,FORMAT_STR("ffmpeg muxer write trailer fail,url:[%1%],msg::%2%",
                    %_ctxFormat->filename%av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))
            }
            _is_header = false;
        }
        if(nullptr != _ctxFormat->pb)
        {
            for(StreamIt it = _streams.begin(); it != _streams.end() ; ++it)
            {
                (*it)->close();
            }
            avio_closep(&_ctxFormat->pb);
        }
        avformat_free_context(_ctxFormat);
        _ctxFormat = nullptr;
        _url.clear();
    }
}

bool ffmpeg_render::is_open()
{
    return !_url.empty();
}

bool ffmpeg_render::is_eof()
{
    return _is_eof;
}

int64_t ffmpeg_render::get_time()
{
    return _time;
}
