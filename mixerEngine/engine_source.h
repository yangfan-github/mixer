#ifndef ENGINE_SOURCE_H
#define ENGINE_SOURCE_H
#include "stdafx.h"
#include "engine_tracker.h"

typedef std::shared_ptr<engine_tracker> TrackerType;

class engine_source;
class tracker_mixer : public output_pin
{
        friend class engine_tracker;
        friend class tracker_source;
    protected:
        typedef map<string,TrackerType> TrackerSet;
        typedef TrackerSet::iterator TrackerIt;
        typedef pair<TrackerSet::key_type,TrackerSet::mapped_type> TrackerPair;

        typedef map<string,media_ptr> OutputSet;
        typedef OutputSet::iterator OutputIt;
        typedef pair<OutputSet::key_type,OutputSet::mapped_type> OutputPair;
    protected:
        engine_source* _source;
        TrackerSet _trackers;
        OutputSet _outputs;
        TrackerIt _it_tracker;
        frame_ptr _frame;
        size_t _count_eof;
        bool _eof;
        uint8_t *_data_dest[8];
        int _linesize_dest[8];
    public:
        bool _have_segments;
        media_ptr _mt_output;
    public:
        tracker_mixer(engine_source* source);
        virtual ~tracker_mixer();
        void set_duration(int64_t duration);
        ret_type load(property_tree::ptree& pt_mixer);
        media_ptr find_output(const string& path);
        TrackerType find_tracker(const string& path);
        int64_t get_time_base();
        int64_t get_time_end();
        ret_type process(media_task* task);
};
typedef std::shared_ptr<tracker_mixer> mixer_ptr;

class mixer_engine;
class engine_source :  public media_filter
{
        friend class tracker_mixer;
        friend class engine_tracker;
        friend class mixer_engine;
    protected:
        typedef map<string,mixer_ptr> MixerSet;
        typedef MixerSet::iterator MixerIt;
        typedef pair<MixerSet::key_type,MixerSet::mapped_type> MixerPair;

        MixerSet _mixers;
        mixer_engine* _engine;
        int64_t _duration;
        MixerIt _it_mixer;
        int64_t _time_base;
        int64_t _time_buf;
        std::mutex _mt_tracker;
        size_t _count_eof;
    public:
        engine_source(mixer_engine* engine);
        virtual ~engine_source();
        ret_type set_media_type(output_pin* pin,media_ptr mt);
        ret_type load(property_tree::ptree& pt);
        std::shared_ptr<tracker_mixer> find(string& path);
        int64_t get_time_base();
        int64_t get_time_end();
        ret_type append(property_tree::ptree& segment);
        ret_type process(media_task* task);
        ret_type add_segment(SegmentIt it);
};

#endif // ENGINE_SOURCE_H
