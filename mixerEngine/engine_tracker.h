#ifndef ENGINE_TRACKER_H
#define ENGINE_TRACKER_H
#include "stdafx.h"
#include "track_source.h"

class tracker_mixer;
class engine_tracker
{
        friend class tracker_mixer;
        friend class tracker_source;
    protected:
        tracker_mixer* _mixer;
        media_ptr _mt;
        int _pos_x;
        int _pos_y;
        SegmentSet _segments;
        SegmentIt  _it_segment;
        frame_ptr _frame;
        frame_ptr _background;
        track_source_ptr _source;
        bool _eof;
    public:
        engine_tracker(tracker_mixer* mixer);
        virtual ~engine_tracker();
        ret_type load(property_tree::ptree& pt);
        ret_type add_segment(property_tree::ptree& pt);
        int64_t get_time_base();
        media_ptr get_media();
        ret_type add_source(source_ptr source,int64_t time_base,int64_t start);
        ret_type next_source(SegmentIt& it);
        ret_type process(media_task* task,frame_ptr frame,uint8_t** dst_data,int* dst_linesize);
        ret_type process(frame_ptr dest_frame,uint8_t** dst_data,int* dst_linesize,frame_ptr sour_frame);
};

#endif // ENGINE_TRACKER_H
