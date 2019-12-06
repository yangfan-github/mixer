#include "ffmpeg_video_scale.h"

ffmpeg_video_scale::ffmpeg_video_scale()
:_ctxSws(nullptr)
{
    //ctor
    g_dump.set_class("ffmpeg_video_scale");
}

ffmpeg_video_scale::~ffmpeg_video_scale()
{
    //dtor
    close();
}

CLS_INFO_DEFINE(media_transform,ffmpeg_video_scale,MAKE_VERSION(0,0,0,0))

uint32_t ffmpeg_video_scale::cls_priority(const char* info,media_type* mt_input,media_type* mt_output)
{
    if(nullptr == mt_input || nullptr == mt_output)
        return 0;
    if(mt_input->get_major() != mt_output->get_major() || MMT_VIDEO != mt_input->get_major())
        return 0;
    if(true == mt_input->is_compress() || true == mt_output->is_compress())
        return 0;
    return 1;
}

ret_type ffmpeg_video_scale::set_media_type(input_pin* pin,media_ptr mt)
{
    if(mt)
    {
        if(mt->get_sub() != MST_RAWVIDEO)
            return rc_param_invalid;
        if(mt->get_video_format() <= VMT_NONE || mt->get_video_format() >= VMT_NB)
            return rc_param_invalid;
        if(mt->get_video_width() <= 0 || mt->get_video_height() <= 0)
            return rc_param_invalid;
        if(mt->get_video_duration() <= 0)
            return rc_param_invalid;
    }
    close();
    return rc_ok;
}

ret_type ffmpeg_video_scale::set_media_type(output_pin* pin,media_ptr mt)
{
    if(mt)
    {
        if(mt->get_sub() != MST_RAWVIDEO)
            return rc_param_invalid;
        if(mt->get_video_format() <= VMT_NONE || mt->get_video_format() >= VMT_NB)
            return rc_param_invalid;
        if(mt->get_video_width() <= 0 || mt->get_video_height() <= 0)
            return rc_param_invalid;
        if(mt->get_video_duration() <= 0)
            return rc_param_invalid;
    }
    close();
    return rc_ok;
}

ret_type ffmpeg_video_scale::process(input_pin* pin,frame_ptr frame)
{
    if(!frame)
    {
        return video_sample(frame);
    }

    //ret_type rt;
    media_ptr mt_in = _pin_input->get_media_type();
    media_ptr mt_out = _pin_output->get_media_type();

    VideoMediaType format_in = mt_in->get_video_format();
    VideoMediaType format_out = mt_out->get_video_format();

    int width_in = mt_in->get_video_width();
    int width_out = mt_out->get_video_width();
    int height_in = mt_in->get_video_height();
    int height_out = mt_out->get_video_height();

    if(format_in != format_out || width_in != width_out || height_in != height_out)
    {
        if(nullptr == _ctxSws)
        {
            JCHK(_ctxSws = sws_getContext(width_in,height_in,(AVPixelFormat)format_in,
                width_out,height_out,(AVPixelFormat)format_out,SWS_BICUBIC, NULL, NULL, NULL),rc_fail);
        }

        ret_type rt;

        frame_ptr frame_out = media_frame::create();
        JCHK(frame_out,rc_new_fail)

        uint8_t *dataIn[AV_NUM_DATA_POINTERS];
        int linesizeIn[AV_NUM_DATA_POINTERS];

        uint8_t *dataOut[AV_NUM_DATA_POINTERS];
        int linesizeOut[AV_NUM_DATA_POINTERS];

        frame_out->_info = frame->_info;

        JIF(convert_frame_to_array(mt_in,frame,(uint8_t**)dataIn,(int*)linesizeIn))
        JIF(convert_frame_to_array(mt_out,frame_out,(uint8_t**)dataOut,(int*)linesizeOut))
        //TRACE(dump::info,FORMAT_STR("video scale frame PTS:%1%",%(frame->_info.pts)))
		JCHKM(height_out == sws_scale(_ctxSws,dataIn,linesizeIn,0,height_in,dataOut,linesizeOut),
            rc_fail,FORMAT_STR("sws_scale fail,PTS=%1%",%frame->_info.pts))

        return video_sample(frame_out);
    }
    else
    {
        return video_sample(frame);
    }
}

ret_type ffmpeg_video_scale::video_sample(frame_ptr frame)
{
    if(!frame)
        return _pin_output->deliver(frame);

    int64_t duration_in = _pin_input->get_media_type()->get_video_duration();
    JCHK(duration_in >= 0,rc_state_invalid)

    int64_t duration_out = _pin_output->get_media_type()->get_video_duration();
    JCHK(duration_out >= 0,rc_state_invalid)

    if(!_frame)
    {
        _frame = frame;
        _frame->_info.duration = duration_out;
        return _pin_output->deliver(frame);
    }
    else
    {
        ret_type rt = rc_ok;
        int64_t delta = frame->_info.pts - _frame->_info.pts;
        if(delta >= duration_out)
        {
            if(delta >= duration_out + duration_in)
            {
                _frame = frame;
                _frame->_info.duration = duration_out;
                return _pin_output->deliver(frame);
            }
            do
            {
                frame_ptr frame_new = media_frame::create();
                JCHK(frame_new,rc_new_fail)
                JIF(media_frame::copy(frame_new,_frame))
                frame_new->_info.pts += duration_out;
                frame_new->_info.dts += duration_out;
                frame_new->_info.flag = 0;
                _pin_output->deliver(frame_new);
                _frame = frame_new;
                delta = frame->_info.pts - _frame->_info.pts;
            }while(delta >= duration_out);
            frame->_info.pts -= delta;
            frame->_info.dts -= delta;
            _frame = frame;
        }
        return rt;
    }
}

void ffmpeg_video_scale::close()
{
    if(nullptr != _ctxSws)
    {
        sws_freeContext(_ctxSws);
        _ctxSws = nullptr;
    }
    _frame.reset();
}
