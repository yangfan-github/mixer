#include "ffmpeg_video_encoder.h"

ffmpeg_video_encoder::ffmpeg_video_encoder()
:_ctxCodec(nullptr)
{
}

ffmpeg_video_encoder::~ffmpeg_video_encoder()
{
    //dtor
    close();
}

CLS_INFO_DEFINE(media_transform,ffmpeg_video_encoder,MAKE_VERSION(0,0,0,0))

uint32_t ffmpeg_video_encoder::cls_priority(const char* info,media_type* mt_input,media_type* mt_output)
{
    if(nullptr == mt_output || nullptr != mt_input)
        return 0;

    if(MMT_VIDEO != mt_output->get_major() || false == mt_output->is_compress())
        return 0;

    AVCodec* codec = avcodec_find_encoder((AVCodecID)mt_output->get_sub());
    if(nullptr == codec)
        return 0;

    return 1;
}

ret_type ffmpeg_video_encoder::set_media_type(input_pin* pin,media_ptr mt)
{
    media_ptr  mt_in = pin->get_media_type();
    if(mt_in)
    {
        JCHK(media_type::compare(mt,mt_in),rc_param_invalid)
    }
    return rc_ok;
}

ret_type ffmpeg_video_encoder::set_media_type(output_pin* pin,media_ptr mt)
{
    if(mt && true == mt->is_compress())
    {
        ret_type rt;
        JIF(open(mt.get()))
        media_ptr mt_in = media_type::create();
        JCHK(mt_in,rc_new_fail)
        JIF(media_type::copy(mt_in,mt))
        mt_in->set_sub(MST_RAWVIDEO);
        mt_in->set_global_header(false);
        mt_in->set_extra_data(nullptr,0);
        mt_in->set_bitrate(0);
        return _pin_input->set_media_type(mt_in);
    }
    else
    {
        close();
        _pin_input->disconnect();
        return rc_ok;
    }
}

ret_type ffmpeg_video_encoder::open(media_type* mt)
{
    close();
    AVCodec* codec;
    JCHKM(codec = avcodec_find_encoder((AVCodecID)mt->get_sub()),rc_param_invalid,FORMAT_STR("media[%1%] can not find encoder",%mt->get_sub_name()));
    JCHK(_ctxCodec = avcodec_alloc_context3(codec),rc_fail)

    VideoMediaType vmt = mt->get_video_format();
    if(NULL != codec->pix_fmts)
    {
        int i=0;
        while(AV_PIX_FMT_NONE != codec->pix_fmts[i] && (AVPixelFormat)vmt != codec->pix_fmts[i]){++i;}
        if(AV_PIX_FMT_NONE == codec->pix_fmts[i])
        {
            vmt = (VideoMediaType)codec->pix_fmts[0];
            mt->set_video_format(vmt);
        }
        _ctxCodec->pix_fmt = (AVPixelFormat)vmt;
    }
    else if(vmt != (VideoMediaType)_ctxCodec->pix_fmt)
    {
        if(AV_PIX_FMT_NONE == _ctxCodec->pix_fmt)
        {
            _ctxCodec->pix_fmt = (AVPixelFormat)vmt;
        }
        else
        {
            vmt = (VideoMediaType)_ctxCodec->pix_fmt;
            mt->set_video_format(vmt);
        }
    }
    else if(VMT_NONE >= vmt || VMT_NB <= vmt)
    {
        vmt = VMT_YUV420P;
        mt->set_video_format(vmt);
        _ctxCodec->pix_fmt = (AVPixelFormat)vmt;
    }

    JCHK(media_type::MIN_VIDEO_WIDTH <= (_ctxCodec->width = mt->get_video_width()),rc_param_invalid);
    JCHK(media_type::MIN_VIDEO_HEIGHT <= (_ctxCodec->height = mt->get_video_height()),rc_param_invalid);

    int64_t duration = mt->get_video_duration();
    JCHK(0 < duration,rc_param_invalid)

    _ctxCodec->framerate.num = ONE_SECOND_UNIT;
    _ctxCodec->framerate.den = duration;
    _ctxCodec->time_base.num = duration;
    _ctxCodec->time_base.den = ONE_SECOND_UNIT;

    int ratioX,ratioY;
    JCHK(0 < (ratioX = mt->get_video_ratioX()),rc_param_invalid)
    JCHK(0 < (ratioY = mt->get_video_ratioY()),rc_param_invalid)

    _ctxCodec->sample_aspect_ratio.num = ratioX * _ctxCodec->height;
    _ctxCodec->sample_aspect_ratio.den = ratioY * _ctxCodec->width;

    int g = gcd(_ctxCodec->sample_aspect_ratio.num,_ctxCodec->sample_aspect_ratio.den);
    if(1 < g)
    {
        _ctxCodec->sample_aspect_ratio.num /= g;
        _ctxCodec->sample_aspect_ratio.den /= g;
    }

    _ctxCodec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    _ctxCodec->flags2 &= ~AV_CODEC_FLAG2_LOCAL_HEADER;

    JCHK(0 < (_ctxCodec->bit_rate = mt->get_bitrate()),rc_param_invalid)
    if(AV_CODEC_ID_MJPEG != _ctxCodec->codec_id)
    {
        _ctxCodec->rc_min_rate = _ctxCodec->bit_rate;
        _ctxCodec->rc_max_rate = _ctxCodec->bit_rate;
        _ctxCodec->bit_rate_tolerance = _ctxCodec->bit_rate;
        _ctxCodec->rc_buffer_size = _ctxCodec->bit_rate;
        _ctxCodec->rc_initial_buffer_occupancy = _ctxCodec->rc_buffer_size*3/4;
    }

    int ret;
    char err[AV_ERROR_MAX_STRING_SIZE] = {0};
    _ctxCodec->thread_count = 0;
    JCHKM(0 == (ret = avcodec_open2(_ctxCodec,codec,NULL)),rc_fail,
            FORMAT_STR("Open media[%1%] encoder fail msg:%2%",
            %mt->get_sub_name()%av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))

    property_tree::ptree pt = mt->get_codec_option();
    get_option(_ctxCodec->priv_data,pt);

    return mt->set_extra_data(_ctxCodec->extradata,_ctxCodec->extradata_size);
}

