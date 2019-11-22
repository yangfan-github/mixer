#include <algorithm>
#include "mixer_engine.h"

mixer_engine::mixer_engine()
:_eof(false)
{
    //ctor
}

mixer_engine::~mixer_engine()
{
    //dtor
}

ret_type mixer_engine::load(const char* template_file)
{
    JCHK(nullptr != template_file,rc_param_invalid)
    try
    {
        stringstream stream(template_file);
        property_tree::ptree pt_soucre;
        property_tree::json_parser::read_json(stream, pt_soucre);
        JCHK(false == pt_soucre.empty(),rc_param_invalid)
        if(!_source)
        {
            _source.reset(new engine_source(this));
            JCHK(_source,rc_new_fail)
        }
        return _source->load(pt_soucre);
    }
    catch (boost::property_tree::json_parser::json_parser_error& e)
    {
        JCHKM(false,rc_param_invalid,e.what())
    }
    catch (boost::property_tree::ptree_bad_path& e)
    {
        JCHKM(false,rc_param_invalid,e.what())
    }
    catch (boost::property_tree::ptree_bad_data& e)
    {
        JCHKM(false,rc_param_invalid,e.what())
    }
    catch(...)
    {
        JCHKM(false,rc_param_invalid,FORMAT_STR("load template file:[%1%] fial",%template_file))
    }
}

ret_type mixer_engine::run(const char* task_file)
{
    JCHK(nullptr != task_file,rc_param_invalid)
    JCHK(_source,rc_state_invalid)
    ret_type rt = rc_ok;

    try
    {
        stringstream stream(task_file);
        property_tree::ptree pt_task;
        property_tree::json_parser::read_json(stream, pt_task);
        JCHK(false == pt_task.empty(),rc_param_invalid)

        optional<property_tree::ptree&> pt_outputs = pt_task.get_child_optional("outputs");
        optional<property_tree::ptree&> pt_segments = pt_task.get_child_optional("inputs");
        JCHK(pt_outputs,rc_param_invalid)
        JCHK(pt_segments,rc_param_invalid)
        JCHK(false == pt_outputs.value().empty(),rc_param_invalid)
        JCHK(false == pt_segments.value().empty(),rc_param_invalid)

        BOOST_FOREACH(property_tree::ptree::value_type &pt_output, pt_outputs.value())
        {
            render_ptr render = create_filter<media_render>(pt_output.first.c_str());
            JCHK(render,rc_param_invalid)
            BOOST_FOREACH(property_tree::ptree::value_type &pt_stream, pt_output.second)
            {
                mixer_ptr mixer;
                media_ptr mt;
                input_pin_ptr pin;
                string path = pt_stream.second.get_value<string>();
                JCHK(mixer = _source->find(path),rc_param_invalid)
                JCHK(mt = mixer->find_output(path),rc_param_invalid)
                JCHK(pin = render->create_pin(mt),rc_param_invalid);
                JIF(connect(std::dynamic_pointer_cast<output_pin>(mixer),pin));
            }
            JIF(render->open(pt_output.first))
            _renders.insert(_renders.end(),render);
        }
        BOOST_FOREACH(property_tree::ptree::value_type &pt_segment, pt_segments.value())
        {
            JIF(_source->append(pt_segment.second))
        }
    }
    catch (boost::property_tree::json_parser::json_parser_error& e)
    {
        JCHKM(false,rc_param_invalid,e.what())
    }
    catch (boost::property_tree::ptree_bad_path& e)
    {
        JCHKM(false,rc_param_invalid,e.what())
    }
    catch (boost::property_tree::ptree_bad_data& e)
    {
        JCHKM(false,rc_param_invalid,e.what())
    }
    catch(...)
    {
        JCHKM(false,rc_param_invalid,FORMAT_STR("load task file:[%1%] fial",%task_file))
    }
    _source->get_time_base();
    _time = MEDIA_FRAME_NONE_TIMESTAMP;
    _eof = false;
    JIF(g_pool.push(this))
    return rt;
}

bool mixer_engine::wait(int ms_wait)
{
    if(true == _eof)
        return true;

    _mt_wait.lock();
    if(0 > ms_wait)
    {
        std::unique_lock<std::timed_mutex> lck(_mt_wait);
        return true;
    }
    else
    {
        if(false == _mt_wait.try_lock_for(std::chrono::milliseconds(ms_wait)))
            return false;
        else
        {
            _mt_wait.unlock();
            return true;
        }
    }
}

ret_type mixer_engine::process()
{
    if(_time == MEDIA_FRAME_NONE_TIMESTAMP)
        _time = 0;

    ret_type rt = _source->process(this);
    if(rt == media_task::rc_eof)
    {
        bool eof = true;
        for(RenderIt it = _renders.begin() ; it != _renders.end() ; ++it)
        {
            if((*it)->is_eof())
            {
                (*it)->close();
                eof = false;
                break;
            }
        }
        if(eof)
        {
            _renders.clear();
            _eof = true;
            rt = media_task::rc_eof;
            if(!_mt_wait.try_lock())
                _mt_wait.unlock();
        }
        else
            rt = rc_ok;
    }
    else if(rt == rc_ok)
    {
        _time += _source->_duration;
        TRACE(dump::info,FORMAT_STR("mix engine time line:%1%ms",%(_time/10000)))
    }
    return rt;
}

void* mixer_create(const char* str_template)
{
    JCHKR(nullptr != str_template,rc_param_invalid,nullptr)

    mixer_engine* engine = new mixer_engine();
    JCHKR(nullptr != engine,rc_new_fail,nullptr);
    if(IS_FAIL(engine->load(str_template)))
    {
        delete engine;
        engine = nullptr;
    }
    return engine;
}

bool  mixer_run(void* handle,const char* str_task)
{
    JCHKR(nullptr != handle,rc_param_invalid,false)

    mixer_engine* engine = (mixer_engine*)handle;

    return IS_OK(engine->run(str_task));
}

bool  mixer_wait(void* handle,int ms_wait)
{
    JCHKR(nullptr != handle,rc_param_invalid,false)
    mixer_engine* engine = (mixer_engine*)handle;
    return engine->wait(ms_wait);
}

void  mixer_delete(void* handle)
{
    RCHK(nullptr != handle,rc_param_invalid)
    mixer_engine* engine = (mixer_engine*)handle;
    delete engine;
}
