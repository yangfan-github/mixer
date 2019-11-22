#include "ffmpeg_source.h"
#define ADTS_HEADER_SIZE 7
#define ADTS_MAX_FRAME_BYTES ((1 << 13) - 1)

ffmpeg_source::stream::stream(media_filter* filter)
:output_pin(filter)
,_source(dynamic_cast<ffmpeg_source*>(filter))
,_stream(nullptr)
,_length(0)
,_start(MEDIA_FRAME_NONE_TIMESTAMP)
,_duration(0)
,_is_global_header(false)
,_ctxBSF(nullptr)
,_adts(nullptr)
,_time_start(MEDIA_FRAME_NONE_TIMESTAMP)
{

}

ffmpeg_source::stream::~stream()
{
    close();
}

ret_type ffmpeg_source::stream::open(AVStream* stream)
{
    JCHK(nullptr != stream,rc_param_invalid)

    ret_type rt = rc_ok;
    media_ptr mt = media_type::create();
    JCHK(mt,rc_new_fail)
    if(stream->time_base.den != 0 && stream->time_base.num != 0)
    {
        if(AV_NOPTS_VALUE != stream->start_time)
            _start = int64_t(stream->start_time * av_q2d(stream->time_base) * 10000000);
        if(AV_NOPTS_VALUE == stream->duration)
        {
            if(MEDIA_FRAME_NONE_TIMESTAMP != _source->_ctxFormat->duration)
            {
                AVRational r = {1,AV_TIME_BASE};
                _length = int64_t(_source->_ctxFormat->duration * av_q2d(r) * 10000000);;
            }
        }
        else
        {
            _length = int64_t(stream->duration * av_q2d(stream->time_base) * 10000000);
        }
        stream->codec->time_base = stream->time_base;
    }
    mt->set_sub((MediaSubType)stream->codec->codec_id);
    mt->set_global_header(true == _source->_is_global_header && 0 < stream->codec->extradata_size);
    JIF(mt->set_extra_data(stream->codec->extradata,stream->codec->extradata_size))

    if(AVMEDIA_TYPE_VIDEO == stream->codec->codec_type)
    {
        mt->set_video_format((VideoMediaType)stream->codec->pix_fmt);

        JCHKM(0 < stream->codec->width && 0 < stream->codec->height,rc_param_invalid,
            FORMAT_STR("url:%1% %2% stream-%3% width:%4%,height:%5% is not valid",
                %_source->_ctxFormat->filename%mt->get_sub_name()
                %stream->codec->width%stream->codec->height))

        mt->set_video_width(stream->codec->width);
        mt->set_video_height(stream->codec->height);

        int ratioX,ratioY;
        if(0 == stream->codec->sample_aspect_ratio.num || 0 == stream->codec->sample_aspect_ratio.den)
        {
            if(0 == stream->sample_aspect_ratio.num || 0 == stream->sample_aspect_ratio.den)
            {
                ratioX = stream->codec->width;
                ratioY = stream->codec->height;
            }
            else
            {
                ratioX = stream->sample_aspect_ratio.num * stream->codec->width;
                ratioY = stream->sample_aspect_ratio.den * stream->codec->height;
            }
        }
        else
        {
            ratioX = stream->codec->sample_aspect_ratio.num * stream->codec->width;
            ratioY = stream->codec->sample_aspect_ratio.den * stream->codec->height;
        }

        unsigned int g = gcd(ratioX,ratioY);
        mt->set_video_ratioX(ratioX/g);
        mt->set_video_ratioY(ratioY/g);

        if(stream->r_frame_rate.den != 0 && stream->r_frame_rate.num != 0)
        {
            _duration = int64_t(ONE_SECOND_UNIT/av_q2d(stream->r_frame_rate)+0.5);
        }
        else if(stream->codec->framerate.den != 0 && stream->codec->framerate.num != 0)
        {
            _duration = int64_t(ONE_SECOND_UNIT/av_q2d(stream->codec->framerate)+0.5);
        }
        else if(stream->avg_frame_rate.den != 0 && stream->avg_frame_rate.num != 0)
        {
            _duration = int64_t(ONE_SECOND_UNIT/av_q2d(stream->avg_frame_rate)+0.5);
        }
        else
            _duration = 0;

        if(0 < _duration)
            mt->set_video_fps((double)ONE_SECOND_UNIT/_duration);
        else
            mt->set_video_fps(0.0);
    }
    else if(AVMEDIA_TYPE_AUDIO == stream->codec->codec_type)
    {
        mt->set_audio_format((AudioMediaType)stream->codec->sample_fmt);
        mt->set_audio_sample_rate(stream->codec->sample_rate);
        if(0 < stream->codec->channel_layout || 0 < stream->codec->channels)
        {
            if(0 < stream->codec->channel_layout)
                stream->codec->channels = av_get_channel_layout_nb_channels(stream->codec->channel_layout);
            else
                stream->codec->channel_layout = av_get_default_channel_layout(stream->codec->channels);
        }
        mt->set_audio_channel(stream->codec->channels);
        if(AV_CODEC_ID_WMAPRO == stream->codec->codec_id)
            stream->codec->frame_size = 8192;
        mt->set_audio_frame_size(stream->codec->frame_size);

        _duration = mt->get_audio_duration();
    }
    _stream = stream;
    return set_media_type(mt);
}

