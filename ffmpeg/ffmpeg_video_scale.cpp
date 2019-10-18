#include "ffmpeg_video_scale.h"

ffmpeg_video_scale::ffmpeg_video_scale()
:_ctxSws(nullptr)
{
    //ctor
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
    if(false == mt_input->is_compress() || false == mt_output->is_compress())
        return 0;
    return 1;
}

ret_type ffmpeg_video_scale::set_media_type(input_pin* pin,media_type* mt)
{
    if(nullptr != mt)
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

ret_type ffmpeg_video_scale::set_media_type(output_pin* pin,media_type* mt)
{
    if(nullptr != mt)
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

ret_type ffmpeg_video_scale::process(input_pin* pin,media_frame* frame)
{
    if(nullptr == frame)
        return video_sample(frame);

    ret_type rt;
    media_type* mt_in = _pin_input->get_media_type();
    media_type* mt_out = _pin_output->get_media_type();

    VideoMediaType format_in = mt_in->get_video_format();
    VideoMediaType format_out = mt_out->get_video_format();
    int width_in = mt_in->get_video_width();
    int width_out = mt_out->get_video_width();
    int height_in = mt_in->get_video_height();
    int height_out = mt_out->get_video_height();

    if(format_in != format_out || width_in != width_out || height_in != height_out)
    {
        if(nullptr != _ctxSws)
        {
            JCHK(_ctxSws = sws_getContext(width_in,height_in,(AVPixelFormat)format_in,
                width_out,height_out,(AVPixelFormat)format_out,SWS_BICUBIC, NULL, NULL, NULL),rc_fail);
        }

        std::shared_ptr<media_frame> frame_out(new media_frame());
        JCHK(frame_out,rc_new_fail)

        uint8_t *dataIn[AV_NUM_DATA_POINTERS];
        int linesizeIn[AV_NUM_DATA_POINTERS];

        uint8_t *dataOut[AV_NUM_DATA_POINTERS];
        int linesizeOut[AV_NUM_DATA_POINTERS];

        JIF(convert_frame_to_array(mt_in,frame,(uint8_t**)dataIn,(int*)linesizeIn))
        JIF(convert_frame_to_array(mt_out,frame_out.get(),(uint8_t**)dataOut,(int*)linesizeOut))

		JCHKM(height_out == sws_scale(_ctxSws,dataIn,linesizeIn,0,height_in,dataOut,linesizeOut),
            rc_fail,FORMAT_STR("ffmpeg video scale frame[PTS:%1%] fail",%frame->_info.pts))
        frame_out->_info = frame->_info;
        return video_sample(frame_out.get());
    }
    else
    {
        return video_sample(frame);
    }
}

ret_type ffmpeg_video_scale::video_sample(media_frame* frame)
{
    if(nullptr == frame)
        return _pin_output->deliver(frame);

    int64_t duration_in = _pin_input->get_media_type()->get_video_duration();
    JCHK(duration_in >= 0,rc_state_invalid)

    int64_t duration_out = _pin_output->get_media_type()->get_video_duration();
    JCHK(duration_out >= 0,rc_state_invalid)

    if(duration_in != duration_out)
    {
        if(_frame)
        {
            ret_type rt = rc_ok;
            int64_t delta = frame->_info.pts - _frame->_info.pts;
            if(delta >= duration_out)
            {
                do
                {
                    std::shared_ptr<media_frame> frame_new(new media_frame());
                    JCHK(frame_new,rc_new_fail)
                    JIF(media_frame::copy(frame_new.get(),_frame.get()))
                    frame_new->_info.pts += duration_out;
                    frame_new->_info.dts += duration_out;
                    frame_new->_info.flag = 0;
                    _pin_output->deliver(frame_new.get());
                    _frame.reset(frame_new.get());
                    delta = frame->_info.pts - _frame->_info.pts;
                }while(delta >= duration_out);
                frame->_info.pts -= delta;
                frame->_info.dts -= delta;
                _frame.reset(frame);
            }
            return rt;
        }
        else
        {
            _frame.reset(frame);
            _frame->_info.duration = duration_out;
            return _pin_output->deliver(frame);
        }
        return rc_ok;
    }
    else
    {
        return _pin_output->deliver(frame);
    }
}

void ffmpeg_video_scale::close()
{
    if(nullptr != _ctxSws)
    {
        sws_freeContext(_ctxSws);
        _ctxSws = NULL;
    }
    _frame.reset();
}