ret_type ffmpeg_video_encoder::process(input_pin* pin,frame_ptr frame)
{
    JCHK(nullptr != _ctxCodec,rc_state_invalid)

	ret_type rt = rc_ok;

	AVFrame avframe;
	AVFrame* avframe_in = nullptr;
	if(frame)
	{
		if(0 != (frame->_info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
		{
			avcodec_flush_buffers(_ctxCodec);
		}

		memset(&avframe,0,sizeof(avframe));
		avframe.extended_data = avframe.data;

		JIF(convert_frame_to_avframe(_pin_input->get_media_type(),&avframe,frame,_ctxCodec))
		avframe_in = &avframe;
        //printf("encode input frame DTS:%ld PTS:%ld\n",pFrame->info.dts/10000,pFrame->info.pts/10000);
	}

    AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = nullptr;
	pkt.size = 0;
	int is_output = 0;
	int ret = avcodec_encode_video2(_ctxCodec,&pkt,avframe_in,&is_output);
	if(0 != is_output)
	{
        frame_ptr frame_out = media_frame::create();
        JCHK(frame_out,rc_new_fail)
        JIF(convert_packet_to_frame(frame_out,pkt,_ctxCodec->time_base))
        _pin_output->deliver(frame_out);
        av_packet_unref(&pkt);
	}
	else
	{
        if(!frame)
        {
            _pin_output->deliver(frame);
        }
        else if(0 > ret)
        {
            char err[AV_ERROR_MAX_STRING_SIZE] = {0};
            JCHKM(0 == ret,rc_fail,FORMAT_STR("ffmpeg encode frame[DTS:%1%] fail,msg:%2%",
                %frame->_info.dts%av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))
        }
	}
	return rt;
}

void ffmpeg_video_encoder::close()
{
    if(nullptr != _ctxCodec)
    {
        avcodec_close(_ctxCodec);
        av_free(_ctxCodec);
        _ctxCodec = nullptr;
    }
}
