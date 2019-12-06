#include "ffmpeg_video_decoder.h"

ffmpeg_video_decoder::ffmpeg_video_decoder()
:_ctxCodec(nullptr)
,_avframe(nullptr)
{
    g_dump.set_class("ffmpeg_video_decoder");
	av_init_packet(&_pkt);
    //ctor
}

ffmpeg_video_decoder::~ffmpeg_video_decoder()
{
    //dtor
    close();
}

CLS_INFO_DEFINE(media_transform,ffmpeg_video_decoder,MAKE_VERSION(0,0,0,0))

uint32_t ffmpeg_video_decoder::cls_priority(const char* info,media_type* mt_input,media_type* mt_output)
{
    if(nullptr == mt_input)
        return 0;
    if(nullptr != mt_output)
    {
        if(mt_input->get_major() != mt_output->get_major() || MMT_VIDEO != mt_input->get_major())
            return 0;
        if(false == mt_input->is_compress() || true == mt_output->is_compress())
            return 0;
        if(mt_input->get_video_format() != mt_output->get_video_format())
            return 0;
        if(mt_input->get_video_width() != mt_output->get_video_width() || mt_input->get_video_height() != mt_output->get_video_height())
            return 0;
        if(mt_input->get_video_fps() != mt_output->get_video_fps())
            return 0;
    }
    else
    {
        if(MMT_VIDEO != mt_input->get_major() || false == mt_input->is_compress())
            return 0;
    }
    return 1;
}

ret_type ffmpeg_video_decoder::set_media_type(input_pin* pin,media_ptr mt)
{
    if(nullptr != mt && true == mt->is_compress())
    {
        ret_type rt;
        JIF(open(mt.get()))
        media_ptr mt_out = media_type::create();
        JCHK(mt_out,rc_new_fail)
        JIF(media_type::copy(mt_out,mt))
        mt_out->set_sub(MST_RAWVIDEO);
        mt_out->set_global_header(false);
        mt_out->set_extra_data(nullptr,0);
        mt_out->set_bitrate(0);
        return _pin_output->set_media_type(mt_out);
    }
    else
    {
        close();
        _pin_output->disconnect_all();
        return rc_ok;
    }
}

ret_type ffmpeg_video_decoder::set_media_type(output_pin* pin,media_ptr mt)
{
    media_ptr mt_out = pin->get_media_type();
    if(mt_out)
    {
        JCHK(media_type::compare(mt,mt_out),rc_param_invalid)
    }
    return rc_ok;
}

