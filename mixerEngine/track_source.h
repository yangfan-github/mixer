#ifndef TRACK_SOURCE_H
#define TRACK_SOURCE_H

#include "stdafx.h"

class tracker_source;
typedef std::shared_ptr<tracker_source> track_source_ptr;
struct segment
{
    property_tree::ptree _pt;
    track_source_ptr _source;
};

typedef map<int64_t,segment> SegmentSet;
typedef SegmentSet::iterator SegmentIt;
typedef pair<SegmentSet::key_type,SegmentSet::mapped_type> SegmentPair;

class engine_tracker;
class tracker_source : public media_filter , public engine_task
{
    protected:
        engine_tracker* _tracker;
        SegmentIt _it;
        int64_t _time_buf;
        media_frame_buf _buf;
        source_ptr _source;
        input_pin_ptr _pin;
        bool _eof;
        engine_task* _task;
        int64_t _time_line;
        std::atomic<bool> _is_buf;
        int64_t _start;
        int64_t _stop;
    public:
        tracker_source(engine_tracker* tracker,SegmentIt it,int64_t time_buf);
        virtual ~tracker_source();
        ret_type set_media_type(input_pin* pin,media_ptr mt);
        ret_type process(input_pin* pin,frame_ptr frame);
        ret_type process();
        ret_type set_source(source_ptr source,media_ptr mt,int64_t start,int64_t stop);
        ret_type pop(engine_task* task,frame_ptr& frame);
};



#endif // TRACK_SOURCE_H