ret_type ffmpeg_source::stream::process(AVPacket& pkt)
{
    ret_type rt = rc_ok;
    if(is_connect())
    {
        JCHK(nullptr != pkt.data,rc_param_invalid)
        JCHK(0 < pkt.size,rc_param_invalid)

        if(true == _is_global_header && false == _mt->get_global_header())
        {
            if(AV_CODEC_ID_H264 == _stream->codec->codec_id)
            {
                if(nullptr == _ctxBSF)
                    JCHK(_ctxBSF = av_bitstream_filter_init("h264_mp4toannexb"),rc_fail);
            }
            else if(AV_CODEC_ID_HEVC == _stream->codec->codec_id)
            {
                if(nullptr == _ctxBSF)
                    JCHK(_ctxBSF = av_bitstream_filter_init("hevc_mp4toannexb"),rc_fail);
            }
            else if(AV_CODEC_ID_AAC == _stream->codec->codec_id)
            {
                if(nullptr == _adts)
                {
                    JIF(adts_decode_extradata(_stream->codec->extradata,_stream->codec->extradata_size))
                }
            }
        }
        frame_ptr frame = media_frame::create();
        JCHK(frame,rc_new_fail)
        if(nullptr != _ctxBSF)
        {
            AVPacket pktOut;
            av_init_packet(&pktOut);
            JCHK(0 == av_packet_copy_props(&pktOut, &pkt),rc_fail)
            pktOut.data = nullptr;
            pktOut.size = 0;
            char err[AV_ERROR_MAX_STRING_SIZE] = {0};
            int ret;
            JCHKM(0 <= (ret=av_bitstream_filter_filter(_ctxBSF,
                _stream->codec,
                nullptr,
                &pktOut.data,
                &pktOut.size,
                pkt.data,
                pkt.size,
                pkt.flags & AV_PKT_FLAG_KEY)),rc_fail,
                FORMAT_STR("ffmpeg demux stream[%1%] frame[DTS:%2%] add header fail msg:%3%",
                    %_stream->index%pkt.dts%av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))

            JIF(convert_packet_to_frame(frame,pktOut,_stream->time_base))
        }
        else if(nullptr != _adts)
        {
            int header_size = ADTS_HEADER_SIZE + _adts->pce_size;
            JIF(convert_packet_to_frame(frame,pkt,_stream->time_base,header_size))
            adts_write_frame_header((uint8_t*)frame->get_buf(),frame->get_len());
        }
        else
        {
            JIF(convert_packet_to_frame(frame,pkt,_stream->time_base))
        }

        //TRACE(dump::info,FORMAT_STR("source demux %1% frame dts:%2%",%_mt->get_major_name()%frame->_info.dts))
        JIF(deliver(frame))
    }
    return rt;
}

void ffmpeg_source::stream::close()
{
     if(nullptr != _adts)
    {
        delete _adts;
        _adts = nullptr;
    }
    if(nullptr != _ctxBSF)
    {
        av_bitstream_filter_close(_ctxBSF);
        _ctxBSF = nullptr;
    }
    _stream = nullptr;
   _source = nullptr;
}

