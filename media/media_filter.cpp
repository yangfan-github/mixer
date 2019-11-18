#include "../inc/media_filter.h"
#include "global.h"

media_pin::media_pin(media_filter* filter)
:_filter(filter)
{

}

media_pin::~media_pin()
{
}

ret_type media_pin::set_media_type(media_ptr mt)
{
    _mt = mt;
    return rc_ok;
}

media_ptr media_pin::get_media_type()
{
    return _mt;
}

filter_ptr media_pin::get_filter()
{
    return _filter->shared_from_this();
}

input_pin::input_pin(media_filter* filter)
:media_pin(filter)
,_pin(nullptr)
{

}

input_pin::~input_pin()
{
    disconnect();
}

ret_type input_pin::set_media_type(media_ptr mt)
{
    ret_type rt;
    JIF(_filter->set_media_type(this,mt))
    return media_pin::set_media_type(mt);
}

ret_type input_pin::deliver(frame_ptr frame)
{
    JCHK(is_connect(),rc_state_invalid)
    ret_type rt = _filter->process(this,frame);
    if(rc_state_invalid == rt)
        rt = _buf.push(frame);
    return rt;
}

void input_pin::disconnect()
{
    if(nullptr != _pin)
    {
        output_pin* pin = _pin;
        _buf.reset();
        _pin = nullptr;
        pin->disconnect(_it);
    }
}

bool input_pin::is_connect()
{
    return nullptr != _pin;
}

bool input_pin::peek(frame_ptr& frame)
{
    return _buf.peek(frame);
}

bool input_pin::pop()
{
    return _buf.pop();
}

bool input_pin::eof()
{
    return _buf.is_eof();
}

ret_type input_pin::connect(output_pin* pin,It it)
{
    JCHK(nullptr != pin,rc_param_invalid)
    ret_type rt = rc_ok;
    if(pin == _pin)
        _it = it;
    else
    {
        disconnect();
        _pin = pin;
        _it = it;
    }
    return rt;
}

output_pin::output_pin(media_filter* filter)
:media_pin(filter)
{

}

output_pin::~output_pin()
{
    disconnect_all();
}

ret_type output_pin::set_media_type(media_ptr mt)
{
    ret_type rt;
    JIF(_filter->set_media_type(this,mt))
    for(input_pin::It it = _pins.begin() ; it != _pins.end() ; ++it)
    {
        JIF((*it)->set_media_type(mt));
    }
    return media_pin::set_media_type(mt);
}

ret_type output_pin::deliver(frame_ptr frame)
{
    for(input_pin::It it = _pins.begin() ; it != _pins.end() ; ++it)
    {
        (*it)->deliver(frame);
    }
    return rc_ok;
}

ret_type output_pin::connect(input_pin_ptr pin,media_ptr mt)
{
    JCHK(pin,rc_param_invalid)

    ret_type rt;
    if(mt)
    {
        JIF(set_media_type(mt))
        JIF(pin->set_media_type(mt))
    }
    else if(_mt)
    {
        JIF(pin->set_media_type(_mt))
    }
    else
    {
        JCHK(mt = pin->get_media_type(),rc_param_invalid)
        JIF(set_media_type(mt))
    }
    return pin->connect(this,_pins.insert(_pins.end(),pin));
}

void output_pin::disconnect_all()
{
    input_pin::It it;
    while((it = _pins.begin()) != _pins.end())
    {
        (*it)->disconnect();
    }
}

bool output_pin::is_connect()
{
    return false == _pins.empty();
}

void output_pin::disconnect(input_pin::It it)
{
    _pins.erase(it);
}

media_filter::media_filter()
{
    //ctor
}

media_filter::~media_filter()
{
    //dtor
}

ret_type media_filter::set_media_type(input_pin* pin,media_ptr mt)
{
    return rc_state_invalid;
}

ret_type media_filter::set_media_type(output_pin* pin,media_ptr mt)
{
    return rc_state_invalid;
}

ret_type media_filter::process(input_pin* pin,frame_ptr frame)
{
    return rc_state_invalid;
}

media_transform::media_transform()
:_pin_input(new input_pin(this))
,_pin_output(new output_pin(this))
{
}

media_transform::~media_transform()
{

}

input_pin_ptr media_transform::get_input_pin()
{
    return input_pin_ptr(_pin_input.get(),pin_deleter<input_pin>(shared_from_this()));
}

output_pin_ptr media_transform::get_output_pin()
{
    return output_pin_ptr(_pin_output.get(),pin_deleter<output_pin>(shared_from_this()));
}

media_source::media_source()
{

}

media_source::~media_source()
{

}

media_render::media_render()
{

}

media_render::~media_render()
{

}