ret_type ffmpeg_video_decoder::process(input_pin* pin,frame_ptr frame)
{
    JCHK(nullptr != _ctxCodec,rc_state_invalid)
    ret_type rt = rc_ok;
    if(!frame)
    {
        _pkt.buf = nullptr;
        _pkt.data = nullptr;
        _pkt.size = 0;
    }
    else
    {
        if(nullptr == _ctxCodec)
        {
            media_ptr mt = _pin_input->get_media_type();
            JCHK(mt,rc_state_invalid)
            JIF(open(mt.get()))
        }
        if(0 != (frame->_info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
        {
            avcodec_flush_buffers(_ctxCodec);
        }
        JIF(convert_frame_to_packet(_pkt,&_ctxCodec->time_base,frame))
        if(AV_NOPTS_VALUE == _pkt.pos || _pkt.pts > _pkt.pos)
            _pkt.pos = _pkt.pts;
    }

    int is_output = 0;
    int ret = avcodec_decode_video2(_ctxCodec,_avframe, &is_output,&_pkt);
    if(0 != is_output)
    {
        media_ptr mt = _pin_output->get_media_type();
        if(_ctxCodec->width != mt->get_video_width())
            mt->set_video_width(_ctxCodec->width);
        if(_ctxCodec->height != mt->get_video_height())
            mt->set_video_height(_ctxCodec->height);

        frame_ptr frame_out = media_frame::create();
        JIF(convert_avframe_to_frame(mt,frame_out,_avframe,_ctxCodec))
        return _pin_output->deliver(frame_out);
    }
    else
    {
        if(!frame)
        {

            return _pin_output->deliver(frame);
        }
        else
        {
            char err[AV_ERROR_MAX_STRING_SIZE] = {0};
            JCHKM(0 <= ret,rc_fail,FORMAT_STR("avcodec_decode_video2 fail,DTS=%1%,message=%2%",
                %frame->_info.dts%av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))
        }
    }
    return rt;
}

ret_type ffmpeg_video_decoder::open(media_type* mt)
{
    close();

    AVCodec* codec;
    JCHKM(codec = avcodec_find_decoder((AVCodecID)mt->get_sub()),rc_param_invalid,FORMAT_STR("can not find decoder,sub=%1%",%mt->get_sub_name()));
    JCHK(_ctxCodec = avcodec_alloc_context3(codec),rc_fail)
    _ctxCodec->time_base = FRAME_TIMEBASE;
    if(true == mt->get_global_header())
    {
        _ctxCodec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        _ctxCodec->flags2 &= ~AV_CODEC_FLAG2_LOCAL_HEADER;
    }
    else
    {
        _ctxCodec->flags &= ~CODEC_FLAG_GLOBAL_HEADER;
        _ctxCodec->flags2 |= AV_CODEC_FLAG2_LOCAL_HEADER;
    }
    _ctxCodec->extradata = mt->get_extra_data();
    _ctxCodec->extradata_size = mt->get_extra_size();

    if(AV_PIX_FMT_NONE == _ctxCodec->pix_fmt)
    {
        if(nullptr == _ctxCodec->codec->pix_fmts)
        {
            _ctxCodec->pix_fmt = (AVPixelFormat)mt->get_video_format();
            if(_ctxCodec->pix_fmt  == AV_PIX_FMT_NONE)
                _ctxCodec->pix_fmt =  AV_PIX_FMT_YUV420P;
        }
        else
            _ctxCodec->pix_fmt = _ctxCodec->codec->pix_fmts[0];
    }
    if(_ctxCodec->pix_fmt != (AVPixelFormat)mt->get_video_format())
        mt->set_video_format((VideoMediaType)_ctxCodec->pix_fmt);

    _ctxCodec->width = mt->get_video_width();
    _ctxCodec->height = mt->get_video_height();

    int ratioX = mt->get_video_ratioX();
    int ratioY = mt->get_video_ratioY();

    if(0 == ratioX)
        ratioX = _ctxCodec->width;
    if(0 == ratioY)
        ratioY = _ctxCodec->height;

    int g = gcd(ratioX,ratioY);
    ratioX /= g;
    ratioY /= g;

    unsigned int sar_x = ratioX * _ctxCodec->height;
    unsigned int sar_y = ratioY * _ctxCodec->width;

    g = gcd(sar_x,sar_y);
    _ctxCodec->sample_aspect_ratio.num = sar_x/g;
    _ctxCodec->sample_aspect_ratio.den = sar_y/g;

    if(1 > _ctxCodec->block_align)
        _ctxCodec->block_align = VIDEO_ALIGN;

    _ctxCodec->framerate.num = ONE_SECOND_UNIT;
    _ctxCodec->framerate.den = mt->get_video_duration();
    _ctxCodec->thread_count = 0;

    av_init_packet(&_pkt);
    if(nullptr == _avframe)
    {
        JCHK(_avframe = av_frame_alloc(),rc_new_fail);
    }
    JCHK(0 == avcodec_open2(_ctxCodec,avcodec_find_decoder(_ctxCodec->codec_id),nullptr),rc_fail)

    return rc_ok;
}

void ffmpeg_video_decoder::close()
{
//    if(nullptr != _avframe)
//        av_frame_free(&_avframe);
//    if(nullptr != _ctxCodec)
//    {
//        avcodec_close(_ctxCodec);
//        av_free(_ctxCodec);
//        _ctxCodec = nullptr;
//    }
}
