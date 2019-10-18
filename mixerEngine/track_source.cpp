#include "track_source.h"

track_source::track_source(engine_tracker* tracker)
:_pin_input(new input_pin(this))
,_tracker(tracker)
{
    //ctor
}

track_source::~track_source()
{
    //dtor
    close();
}

ret_type track_source::open(const string& url)
{
    media_type* mt_in;
    JCHK(mt_in = _tracker->get_media_type(),rc_state_invalid);

    ret_type rt = rc_fail;
    JCHK(_source = create_filter<media_source>(url.c_str()),rc_param_invalid)

    uint32_t index = 0;
    while(true)
    {
        std::shared_ptr<output_pin> pin_out = _source->get_pin(index++);
        if(pin_out)
        {
            media_type* mt_out;

            JCHK(mt_out = pin_out->get_media_type(),rc_state_invalid);

            if(mt_in->get_major() == mt_out->get_major())
            {
                if(IS_OK(rt = connect(pin_out.get(),_pin_input.get(),nullptr,mt_in)))
                {
                    _buf.reset();
                    break;
                }
            }
        }
        else
            break;
    }
    return rt;
}
/*
ret_type track_source::process()
{
    JCHK(_source,rc_state_invalid)
    return _source->process((media_frame*)nullptr);
}*/

void track_source::close()
{
    _buf.reset();
    _source.reset();
}

ret_type track_source::set_media_type(input_pin* pin,media_type* mt)
{
    return rc_ok;
}

ret_type track_source::process(media_frame* frame)
{
    return _buf.push(frame);
}

