#include <algorithm>
#include "mixer_engine.h"

mixer_engine::mixer_engine()
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
        property_tree::ptree pt_soucre;
        property_tree::json_parser::read_json(template_file, pt_soucre);
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
        property_tree::ptree pt_task;
        property_tree::json_parser::read_json(task_file, pt_task);
        JCHK(false == pt_task.empty(),rc_param_invalid)

        optional<property_tree::ptree&> pt_outputs = pt_task.get_child_optional("outputs");
        optional<property_tree::ptree&> pt_segments = pt_task.get_child_optional("inputs");
        JCHK(pt_outputs,rc_param_invalid)
        JCHK(pt_segments,rc_param_invalid)
        JCHK(false == pt_outputs.value().empty(),rc_param_invalid)
        JCHK(false == pt_segments.value().empty(),rc_param_invalid)

        BOOST_FOREACH(property_tree::ptree::value_type &pt_output, pt_outputs.value())
        {
            std::shared_ptr<media_render> render = create_filter<media_render>(pt_output.first.c_str());
            JCHK(render,rc_param_invalid)
            BOOST_FOREACH(property_tree::ptree::value_type &pt_stream, pt_output.second)
            {
                tracker_mixer* mixer;
                media_type* mt;
                std::shared_ptr<input_pin> pin;
                string path = pt_stream.second.get_value<string>();
                JCHK(mixer = _source->find(path),rc_param_invalid)
                JCHK(mt = mixer->find(path),rc_param_invalid)
                JCHK(pin = render->create_pin(mt),rc_param_invalid);
                JIF(connect(mixer,pin.get(),nullptr,mt));
            }
            JIF(render->open(pt_output.first))
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
    return rc_ok;
}
