#include "engine_tracker.h"
#include "mixer_engine.h"

engine_tracker::engine_tracker(tracker_mixer* mixer)
:_mixer(mixer)
,_mt(media_type::create())
,_pos_x(0)
,_pos_y(0)
,_it_segment(_segments.end())
,_eof(false)
{
    //ctor
}

engine_tracker::~engine_tracker()
{
    //dtor
}

ret_type engine_tracker::load(property_tree::ptree& pt)
{
    media_ptr mt_mixer = _mixer->get_media_type();
    MediaMajorType major = mt_mixer->get_major();
    if(MMT_VIDEO == major)
    {
        optional<int> pos_x = pt.get_optional<int>("pos_x");
        optional<int> pos_y = pt.get_optional<int>("pos_y");
        optional<int> pos_cx = pt.get_optional<int>("pos_cx");
        optional<int> pos_cy = pt.get_optional<int>("pos_cy");

        int x = pos_x ? pos_x.value() : 0;
        JCHK(0 <= x && x <= mt_mixer->get_video_width() - media_type::MIN_VIDEO_WIDTH,rc_param_invalid)
        int y = pos_y ? pos_y.value() : 0;
        JCHK(0 <= y && y <= mt_mixer->get_video_height() - media_type::MIN_VIDEO_HEIGHT,rc_param_invalid)

        int cx;
        if(pos_cx)
        {
            cx = pos_cx.value();
            JCHK(0 <= cx && cx <= mt_mixer->get_video_width() - x && media_type::MIN_VIDEO_WIDTH <= cx,rc_param_invalid)
        }
        else
            cx = mt_mixer->get_video_width() - x;

        int cy;
        if(pos_cy)
        {
            cy = pos_cy.value();
            JCHK(0 <= cy && cy <= mt_mixer->get_video_height() - y && media_type::MIN_VIDEO_HEIGHT <= cy,rc_param_invalid)
        }
        else
            cy = mt_mixer->get_video_height() - y;

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

ret_type engine_tracker::add_segment(property_tree::ptree& pt)
{
    optional<int64_t> time_base = pt.get_optional<int64_t>("time_base");
    JCHK(time_base,rc_param_invalid)
    segment seg;
    seg._pt = pt;
    JCHK(_segments.insert(SegmentPair(time_base.value(),seg)).second,rc_param_invalid)
    return rc_ok;
}

ret_type engine_tracker::add_source(source_ptr source,int64_t time_base,int64_t start)
{
    SegmentIt it = _segments.find(time_base);
    JCHK(it != _segments.end(),rc_param_invalid)

    JCHK(!it->second._source,rc_state_invalid)

    track_source_ptr tsource(new tracker_source(this,it,_mixer->_source->_time_buf));
    JCHK(tsource,rc_new_fail)

    int64_t stop;
    optional<int64_t> duration = it->second._pt.get_optional<int64_t>("duration");
    if(duration)
        stop = start + duration.value() * 10000;
    else
        stop = MEDIA_FRAME_NONE_TIMESTAMP;
    SegmentIt next_it = it;
    ++next_it;
    if(next_it != _segments.end())
    {
        if(MEDIA_FRAME_NONE_TIMESTAMP == stop || next_it->first < stop)
            stop = next_it->first * 10000;
    }
    ret_type rt;
    JIF(tsource->set_source(source,_mt,start,stop))
    it->second._source = tsource;
    return rt;
}

ret_type engine_tracker::next_source(SegmentIt& it)
{
    ret_type rt = rc_ok;

    if(_segments.end() == it)
        it = _segments.begin();
    else
        ++it;

    if(it == _segments.end())
        return engine_task::rc_eof;

    {
        unique_lock<std::mutex> lck(_mixer->_source->_mt_tracker);

        if(!it->second._source)
        {
            JIF(_mixer->_source->add_segment(it))
        }
     }
    return rt;
}

int64_t engine_tracker::get_time_base()
{
    SegmentIt it = _segments.begin();
    return it == _segments.end() ? MEDIA_FRAME_NONE_TIMESTAMP : it->first;
}

ret_type engine_tracker::process(engine_task* task,frame_ptr frame,uint8_t** dst_data,int* dst_linesize)
{
    if(true == _eof)
        return engine_task::rc_eof;

    ret_type rt;

    if(!_source)
    {
        rt = next_source(_it_segment);
        if(rt != rc_ok)
        {
            _eof = true;
            return engine_task::rc_eof;
        }
        JCHK(_source = _it_segment->second._source,rc_state_invalid)
    }

    rt = _source->pop(task,_frame);
    if(rc_ok == rt)
    {
        if(_frame)
        {

            uint8_t *data_sour[8];
            int linesize_sour[8];

            JIF(convert_frame_to_array(_mt,_frame,data_sour,linesize_sour))

            MediaMajorType major = _mt->get_major();

            if(MMT_VIDEO == major)
            {
                uint8_t* dest = dst_data[0];
                uint8_t* sour = data_sour[0];

                int line_dest = 4 * frame->_info.stride;
                int height_sour = _mt->get_video_height();
                int line_sour = 4 * _frame->_info.stride;

                dest += line_dest * _pos_y;
                for(int i = 0 ; i < height_sour ; ++i)
                {
                    memcpy(dest + 4 * _pos_x,sour,line_sour);
                    sour += line_sour;
                    dest += line_dest;
                }
            }
            else if(MMT_AUDIO == major)
            {
                int16_t* dest = (int16_t*)dst_data[0];
                int16_t* sour = (int16_t*)data_sour[0];
                int len = dst_linesize[0]/sizeof(int16_t);
                for(int i = 0 ; i < len ; ++i)
                {
                    if( dest[i] < 0 && sour[i] < 0)
                        dest[i] = dest[i] + sour[i] + (dest[i] * sour[i] / 32767);
                    else
                        dest[i] = dest[i] + sour[i] - (dest[i] * sour[i] / 32767);
                }
            }
        }
    }
    else if(engine_task::rc_eof == rt)
    {
        _source.reset();
        _it_segment->second._source.reset();
        return process(task,frame,dst_data,dst_linesize);
    }
    return rt;
}
