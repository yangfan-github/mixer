#ifndef ENGINE_TRACKER_H
#define ENGINE_TRACKER_H
#include "stdafx.h"

class tracker_mixer;
class engine_tracker : public std::enable_shared_from_this<engine_tracker>
{
        typedef list<string> SourceSet;
        typedef SourceSet::iterator SourceSetIt;
    protected:
        tracker_mixer* _mixer;
        SourceSet _sources;
        std::shared_ptr<media_type> _mt;
        int _pos_x;
        int _pos_y;
    public:
        engine_tracker(tracker_mixer* mixer);
        virtual ~engine_tracker();
        ret_type load(property_tree::ptree& pt);
        ret_type append(property_tree::ptree& pt);
        media_type* get_media_type();
        media_frame* get_media_frame();
    protected:
};

#endif // ENGINE_TRACKER_H
