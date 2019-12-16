#include "engine_source.h"
tracker_mixer::tracker_mixer(engine_source* source)
:output_pin(source)
,_source(source)
,_it_tracker(_trackers.end())
,_count_eof(0)
,_eof(false)
{
    g_dump.set_class("tracker_mixer");
}

tracker_mixer::~tracker_mixer()
{
}

void tracker_mixer::set_duration(int64_t duration)
{
    for(OutputIt it =_outputs.begin() ; it != _outputs.end() ; ++it)
    {
        it->second->set_duration(duration);
    }
    for(TrackerIt it = _trackers.begin() ; it != _trackers.end() ; ++ it)
    {
        it->second->_mt->set_duration(duration);
    }
    _mt->set_duration(duration);
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

    _mt = media_type::create();
    JCHK(_mt,rc_new_fail)
    JIF(_mt->load(pt_mt.value()))

    MediaMajorType major = _mt->get_major();
    JCHK(MMT_VIDEO == major || MMT_AUDIO == major,rc_param_invalid)

    if(MMT_VIDEO == major)
    {
        _mt->set_sub(MST_RAWVIDEO);
        _mt->set_video_format(VMT_BGRA);
    }
    else if(MMT_AUDIO == major)
    {
        _mt->set_sub(MST_PCM);
        _mt->set_audio_format(AMT_S16);
    }
    BOOST_FOREACH(property_tree::ptree::value_type &pt_output, pt_outputs.value())
    {
        media_ptr mt_out = media_type::create();
        JCHK(mt_out,rc_new_fail)
        JIF(mt_out->load(pt_output.second))
        JCHK(mt_out->get_major() == _mt->get_major(),rc_param_invalid)
        JIF(media_type::copy(mt_out,_mt,true))
        JCHKM(_outputs.insert(OutputPair(pt_output.first,mt_out)).second,rc_param_invalid,FORMAT_STR("mixer's output is exist,name=%1%",%pt_output.first))
    }

    BOOST_FOREACH(property_tree::ptree::value_type &pt_tracker, pt_trackers.value())
    {
        std::shared_ptr<engine_tracker> tracker(new engine_tracker(this));
        JCHK(tracker,rc_new_fail)
        JCHKM(_trackers.insert(TrackerPair(pt_tracker.first,tracker)).second,rc_param_invalid,FORMAT_STR("mixer's tracker is exist,name=%1%",%pt_tracker.first))
        JIF(tracker->load(pt_tracker.second))
    }
    return rt;
}

media_ptr tracker_mixer::find_output(const string& path)
{
    OutputIt it = _outputs.find(path);
    JCHKR(it != _outputs.end(),rc_param_invalid,nullptr)
    return it->second;
}

TrackerType tracker_mixer::find_tracker(const string& path)
{
    TrackerIt it = _trackers.find(path);
    JCHKR(it != _trackers.end(),rc_param_invalid,nullptr)
    return it->second;
}

int64_t tracker_mixer::get_time_base()
{
    int64_t time_base = MEDIA_FRAME_NONE_TIMESTAMP;
    for(TrackerIt it = _trackers.begin() ; it != _trackers.end() ; ++it)
    {
        int64_t time = it->second->get_time_base();
        if(time < time_base || time_base == MEDIA_FRAME_NONE_TIMESTAMP)
            time_base = time;
    }
    _eof = false;
    return time_base;
}

ret_type tracker_mixer::process(media_task* task)
{
    if(is_connect())
    {
        if(_eof)
            deliver(_frame);
        else
        {
            ret_type rt;
            if(!_frame)
            {
                _frame = media_frame::create();
                JCHK(_frame,rc_new_fail)
                _frame->_info.duration = _mt->get_duration();
                _frame->_info.dts = task->_time;
                _frame->_info.pts = task->_time;
                JIF(convert_frame_to_array(_mt,_frame,_data_dest,_linesize_dest))
                _count_eof = 0;
            }
            if(_trackers.end() == _it_tracker)
                _it_tracker = _trackers.begin();
            while(_it_tracker != _trackers.end())
            {
                rt = _it_tracker->second->process(task,_frame,_data_dest,_linesize_dest);
                if(rt == media_task::rc_again)
                    return rt;
                else
                {
                    if(rt == media_task::rc_eof)
                        ++_count_eof;
                    ++_it_tracker;
                }
            }
            if(_count_eof == _trackers.size())
                _eof = true;
            else
                deliver(_frame);
            _frame.reset();
        }
    }
    else if(!_eof)
        _eof = true;

    if(_eof)
        return media_task::rc_eof;
    else
        return rc_ok;
}

engine_source::engine_source(mixer_engine* engine)
:_engine(engine)
,_duration(0)
,_it_mixer(_mixers.end())
,_time_base(MEDIA_FRAME_NONE_TIMESTAMP)
,_time_buf(5000000)
,_count_eof(0)
{
    //ctor
    g_dump.set_class("engine_source");
}

