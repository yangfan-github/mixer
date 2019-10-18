#ifndef ENGINE_SOURCE_H
#define ENGINE_SOURCE_H
#include "stdafx.h"
#include "engine_tracker.h"


class engine_source;
class tracker_mixer : public output_pin
{
    protected:
        typedef std::shared_ptr<engine_tracker> TrackerType;
        typedef map<string,TrackerType> TrackerSet;
        typedef TrackerSet::iterator TrackerIt;
        typedef pair<TrackerSet::key_type,TrackerSet::mapped_type> TrackerPair;

        typedef std::shared_ptr<media_type> OutputType;
        typedef map<string,OutputType> OutputSet;
        typedef OutputSet::iterator OutputIt;
        typedef pair<OutputSet::key_type,OutputSet::mapped_type> OutputPair;
    protected:
        engine_source* _source;
        TrackerSet _trackers;
        OutputSet _outputs;
        TrackerIt _it_tracker;
        std::shared_ptr<media_frame> _frame;
    public:
        std::shared_ptr<media_type> _mt_output;
    public:
        tracker_mixer(engine_source* source);
        virtual ~tracker_mixer();
        ret_type load(property_tree::ptree& pt_mixer);
        media_type* find(const string& path);
        ret_type append(string& path,property_tree::ptree& segment);
        ret_type process();
};


class mixer_engine;
class engine_source :  public media_filter
{
    public:
        typedef std::shared_ptr<tracker_mixer> MixerType;
        typedef map<string,MixerType> MixerSet;
        typedef MixerSet::iterator MixerIt;
        typedef pair<MixerSet::key_type,MixerSet::mapped_type> MixerPair;
        MixerSet _mixers;
    protected:
        mixer_engine* _engine;
    public:
        engine_source(mixer_engine* engine);
        virtual ~engine_source();
        ret_type set_media_type(output_pin* pin,media_type* mt);
        ret_type load(property_tree::ptree& pt);
        tracker_mixer* find(string& path);
        ret_type append(property_tree::ptree& segment);
        //media_filter
        //ret_type set_media_type(input_pin* pin,media_type* mt);
};

#endif // ENGINE_SOURCE_H
