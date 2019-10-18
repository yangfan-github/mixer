#include "engine_source.h"
tracker_mixer::tracker_mixer(engine_source* source)
:output_pin(source)
,_source(source)
,_it_tracker(_trackers.end())
{

}

tracker_mixer::~tracker_mixer()
{
}

ret_type tracker_mixer::load(property_tree::ptree& pt_mixer)
{
    ret_type rt;
    optional<property_tree::ptree&> pt_mt = pt_mixer.get_child_optional("media_type");
    optional<property_tree::ptree&> pt_outputs = pt_mixer.get_child_optional("outputs");
    optional<property_tree::ptree&> pt_trackers = pt_mixer.get_child_optional("trackers");

    JCHK(pt_mt,rc_param_invalid)
    JCHK(pt_trackers,rc_param_invalid)
    JCHK(!pt_outputs.value().empty(),rc_param_invalid)
    JCHK(!pt_trackers.value().empty(),rc_param_invalid)

    std::shared_ptr<media_type> mt(new media_type());
    JCHK(mt,rc_new_fail)
    JIF(mt->load(pt_mt.value()))

    MediaMajorType major = mt->get_major();
    JCHK(MMT_VIDEO == major || MMT_AUDIO == major,rc_param_invalid)

    if(MMT_VIDEO == major)
    {
        mt->set_sub(MST_RAWVIDEO);
        mt->set_video_format(VMT_ARGB);
    }
    else if(MMT_AUDIO == major)
    {
        mt->set_sub(MST_PCM);
        mt->set_audio_format(AMT_FLT);
    }
    JIF(set_media_type(mt.get()))

    BOOST_FOREACH(property_tree::ptree::value_type &pt_output, pt_outputs.value())
    {
        std::shared_ptr<media_type> mt_out(new media_type());
        JCHK(mt_out,rc_new_fail)
        JIF(mt_out->load(pt_output.second))
        JCHK(mt_out->get_major() == mt->get_major(),rc_param_invalid)
        JIF(media_type::copy(mt_out.get(),mt.get(),true))
        JCHK(_outputs.insert(OutputPair(pt_output.first,mt_out)).second,rc_param_invalid)
    }

    BOOST_FOREACH(property_tree::ptree::value_type &pt_tracker, pt_trackers.value())
    {
        std::shared_ptr<engine_tracker> tracker(new engine_tracker(this));
        JCHK(tracker,rc_new_fail)
        JCHK(_trackers.insert(TrackerPair(pt_tracker.first,tracker)).second,rc_param_invalid)
        JIF(tracker->load(pt_tracker.second))
    }

    return rt;
}

media_type* tracker_mixer::find(const string& path)
{
    OutputIt it = _outputs.find(path);
    JCHKR(it != _outputs.end(),rc_param_invalid,nullptr)
    return it->second.get();
}

ret_type tracker_mixer::append(string& path,property_tree::ptree& segment)
{
    TrackerIt it = _trackers.find(path);
    JCHK(it != _trackers.end(),rc_param_invalid)
    return it->second->append(segment);
}

ret_type tracker_mixer::process()
{
    if(is_connect())
    {
        if(!_frame)
        {
            _frame.reset(new media_frame());
            JCHK(_frame,rc_new_fail)
        }
        if(_trackers.end() == _it_tracker)
        {

            do
            {

            }while(_it_tracker != _trackers.end());
            deliver(_frame.get());
            _frame.reset();
        }
    }
    return rc_ok;
}

engine_source::engine_source(mixer_engine* engine)
:_engine(engine)
{
    //ctor
}

engine_source::~engine_source()
{
    //dtor
}

ret_type engine_source::set_media_type(output_pin* pin,media_type* mt)
{
    return rc_ok;
}

ret_type engine_source::load(property_tree::ptree& pt)
{
    ret_type rt = rc_ok;
    BOOST_FOREACH(property_tree::ptree::value_type &pt_stream, pt)
    {
        std::shared_ptr<tracker_mixer> mixer(new tracker_mixer(this));
        JCHK(mixer,rc_new_fail)
        JCHK(_mixers.insert(MixerPair(pt_stream.first,mixer)).second,rc_param_invalid)
        JIF(mixer->load(pt_stream.second))
    }
    return rt;
}

tracker_mixer* engine_source::find(string& path)
{
    size_t slash = path.find('/');
    JCHKR(slash != string::npos,rc_param_invalid,nullptr)
    string key = path.substr(0,slash);
    path = path.substr(slash+1);
    MixerIt it = _mixers.find(key);
    JCHKR(it != _mixers.end(),rc_param_invalid,nullptr)
    return it->second.get();
}

ret_type engine_source::append(property_tree::ptree& segment)
{
    ret_type rt = rc_ok;

    optional<property_tree::ptree&> pt_trackers = segment.get_child_optional("trackers");
    JCHK(pt_trackers,rc_param_invalid)

    BOOST_FOREACH(property_tree::ptree::value_type &pt_tracker, pt_trackers.value())
    {
        string path = pt_tracker.second.get_value<string>();
        size_t slash = path.find('/');
        JCHK(slash != string::npos,rc_param_invalid)
        string key = path.substr(0,slash);
        path = path.substr(slash+1);
        MixerIt it = _mixers.find(key);
        JCHK(it != _mixers.end(),rc_param_invalid)
        JIF(it->second->append(path,segment))
    }
    return rt;
}

//ret_type engine_source::load(engine_tracker* tracker,const string& url)
//{
//    return rc_ok;
//}
