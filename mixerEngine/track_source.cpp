#include "track_source.h"
#include "engine_source.h"

meta_source::meta_source()
{
}

meta_source::~meta_source()
{
}

ret_type meta_source::process()
{
    JCHK(_source,rc_state_invalid)
    return _source->process();
}

ret_type meta_source::open(const string& url)
{
    ret_type rt;
    _source = create_filter<media_source>(url.c_str());
    JCHK(_source,rc_param_invalid)
    JIFM(_source->open(url),FORMAT_STR("segment source open fail,will be ignore,url=%1%",%url))
    return rt;
}

source_ptr meta_source::get_source()
{
    return _source;
}

tracker_source::tracker_source(engine_tracker* tracker,SegmentIt it,int64_t time_buf)
:_tracker(tracker)
,_it(it)
,_time_buf(time_buf)
,_pin(new input_pin(this))
,_task(nullptr)
,_time(MEDIA_FRAME_NONE_TIMESTAMP)
,_time_line(MEDIA_FRAME_NONE_TIMESTAMP)
,_is_buf(false)
,_start(MEDIA_FRAME_NONE_TIMESTAMP)
,_stop(MEDIA_FRAME_NONE_TIMESTAMP)
{
    g_dump.set_class("tracker_source");
}

tracker_source::~tracker_source()
{
    _pin->disconnect();
}

ret_type tracker_source::set_media_type(input_pin* pin,media_ptr mt)
{
    return rc_ok;
}

ret_type tracker_source::process(input_pin* pin,frame_ptr frame)
{
    if(_buf.is_eof())
        return rc_ok;

    ret_type rt;

    if(frame)
    {
        if(_stop != MEDIA_FRAME_NONE_TIMESTAMP && frame->_info.dts >= _stop)
            frame.reset();
        else
            _time = frame->_info.dts - _time_buf;
    }

    JIF(_buf.push(frame))

    if(_buf.is_eof() || _time > _time_line)
    {
        bool except = true;
        if(_is_buf.compare_exchange_weak(except,false))
            _source->stop();
    }

    if(nullptr != _task)
    {
        if(_buf.is_eof() || frame->_info.dts >= _task->_time)
        {
            _task->run();
            _task = nullptr;
        }
    }

    if(!frame)
    {
        if(nullptr != _tracker)
            _tracker->next_source(_it);
    }

    return rt;
}

ret_type tracker_source::set_meta(meta_source::ptr meta,media_ptr mt,int64_t start,int64_t& length,int64_t stop)
{
    JCHK(meta,rc_param_invalid)
    JCHK(mt,rc_param_invalid)
    source_ptr source = meta->get_source();
    JCHK(source,rc_param_invalid)

    ret_type rt;
    output_pin_ptr pin_out;
    uint32_t index = 0;
    while(true)
    {
        output_pin_ptr pin = source->get_pin(index++);
        if(pin)
        {
            media_ptr mt_pin = pin->get_media_type();
            JCHK(mt_pin,rc_param_invalid)
            if(mt_pin->get_major() == mt->get_major())
            {
                pin_out = pin;
                if(mt_pin->get_length() > length)
                    length = mt_pin->get_length();
                break;
            }
        }
        else
            break;
    }
    JCHK(pin_out,rc_param_invalid)
    JIF(connect(pin_out,_pin,media_ptr(),mt))
    _start = start;
    _stop = stop;
    _source = meta;
    return rt;
}

ret_type tracker_source::pop(media_task* task,frame_ptr& frame)
{
    JCHK(_source,rc_state_invalid)

    if(nullptr == task)
    {
        while(false == _buf.peek(frame))
        {
            if(_buf.is_eof())
                return media_task::rc_eof;
            _source->process();
        }
        _buf.pop();
        return rc_ok;
    }
    else
    {
        ret_type rt;
        _time_line = task->_time;
        frame_ptr frame_out;
        while(_buf.peek(frame_out))
        {
            if(_time_line >= frame_out->_info.pts && _time_line < frame_out->_info.pts + frame_out->_info.duration)
            {
                _buf.pop();
                frame_out->_info.pts = _time_line;
                frame_out->_info.dts = _time_line;
                break;
            }
            else if(_time_line < frame_out->_info.pts)
            {
                break;
            }
            else
            {
                _buf.pop();
            }
        }
        if(frame_out)
        {
            if(frame_out->_info.pts == _time_line)
                frame = frame_out;
            rt = rc_ok;
        }
        else
        {
            if(_buf.is_eof())
                rt = media_task::rc_eof;
            else
            {
                _task = task;
                rt = media_task::rc_again;
            }
        }
        if(false == _buf.is_eof() && _time <= _time_line)
        {
            bool except = false;
            if(_is_buf.compare_exchange_weak(except,true))
                _source->run();
        }
        return rt;
    }
}
