#include "engine_tracker.h"
#include "mixer_engine.h"

engine_tracker::engine_tracker(tracker_mixer* mixer)
:_mixer(mixer)
,_pos_x(0)
,_pos_y(0)
{
    //ctor
}

engine_tracker::~engine_tracker()
{
    //dtor
}

ret_type engine_tracker::load(property_tree::ptree& pt)
{
    ret_type rt;
    media_type* mt = _mixer->get_media_type();
    JCHK(nullptr != mt,rc_state_invalid)
    _mt.reset(new media_type());
    JCHK(_mt,rc_new_fail)
    JIF(media_type::copy(_mt.get(),mt))
    MediaMajorType major = mt->get_major();
    if(MMT_VIDEO == major)
    {
        optional<int> pos_x = pt.get_optional<int>("pos_x");
        optional<int> pos_y = pt.get_optional<int>("pos_y");
        optional<int> pos_cx = pt.get_optional<int>("pos_cx");
        optional<int> pos_cy = pt.get_optional<int>("pos_cy");

        int x = pos_x ? pos_x.value() : 0;
        JCHK(0 <= x && x <= _mt->get_video_width() - media_type::MIN_VIDEO_WIDTH,rc_param_invalid)
        int y = pos_y ? pos_y.value() : 0;
        JCHK(0 <= y && y <= _mt->get_video_height() - media_type::MIN_VIDEO_HEIGHT,rc_param_invalid)

        int cx;
        if(pos_cx)
        {
            cx = pos_cx.value();
            JCHK(0 <= cx && cx <= _mt->get_video_width() - x && media_type::MIN_VIDEO_WIDTH <= cx,rc_param_invalid)
        }
        else
            cx = _mt->get_video_width() - x;

        int cy;
        if(pos_cy)
        {
            cy = pos_cy.value();
            JCHK(0 <= cy && cy <= _mt->get_video_height() - y && media_type::MIN_VIDEO_HEIGHT <= cy,rc_param_invalid)
        }
        else
            cy = _mt->get_video_height() - y;

        _pos_x = x;
        _pos_y = y;
        _mt->set_video_width(cx);
        _mt->set_video_height(cy);
    }
    else if(MMT_AUDIO == major)
    {

    }
    return rc_ok;
}

ret_type engine_tracker::append(property_tree::ptree& pt)
{
    return rc_ok;
}

media_type* engine_tracker::get_media_type()
{
    return _mt.get();
}
