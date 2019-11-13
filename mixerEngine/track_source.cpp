#include "track_source.h"
#include "engine_source.h"

tracker_source::tracker_source(engine_tracker* tracker,SegmentIt it,int64_t time_buf)
:_tracker(tracker)
,_it(it)
,_time_buf(time_buf)
,_pin(new input_pin(this))
,_eof(false)
,_task(nullptr)
,_time_line(MEDIA_FRAME_NONE_TIMESTAMP)
,_is_buf(false)
,_start(MEDIA_FRAME_NONE_TIMESTAMP)
,_stop(MEDIA_FRAME_NONE_TIMESTAMP)
{
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
    ret_type rt;

    if(frame)
    {
        int64_t time = frame->_info.dts - _time_buf;
        //TRACE(dump::info,FORMAT_STR("tracker:%1% input frame dts:%2%ms pts%3%ms duration:%4%ms _time:%5%ms time_line:%6%ms",
        //    %pin->get_media_type()->get_major_name()%(frame->_info.dts/10000)%(frame->_info.pts/10000)%(frame->_info.duration/10000)%(_time/10000)%(_time_line/10000)))
        _time = time;
    }

    JIF(_buf.push(frame))

    if(nullptr != _task)
    {
        if(!frame || frame->_info.dts >= _task->_time)
        {
            TRACE(dump::info,"start engine mix")
            g_pool.push(_task);
            _task = nullptr;
        }
    }
    return rt;
}

ret_type tracker_source::process()
{
    JCHK(_source,rc_state_invalid)

    _source->process();

    bool is_next;
    if(_stop != MEDIA_FRAME_NONE_TIMESTAMP)
        is_next = _time + _time_buf >= _stop;

    if(true == _source->is_eof() && false == is_next)
        is_next = true;

    if(false == is_next)
    {
        if(_time > _time_line)
        {
            bool except = true;
            _is_buf.compare_exchange_weak(except,false);
            return engine_task::rc_again;
        }
        else
            return rc_ok;
    }
    else
    {
        if(false == _eof)
        {
            _tracker->next_source(_it);
            _eof = true;
        }
        if(_buf.is_eof())
        {
            return engine_task::rc_eof;
        }
        else
            return rc_ok;
    }
}

ret_type tracker_source::set_source(source_ptr source,media_ptr mt,int64_t start,int64_t stop)
{
    JCHK(source,rc_param_invalid)
    JCHK(mt,rc_param_invalid)

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
    _source = source;
    return rt;
}

ret_type tracker_source::pop(engine_task* task,frame_ptr& frame)
{
    JCHK(nullptr != task,rc_param_invalid)

    bool ok;
    _time_line = task->_time;
    frame_ptr frame_out;
    while((ok = _buf.peek(frame_out)) && frame_out)
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
    if(true == ok)
    {
        if(_time <= _time_line)
        {
            bool except = false;
            if(_is_buf.compare_exchange_weak(except,true))
            {
                g_pool.push(this);
            }
        }
        if(frame_out)
        {
            if(frame_out->_info.pts == _time_line)
                frame = frame_out;
            return rc_ok;
        }
        else
        {
            TRACE(dump::info,"end engine mix")
            _task = task;
            return engine_task::rc_again;
        }
    }
    else
        return engine_task::rc_eof;
}
