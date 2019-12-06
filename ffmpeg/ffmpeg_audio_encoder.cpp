#include "ffmpeg_audio_encoder.h"

ffmpeg_audio_encoder::ffmpeg_audio_encoder()
:_ctxCodec(nullptr)
{
    //ctor
    g_dump.set_class("ffmpeg_audio_encoder");
}

ffmpeg_audio_encoder::~ffmpeg_audio_encoder()
{
    //dtor
    close();
}

CLS_INFO_DEFINE(media_transform,ffmpeg_audio_encoder,MAKE_VERSION(0,0,0,0))

uint32_t ffmpeg_audio_encoder::cls_priority(const char* info,media_type* mt_input,media_type* mt_output)
{
    if(nullptr == mt_output || nullptr != mt_input)
        return 0;

    if(MMT_AUDIO != mt_output->get_major() || false == mt_output->is_compress())
        return 0;

    AVCodec* codec = avcodec_find_encoder((AVCodecID)mt_output->get_sub());
    if(nullptr == codec)
        return 0;

    return 1;
}

ret_type ffmpeg_audio_encoder::set_media_type(input_pin* pin,media_ptr mt)
{
    media_ptr mt_in = pin->get_media_type();
    if(mt_in)
    {
        JCHK(media_type::compare(mt,mt_in),rc_param_invalid)
    }
    return rc_ok;
}

ret_type ffmpeg_audio_encoder::set_media_type(output_pin* pin,media_ptr mt)
{
    if(mt && true == mt->is_compress())
    {
        ret_type rt;
        JIF(open(mt.get()))
        media_ptr mt_in = media_type::create();
        JCHK(mt_in,rc_new_fail)
        JIF(media_type::copy(mt_in,mt))
        mt_in->set_sub(MST_PCM);
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

ret_type ffmpeg_audio_encoder::open(media_type* mt)
{
    close();
    AVCodec* codec;
    JCHKM(codec = avcodec_find_encoder((AVCodecID)mt->get_sub()),rc_param_invalid,FORMAT_STR("can not find encoder,sub=%1%",%mt->get_sub_name()));
    JCHK(_ctxCodec = avcodec_alloc_context3(codec),rc_fail)

    AudioMediaType amt = mt->get_audio_format();
    if(NULL != codec->sample_fmts)
    {
        int i=0;
        while(AV_SAMPLE_FMT_NONE != codec->sample_fmts[i] && (AVSampleFormat)amt != codec->sample_fmts[i]){++i;}
        if(AV_SAMPLE_FMT_NONE == codec->sample_fmts[i])
        {
            amt = (AudioMediaType)codec->sample_fmts[0];
            mt->set_audio_format(amt);
        }
        _ctxCodec->sample_fmt = (AVSampleFormat)amt;
    }
    else if(amt != (AudioMediaType)_ctxCodec->sample_fmt)
    {
        if(AV_SAMPLE_FMT_NONE == _ctxCodec->sample_fmt)
        {
            _ctxCodec->sample_fmt = (AVSampleFormat)amt;
        }
        else
        {
            amt = (AudioMediaType)_ctxCodec->sample_fmt;
            mt->set_audio_format(amt);
        }
    }
    else if(AMT_NONE >= amt || AMT_NB <= amt)
    {
        amt = AMT_FLT;
        mt->set_audio_format(amt);
        _ctxCodec->sample_fmt = (AVSampleFormat)amt;
    }

    JCHK(0 <(_ctxCodec->channels = mt->get_audio_channel()),rc_param_invalid)
    _ctxCodec->channel_layout = av_get_default_channel_layout(_ctxCodec->channels);
    JCHK(0 < (_ctxCodec->sample_rate = mt->get_audio_sample_rate()),rc_param_invalid)

    if(NULL != codec->channel_layouts)
    {
        int i=0;
        while(0 != codec->channel_layouts[i] && _ctxCodec->channel_layout != codec->channel_layouts[i]){++i;}
        if(0 == codec->channel_layouts[i])
        {
            TRACE(dump::warn,FORMAT_STR("not support channel layout,old=%1%,new=%2%",
                %_ctxCodec->channel_layout%codec->channel_layouts[0]))
            _ctxCodec->channel_layout = codec->channel_layouts[0];
            mt->set_audio_channel(av_get_channel_layout_nb_channels(_ctxCodec->channel_layout));
        }
    }

    if(NULL != codec->supported_samplerates)
    {
        int tmp = _ctxCodec->sample_rate;
        get_audio_sample_rate(codec->supported_samplerates,tmp);
        if(tmp != _ctxCodec->sample_rate)
        {
            TRACE(dump::warn,FORMAT_STR("not support sample,old=%1%,new=%2%",%_ctxCodec->sample_rate%tmp))
            _ctxCodec->sample_rate = tmp;
            mt->set_audio_sample_rate(tmp);
        }
    }

    _ctxCodec->time_base.num = 1;
    _ctxCodec->time_base.den = _ctxCodec->sample_rate;

    codec->init(_ctxCodec);
    JCHK(0 < _ctxCodec->frame_size,rc_fail);
    mt->set_audio_frame_size(_ctxCodec->frame_size);
    _ctxCodec->block_align = AUDIO_ALIGN;

    _ctxCodec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    _ctxCodec->flags2 &= ~AV_CODEC_FLAG2_LOCAL_HEADER;

    JCHK(0 < (_ctxCodec->bit_rate = mt->get_bitrate()),rc_param_invalid)

    int ret;
    char err[AV_ERROR_MAX_STRING_SIZE] = {0};
    _ctxCodec->thread_count = 0;
    JCHKM(0 == (ret = avcodec_open2(_ctxCodec,codec,NULL)),rc_fail,
            FORMAT_STR("avcodec_open2 fail,sub=%1%,message=%2%",
            %mt->get_sub_name()%av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))

    property_tree::ptree pt = mt->get_codec_option();
    get_option(_ctxCodec->priv_data,pt);

    return mt->set_extra_data(_ctxCodec->extradata,_ctxCodec->extradata_size);
}

ret_type ffmpeg_audio_encoder::process(input_pin* pin,frame_ptr frame)
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
	int ret = avcodec_encode_audio2(_ctxCodec,&pkt,avframe_in,&is_output);
	if(0 != is_output)
	{
        frame_ptr frame_out= media_frame::create();
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
            JCHKM(0 == ret,rc_fail,FORMAT_STR("avcodec_encode_audio2 fail,DTS=%1%,message=%2%",
                %frame->_info.dts%av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))
        }
	}
	return rt;
}

void ffmpeg_audio_encoder::close()
{
    if(nullptr != _ctxCodec)
    {
        avcodec_close(_ctxCodec);
        av_free(_ctxCodec);
        _ctxCodec = nullptr;
    }
}
