#include "ffmpeg_audio_decoder.h"

ffmpeg_audio_decoder::ffmpeg_audio_decoder()
:_ctxCodec(nullptr)
,_avframe(nullptr)
{
	av_init_packet(&_pkt);
    //ctor
}

ffmpeg_audio_decoder::~ffmpeg_audio_decoder()
{
    //dtor
    close();
}

CLS_INFO_DEFINE(media_transform,ffmpeg_audio_decoder,MAKE_VERSION(0,0,0,0))

uint32_t ffmpeg_audio_decoder::cls_priority(const char* info,media_type* mt_input,media_type* mt_output)
{
    if(nullptr == mt_input)
        return 0;
    if(nullptr != mt_output)
    {
        if(mt_input->get_major() != mt_output->get_major() || MMT_AUDIO != mt_input->get_major())
            return 0;
        if(false == mt_input->is_compress() || true == mt_output->is_compress())
            return 0;
        if(mt_input->get_audio_format() != mt_output->get_audio_format())
            return 0;
        if(mt_input->get_audio_channel() != mt_output->get_audio_channel())
            return 0;
        if(mt_input->get_audio_sample_rate() != mt_output->get_audio_sample_rate())
            return 0;
        if(mt_input->get_audio_frame_size() != mt_output->get_audio_frame_size())
            return 0;
    }
    else
    {
        if(MMT_AUDIO != mt_input->get_major() || false == mt_input->is_compress())
            return 0;
    }
    return 1;
}

ret_type ffmpeg_audio_decoder::set_media_type(input_pin* pin,media_ptr mt)
{
    if(mt)
    {
        JCHK(mt->is_compress(),rc_param_invalid)

        ret_type rt;
        JIF(open(mt.get()))
        media_ptr mt_out = media_type::create();
        JCHK(mt_out,rc_new_fail)
        JIF(media_type::copy(mt_out,mt))
        mt_out->set_sub(MST_PCM);
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

ret_type ffmpeg_audio_decoder::set_media_type(output_pin* pin,media_ptr mt)
{
    media_ptr mt_out;
    if(mt_out = pin->get_media_type())
    {
        JCHK(media_type::compare(mt,mt_out),rc_param_invalid)
    }
    return rc_ok;
}

ret_type ffmpeg_audio_decoder::process(input_pin* pin,frame_ptr frame)
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
        //TRACE(dump::info,FORMAT_STR("audio decoder input frame dts:%1% pts%2% flag:%3%",
        //    %(frame->_info.dts/10000)%(frame->_info.pts/10000)%frame->_info.flag))
        if(0 != (frame->_info.flag & MEDIA_FRAME_FLAG_NEWSEGMENT))
        {
            avcodec_flush_buffers(_ctxCodec);
        }
        JIF(convert_frame_to_packet(_pkt,&_ctxCodec->time_base,frame))
    }

    int is_output = 0;
    int64_t pts = MEDIA_FRAME_NONE_TIMESTAMP;
    do
    {
        int cb = avcodec_decode_audio4(_ctxCodec,_avframe, &is_output,&_pkt);
        if(0 != is_output)
        {
            if(_avframe->pkt_pts <= pts)
                _avframe->pkt_pts = pts;
            if(MEDIA_FRAME_NONE_TIMESTAMP == pts)
                pts = _avframe->pkt_pts;
            pts += _avframe->nb_samples;
            if(_avframe->pkt_dts != _avframe->pkt_pts)
                _avframe->pkt_dts = _avframe->pkt_pts;

            media_ptr mt = _pin_output->get_media_type();
            if(mt->get_audio_frame_size() != _avframe->nb_samples)
            {
                mt->set_audio_frame_size(_avframe->nb_samples);
                //JIF(_pin_output->set_media_type(mt));
            }
            frame_ptr frame_out = media_frame::create();
            JIF(convert_avframe_to_frame(mt,frame_out,_avframe,_ctxCodec))
            return _pin_output->deliver(frame_out);
        }
        else
        {
            if(!frame)
                return _pin_output->deliver(frame);
            else if(0 > cb)
            {
                char err[AV_ERROR_MAX_STRING_SIZE] = {0};
                JCHKM(0 <= cb,rc_fail,FORMAT_STR("ffmpeg decode frame[DTS:%1%] fail,msg:%2%",
                    %(frame->_info.dts/10000)%av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,cb)))
            }
        }
        _pkt.size -= cb;
        _pkt.data += cb;
    }while(0 < _pkt.size);
    return rt;
}

ret_type ffmpeg_audio_decoder::open(media_type* mt)
{
    close();

    AVCodec* codec;
    AVCodecContext* ctxCodec;
    JCHKM(codec = avcodec_find_decoder((AVCodecID)mt->get_sub()),rc_param_invalid,FORMAT_STR("media[%1%] can not find decoder",%mt->get_sub_name()));
    JCHK(ctxCodec = avcodec_alloc_context3(codec),rc_fail)
    ctxCodec->time_base = FRAME_TIMEBASE;
    ctxCodec->sample_fmt = (AVSampleFormat)mt->get_audio_format();
    ctxCodec->channels = mt->get_audio_channel();
    ctxCodec->sample_rate = mt->get_audio_sample_rate();
    if(MST_WMAPRO == mt->get_sub())
        ctxCodec->frame_size = 2048;
    else
        ctxCodec->frame_size = mt->get_audio_frame_size();

    av_init_packet(&_pkt);
    if(nullptr == _avframe)
    {
        JCHK(_avframe = av_frame_alloc(),rc_new_fail);
    }
    JCHK(0 == avcodec_open2(ctxCodec,avcodec_find_decoder(ctxCodec->codec_id),nullptr),rc_fail)

    _ctxCodec = ctxCodec;
    return rc_ok;
}

void ffmpeg_audio_decoder::close()
{
    if(nullptr != _avframe)
        av_frame_free(&_avframe);
    if(nullptr != _ctxCodec)
    {
        avcodec_close(_ctxCodec);
        av_free(_ctxCodec);
        _ctxCodec = nullptr;
    }
}