ret_type ffmpeg_source::stream::adts_decode_extradata(const uint8_t *buf, size_t size)
{
    GetBitContext gb;
    PutBitContext pb;
    MPEG4AudioConfig m4ac;
	memset(&m4ac,0,sizeof(m4ac));
    int off;

    init_get_bits(&gb, buf, size * 8);
    JCHK(0 <= (off = avpriv_mpeg4audio_get_config(&m4ac, buf, size * 8, 1)),rc_param_invalid);

    skip_bits_long(&gb, off);
    if(NULL == _adts)
        _adts = new ADTSContext();
    memset(_adts,0,sizeof(ADTSContext));
    _adts->objecttype        = m4ac.object_type - 1;
    _adts->sample_rate_index = m4ac.sampling_index;
    _adts->channel_conf      = m4ac.chan_config;

    JCHKM(3U >= _adts->objecttype,rc_param_invalid,FORMAT_STR("MPEG-4 AOT %1% is not allowed in ADTS",%(_adts->objecttype+1)));
    JCHKM(15 != _adts->sample_rate_index,rc_param_invalid,"Escape sample rate index illegal in ADTS");
    JCHKM(0 == get_bits(&gb, 1),rc_param_invalid,"960/120 MDCT window is not allowed in ADTS");
    JCHKM(0 == get_bits(&gb, 1),rc_param_invalid,"Scalable configurations are not allowed in ADTS");
    JCHKM(0 == get_bits(&gb, 1),rc_param_invalid,"Extension flag is not allowed in ADTS");

    if (!_adts->channel_conf) {
        init_put_bits(&pb, _adts->pce_data, MAX_PCE_SIZE);

        put_bits(&pb, 3, 5); //ID_PCE
        _adts->pce_size = (avpriv_copy_pce_data(&pb, &gb) + 3) / 8;
        flush_put_bits(&pb);
    }

    _adts->write_adts = 1;

    return rc_ok;
}

size_t ffmpeg_source::stream::adts_write_frame_header(uint8_t *buf,size_t size)
{
	PutBitContext pb;
	init_put_bits(&pb, buf, ADTS_HEADER_SIZE);

	/* adts_fixed_header */
	put_bits(&pb, 12, 0xfff);   /* syncword */
	put_bits(&pb, 1, 0);        /* ID */
	put_bits(&pb, 2, 0);        /* layer */
	put_bits(&pb, 1, 1);        /* protection_absent */
	put_bits(&pb, 2, _adts->objecttype); /* profile_objecttype */
	put_bits(&pb, 4, _adts->sample_rate_index);
	put_bits(&pb, 1, 0);        /* private_bit */
	put_bits(&pb, 3, _adts->channel_conf); /* channel_configuration */
	put_bits(&pb, 1, 0);        /* original_copy */
	put_bits(&pb, 1, 0);        /* home */

	/* adts_variable_header */
	put_bits(&pb, 1, 0);        /* copyright_identification_bit */
	put_bits(&pb, 1, 0);        /* copyright_identification_start */
	put_bits(&pb, 13, size); /* aac_frame_length */
	put_bits(&pb, 11, 0x7ff);   /* adts_buffer_fullness */
	put_bits(&pb, 2, 0);        /* number_of_raw_data_blocks_in_frame */
	flush_put_bits(&pb);

	if(0 < _adts->pce_size)
	{
		memcpy(buf+ADTS_HEADER_SIZE,_adts->pce_data,_adts->pce_size);
	}
	return ADTS_HEADER_SIZE + _adts->pce_size;
}

ffmpeg_source::ffmpeg_source()
:_ctxFormat(nullptr)
,_is_global_header(false)
,_eof(false)
,_time(MEDIA_FRAME_NONE_TIMESTAMP)
,_time_base(MEDIA_FRAME_NONE_TIMESTAMP)
,_time_start(MEDIA_FRAME_NONE_TIMESTAMP)
,_time_begin(MEDIA_FRAME_NONE_TIMESTAMP)
,_time_delta(0)
,_is_live(false)
{
    //ctor
}

ffmpeg_source::~ffmpeg_source()
{
    //dtor
    close();
}

CLS_INFO_DEFINE(media_source,ffmpeg_source,MAKE_VERSION(0,0,0,0))

uint32_t ffmpeg_source::cls_priority(const char* info,media_type* mt_input,media_type* mt_output)
{
    return 1;
}

ret_type ffmpeg_source::set_media_type(output_pin* pin,media_ptr mt)
{
    return pin->get_media_type() ? rc_state_invalid : rc_ok;
}

ret_type ffmpeg_source::deliver(output_pin* pin,frame_ptr frame)
{
    if(!frame)
        return rc_ok;
    if(frame->_info.dts > _time)
    {
        if(MEDIA_FRAME_NONE_TIMESTAMP == _time)
        {
            if(MEDIA_FRAME_NONE_TIMESTAMP == _time_start)
                _time_start = frame->_info.dts;

            _time_delta = 0;
            if(_is_live)
            {
                if(MEDIA_FRAME_NONE_TIMESTAMP == _time_begin)
                    _time_begin = get_local_time();
                else
                    _time_delta = get_local_time() - _time_begin;
            }
            if(MEDIA_FRAME_NONE_TIMESTAMP != _time_base)
                _time_delta += _time_base - _time_start;
        }
        _time = frame->_info.dts;
    }
    frame->_info.dts += _time_delta;
    frame->_info.pts += _time_delta;
    return rc_ok;
}

