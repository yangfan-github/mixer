#include "../inc/media_filter.h"
#include "global.h"

media_pin::media_pin(media_filter* filter)
:_filter(filter)
{

}

media_pin::~media_pin()
{
}

ret_type media_pin::set_media_type(media_type* mt)
{
    _mt.reset(mt);
    return rc_ok;
}

media_type* media_pin::get_media_type()
{
    return _mt.get();
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

ret_type input_pin::set_media_type(media_type* mt)
{
    ret_type rt;
    JIF(_filter->set_media_type(this,mt))
    return media_pin::set_media_type(mt);
}

ret_type input_pin::deliver(media_frame* frame)
{
    JCHK(is_connect(),rc_state_invalid)
    ret_type rt = _filter->process(this,frame);
    if(rc_state_invalid == rt)
        rt = _buf.push(frame);
    return rt;
}

ret_type input_pin::connect(output_pin* pin,It it)
{
    JCHK(nullptr != pin,rc_param_invalid)
    ret_type rt = rc_ok;
    if(pin == _pin)
        _it = it;
    else
    {
        JIF(set_media_type(pin->get_media_type()))
        disconnect();
        _pin = pin;
        _it = it;
    }
    return rt;
}

void input_pin::disconnect()
{
    if(nullptr != _pin)
    {
        _pin->disconnect(_it);
        _pin = nullptr;
        set_media_type(nullptr);
        _buf.reset();
    }
}

bool input_pin::is_connect()
{
    return nullptr != _pin;
}

bool input_pin::peek(std::shared_ptr<media_frame>& frame)
{
    return _buf.peek(frame);
}

bool input_pin::pop()
{
    return _buf.pop();
}

output_pin::output_pin(media_filter* filter)
:media_pin(filter)
{

}

output_pin::~output_pin()
{
    disconnect_all();
}

ret_type output_pin::set_media_type(media_type* mt)
{
    ret_type rt;
    JIF(_filter->set_media_type(this,mt))
    return media_pin::set_media_type(mt);
}

ret_type output_pin::deliver(media_frame* frame)
{
    for(input_pin::It it = _pins.begin() ; it != _pins.end() ; ++it)
    {
        (*it)->deliver(frame);
    }
    return rc_ok;
}

ret_type output_pin::connect(std::shared_ptr<input_pin> pin,media_type* mt)
{
    JCHK(pin,rc_param_invalid)
    if(nullptr != mt)
    {
        ret_type rt;
        JIF(set_media_type(mt))
    }
    return pin->connect(this,_pins.insert(_pins.end(),pin));
}

void output_pin::disconnect(input_pin::It& it)
{
    _pins.erase(it);
    it = _pins.end();
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

media_filter::media_filter()
{
    //ctor
}

media_filter::~media_filter()
{
    //dtor
}

ret_type media_filter::set_media_type(input_pin* pin,media_type* mt)
{
    return rc_state_invalid;
}

ret_type media_filter::set_media_type(output_pin* pin,media_type* mt)
{
    return rc_state_invalid;
}

ret_type media_filter::process(input_pin* pin,media_frame* frame)
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

std::shared_ptr<input_pin> media_transform::get_input_pin()
{
    return std::shared_ptr<input_pin>(_pin_input.get(),pin_deleter<input_pin>(this));
}

std::shared_ptr<output_pin> media_transform::get_output_pin()
{
    return std::shared_ptr<output_pin>(_pin_output.get(),pin_deleter<output_pin>(this));
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

