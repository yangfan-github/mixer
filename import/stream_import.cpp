#include "stream_import.h"
const int64_t EXCEPTION_WAIT_DRRATION = 50000000;

stream_import::stream_import(const char* sour,const char* dest)
:_sour(sour)
,_dest(dest)
,_exit(true)
{
    //ctor
    bool except = true;
    if(_exit.compare_exchange_weak(except,false))
    {
        g_pool.post(this);
    }
}

stream_import::~stream_import()
{
    //dtor
    bool except = false;
    if(_exit.compare_exchange_weak(except,true))
    {
        _mt_wait.lock();
         if(_source)
           _source->exit();
        std::unique_lock<std::timed_mutex> lck(_mt_wait);
    }
}

ret_type stream_import::run()
{
    ret_type rt = rc_ok;
    if(!_source)
    {
        _source = create_filter<media_source>(_sour.c_str());
        JCHK(_source,rc_param_invalid)
    }
    if(!_render)
    {
        _render = create_filter<media_render>(_dest.c_str());
        JCHK(_render,rc_param_invalid)
    }
    if(!_source->is_open())
    {
        JIF(_source->open(_sour))
    }
    if(!_render->is_open())
    {
        uint32_t index = 0;
        while(true)
        {
            output_pin_ptr pin_out = _source->get_pin(index++);
            if(pin_out)
            {
                if(false == pin_out->is_connect())
                {
                    input_pin_ptr pin_in;
                    media_ptr mt = pin_out->get_media_type();
                    JCHK(pin_in = _render->create_pin(mt),rc_param_invalid);
                    JIF(connect(pin_out,pin_in));
                }
            }
            else
                break;
        }
        JIF(_render->open(_dest));
        _is_live = false;
    }
    _source->process();
    if(_render->is_eof())
    {
        _source.reset();
        _render.reset();
        return media_task::rc_eof;
    }
    return rt;
}

ret_type stream_import::process()
{
    if(_exit)
    {
        if(!_mt_wait.try_lock())
            _mt_wait.unlock();
        return media_task::rc_eof;
    }
    if(IS_FAIL(run()))
    {
        if(!_exit)
        {
            _is_live = true;
            _time = get_local_time();
            _time += EXCEPTION_WAIT_DRRATION;
        }
    }
    else
    {
        _time = _render->get_time();
        //TRACE(dump::info,FORMAT_STR("push time:%1%ms",%(_time/10000)))
    }
    return rc_ok;
}