ret_type ffmpeg_source::process()
{
    std::unique_lock<std::mutex> lck(_mt_process);
    if(false == _eof)
    {
        JCHK(_ctxFormat,rc_state_invalid)

        AVPacket pkt;
        av_init_packet(&pkt);
        int ret = av_read_frame(_ctxFormat, &pkt);
        if(0 <= ret)
        {
            if(pkt.stream_index < (int)_pins.size())
            {
                dynamic_cast<stream*>(_pins[pkt.stream_index].get())->process(pkt);
            }
            av_packet_unref(&pkt);
        }
        else
        {
            if(AVERROR_EOF == ret || -5 == ret)
                _eof = true;
        }
    }
    if(true == _eof)
    {
        for(size_t i = 0 ; i < _pins.size() ; ++i)
            _pins[i]->deliver(nullptr);
    }
	return rc_ok;
}

output_pin_ptr ffmpeg_source::get_pin(uint32_t index)
{
    output_pin_ptr pin;
    if(index < _pins.size())
        pin.reset(_pins.at(index).get(),pin_deleter<output_pin>(shared_from_this()));
    return pin;
}

ret_type ffmpeg_source::open(const string& url)
{
    JCHK(false == url.empty(),rc_param_invalid)
    if(nullptr != _ctxFormat)
        close();

	char err[AV_ERROR_MAX_STRING_SIZE] = {0};
    AVFormatContext* ctxFormat;
	JCHK(ctxFormat = avformat_alloc_context(),rc_new_fail);

	ctxFormat->interrupt_callback.callback = timeout_callback;
    ctxFormat->interrupt_callback.opaque = this;

	int hr;
    JCHKM(0 <= (hr = avformat_open_input(&ctxFormat,url.c_str(),NULL,NULL)),
            rc_fail,FORMAT_STR("ffmpeg demuxer open url[%1%] fail,message:%2%",
            %url%av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr)));

	JCHKM(0 <= (hr = avformat_find_stream_info(ctxFormat,NULL)),
        rc_fail,FORMAT_STR("ffmpeg demuxer open url[%1%] find stream fail,message:%2%",
        %url%av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,hr)));

	JCHKM(0 < ctxFormat->nb_streams,rc_fail,
        FORMAT_STR("ffmpeg demuxer can't find stream in url[%1%]",%url));

    _ctxFormat = ctxFormat;
	av_dump_format(_ctxFormat, 0, "", 0);
    if(0 == strcmp(_ctxFormat->iformat->name,"mov,mp4,m4a,3gp,3g2,mj2") ||
        0 == strcmp(_ctxFormat->iformat->name,"flv") ||
        0 == strcmp(_ctxFormat->iformat->name,"matroska,webm") ||
        0 == strcmp(_ctxFormat->iformat->name,"3gp"))
        _is_global_header = true;
    else
        _is_global_header = false;

    _is_live = false;

    std::vector<std::string> values;
    if(parse_url(url,values))
    {
        string protocol = values[us_protocol];
        transform(protocol.begin(),protocol.end(),protocol.begin(),::tolower);
        if(protocol == "rtmp")
            _is_live = true;
    }

    ret_type rt = rc_ok;
    _pins.resize(_ctxFormat->nb_streams);
    for(size_t i = 0 ; i < _ctxFormat->nb_streams ; ++i)
    {
        std::shared_ptr<stream> strm(new stream(this));
        JCHK(strm,rc_new_fail)
        JIF(strm->open(_ctxFormat->streams[i]))
        _pins[i] = strm;
    }
    reset();
    return rt;
}

void ffmpeg_source::exit()
{
    _eof = true;
}

void ffmpeg_source::close()
{
    if(nullptr != _ctxFormat)
        avformat_close_input(&_ctxFormat);
    _pins.clear();
}

void ffmpeg_source::set_base(int64_t time_base)
{
    _time_base = time_base;
}

bool ffmpeg_source::is_open()
{
    return _ctxFormat != nullptr;
}

bool ffmpeg_source::is_eof()
{
    return _eof;
}

void ffmpeg_source::reset()
{
    for(size_t i = 0 ; i < _pins.size() ; ++i)
        _pins[i]->new_segment();
    _time = MEDIA_FRAME_NONE_TIMESTAMP;
    _time_start = MEDIA_FRAME_NONE_TIMESTAMP;
    _time_begin = MEDIA_FRAME_NONE_TIMESTAMP;
    _eof = false;
}

int ffmpeg_source::timeout_callback(void *param)
{
    //ffmpeg_source* source = (ffmpeg_source*)param;
    return 0;
}