engine_source::~engine_source()
{
    //dtor
}

ret_type engine_source::set_media_type(output_pin* pin,media_ptr mt)
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
        JCHKM(_mixers.insert(MixerPair(pt_stream.first,mixer)).second,rc_param_invalid,FORMAT_STR("mixer is exist,name-=%1%",%pt_stream.first))
        JIF(mixer->load(pt_stream.second))
    }
    _duration = 0;
    for(MixerIt it = _mixers.begin() ; it != _mixers.end() ; ++it)
    {
        int64_t duration = it->second->get_media_type()->get_duration();
        if(0 < duration && (0 == _duration || duration < _duration))
            _duration = duration;
    }

    for(MixerIt it = _mixers.begin() ; it != _mixers.end() ; ++it)
    {
        it->second->set_duration(_duration);
    }

    _it_mixer = _mixers.end();
    return rt;
}

std::shared_ptr<tracker_mixer> engine_source::find(string& path)
{
    size_t slash = path.find('/');
    JCHKR(slash != string::npos,rc_param_invalid,nullptr)
    string key = path.substr(0,slash);
    path = path.substr(slash+1);
    MixerIt it = _mixers.find(key);
    JCHKMR(it != _mixers.end(),rc_param_invalid,FORMAT_STR("can not find mixer,name=%1%",%path),nullptr)
    return it->second;
}

void engine_source::get_time_base()
{
    _time_base = MEDIA_FRAME_NONE_TIMESTAMP;
    for(MixerIt it = _mixers.begin() ; it != _mixers.end() ; ++it)
    {
        int64_t time = it->second->get_time_base();
        if(time < _time_base || MEDIA_FRAME_NONE_TIMESTAMP == _time_base)
            _time_base = time;
    }
}

ret_type engine_source::append(property_tree::ptree& segment)
{
    optional<property_tree::ptree&> pt_trackers = segment.get_child_optional("trackers");
    JCHK(pt_trackers,rc_param_invalid)
    ret_type rt = rc_ok;
    BOOST_FOREACH(property_tree::ptree::value_type &pt_tracker, pt_trackers.value())
    {
        string path = pt_tracker.second.get_value<string>();
        size_t slash = path.find('/');
        JCHKM(slash != string::npos,rc_param_invalid,FORMAT_STR("invalid segment path,path=%1%",%path))
        string key = path.substr(0,slash);
        path = path.substr(slash+1);
        MixerIt it_mixer = _mixers.find(key);
        JCHKM(it_mixer != _mixers.end(),rc_param_invalid,FORMAT_STR("can not find segment mixer,name=%1%",%key))
        TrackerType tracker = it_mixer->second->find_tracker(path);
        JCHKM(tracker,rc_param_invalid,FORMAT_STR("can not find segment tracker,name=%1%",%path))
        JIF(tracker->add_segment(segment))
    }
    return rt;
}

ret_type engine_source::process(media_task* task)
{
    if(_mixers.end() == _it_mixer)
    {
        _it_mixer = _mixers.begin();
        _count_eof = 0;
    }

    while(_it_mixer != _mixers.end())
    {
        ret_type rt = _it_mixer->second->process(task);
        if(rt == media_task::rc_again)
            return rt;
        else
        {
            if(rt == media_task::rc_eof)
                ++_count_eof;
            ++_it_mixer;
        }
    }

    if(_count_eof == _mixers.size())
        return media_task::rc_eof;
    else
        return  rc_ok;
}

ret_type engine_source::add_segment(SegmentIt it)
{
    ret_type rt;
    optional<string> url = it->second._pt.get_optional<string>("url");
    JCHK(url && !url.value().empty(),rc_param_invalid)
    optional<property_tree::ptree&> pt_trackers = it->second._pt.get_child_optional("trackers");
    JCHK(pt_trackers,rc_param_invalid)

    source_ptr source = create_filter<media_source>(url.value().c_str());
    JCHK(source,rc_param_invalid)
    JIFM(source->open(url.value()),FORMAT_STR("segment source open fail,will be ignore,url=%1%",%url.value()))
    int64_t time_base = (it->first - _time_base)*10000;
    source->set_base(time_base);

    BOOST_FOREACH(property_tree::ptree::value_type &pt_tracker, pt_trackers.value())
    {
        string path = pt_tracker.second.get_value<string>();
        size_t slash = path.find('/');
        JCHK(slash != string::npos,rc_param_invalid)
        string key = path.substr(0,slash);
        path = path.substr(slash+1);
        MixerIt it_mixer = _mixers.find(key);
        JCHK(it_mixer != _mixers.end(),rc_param_invalid)
        TrackerType tracker = it_mixer->second->find_tracker(path);
        JCHK(tracker,rc_param_invalid)
        JIF(tracker->add_source(source,it->first,time_base))
    }
    return rt;
}

//ret_type engine_source::load(engine_tracker* tracker,const string& url)
//{
//    return rc_ok;
//}
