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
        if(false == mt_input->is_compress())
            return 0;
    }
    return 1;
}

ret_type ffmpeg_audio_decoder::set_media_type(input_pin* pin,media_type* mt)
{
    if(nullptr != mt && true == mt->is_compress())
    {
        ret_type rt;
        JIF(open(mt))
        std::shared_ptr<media_type> mt_out(new media_type());
        JCHK(mt_out,rc_new_fail)
        JIF(media_type::copy(mt_out.get(),mt))
        mt_out->set_sub(MST_PCM);
        mt_out->set_global_header(false);
        mt_out->set_extra_data(nullptr,0);
        mt_out->set_bitrate(0);
        return _pin_output->set_media_type(mt_out.get());
    }
    else
    {
        close();
        _pin_output->disconnect_all();
        return rc_ok;
    }
}

ret_type ffmpeg_audio_decoder::set_media_type(output_pin* pin,media_type* mt)
{
    media_type* mt_out;
    if(nullptr != (mt_out = pin->get_media_type()))
    {
        JCHK(media_type::compare(mt,mt_out),rc_param_invalid)
    }
    return rc_ok;
}

ret_type ffmpeg_audio_decoder::process(input_pin* pin,media_frame* frame)
{
    if(nullptr != _ctxCodec)
        return audio_decode(frame);
    else
        return _pin_output->deliver(frame);
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

ret_type ffmpeg_audio_decoder::audio_decode(media_frame* frame)
{
    ret_type rt = rc_ok;
    if(frame == nullptr)
    {
        _pkt.buf = nullptr;
        _pkt.data = nullptr;
        _pkt.size = 0;
    }
    else
    {
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

            media_type* mt = _pin_output->get_media_type();
            std::shared_ptr<media_frame> frame(new media_frame());
            JIF(convert_array_to_frame(mt,(const uint8_t**)_avframe->data,(const int*)_avframe->linesize,frame.get()))
            frame->_info.flag |= 0 == _avframe->key_frame ? 0 : MEDIA_FRAME_FLAG_SYNCPOINT;
            frame->_info.dts = _avframe->pkt_dts;
            frame->_info.pts = _avframe->pkt_pts;
            return _pin_output->deliver(frame.get());
        }
        else
        {
            if(nullptr == _pkt.data)
            {
                return _pin_output->deliver(nullptr);
            }
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
