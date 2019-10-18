// The functions contained in this file are pretty dummy
// and are included only as a placeholder. Nevertheless,
// they *will* get included in the shared library if you
// don't remove them :)
//
// Obviously, you 'll have to write yourself the super-duper
// functions to include in the resulting library...
// Also, it's not necessary to write every function in this file.
// Feel free to add more files in this project. They will be
// included in the resulting library.
#include "global.h"
#include "../inc/media.h"

extern "C"
{
    void init(dump* dmp,const char* name)
    {
        g_dump.init(dmp,name);
    }
}

plugin_filter::plugin_filter(const char* type)
:_type(type)
,_filter(nullptr)
,_handle(nullptr)
{

}

plugin_filter::~plugin_filter()
{

}

const char* plugin_filter::get_type()
{
    return _type;
}

void plugin_filter::set_filter(media_filter* filter)
{
    if(_filter == filter)
        return;
    if(nullptr != _filter)
        delete _filter;
    _filter = filter;
}

void plugin_filter::set_handle(void* handle)
{
    if(handle == _handle)
        return;
    if(nullptr != _handle)
    {
        set_filter(nullptr);
        dlclose(_handle);
    }
    _handle = handle;
}

void plugin_filter::operator()(media_filter* filter)
{
    set_filter(nullptr);
    set_handle(nullptr);
}

ret_type create_filter(plugin_filter& filter,const string& dir,const char* info,media_type* mt_input,media_type* mt_output)
{
    DIR *dp;
    JCHKM(nullptr != (dp = opendir(dir.c_str())),rc_param_invalid,FORMAT_STR("open dir:[%1%]",%dir))

    uint32_t priority = 0;
    CLS_CREATE_FUNC cls_create = nullptr;
    struct dirent *entry;
    while(nullptr != (entry = readdir(dp)))
    {
        char* dot = strrchr(entry->d_name,'.');
        if(nullptr != dot && 0 == strcmp(dot,".so"))
        {
            string path = dir;
            path.append(entry->d_name);
            void* h = dlopen(path.c_str(),RTLD_NOW);
            if(nullptr == h)
                continue;
            PLUGIN_INIT_FUNC init = (PLUGIN_INIT_FUNC)dlsym(h,PLUGIN_INIT_FUNC_NAME);
            PLUGIN_ENUM_FILTER_FUNC enum_filter = (PLUGIN_ENUM_FILTER_FUNC)dlsym(h,PLUGIN_ENUM_FILTER_FUNC_NAME);
            if(nullptr == init || nullptr == enum_filter)
            {
                dlclose(h);
                continue;
            }
            init(&g_dump,entry->d_name);
            uint32_t index = 0;
            CLS_CREATE_FUNC create_func = nullptr;
            while(enum_filter(index,filter.get_type(),info,mt_input,mt_output,create_func,priority)){++index;}
            if(create_func == cls_create)
                dlclose(h);
            else
            {
                cls_create = create_func;
                filter.set_handle(h);
            }
        }
    }
    closedir(dp);
    if(nullptr == cls_create)
        return rc_fail;

    filter.set_filter(cls_create());
    return rc_ok;
}

ret_type connect(output_pin* pin_out,input_pin* pin_in,media_type* mt_out,media_type* mt_in,const string& dir)
{
    JCHK(nullptr != pin_out,rc_param_invalid)
    JCHK(nullptr != pin_in,rc_param_invalid)
    ret_type rt;
    if(nullptr != mt_out)
    {
        JIF(pin_out->set_media_type(mt_out))
    }
    mt_out = pin_out->get_media_type();
    JCHK(nullptr != mt_out,rc_param_invalid)
    if(nullptr == mt_in)
    {
        if(nullptr == (mt_in = pin_in->get_media_type()))
            mt_in = mt_out;
    }
    JIF(media_type::copy(mt_in,mt_out,true))
    if(true == media_type::compare(mt_out,mt_in))
    {
        JIF(pin_out->connect(std::shared_ptr<input_pin>(pin_in)))
    }
    else
    {
        std::shared_ptr<media_transform> filter;
        filter = create_filter<media_transform>(nullptr,mt_out,mt_in,dir);
        if(filter)
        {
            JIF(pin_out->connect(filter->get_input_pin()))
            JIF(filter->get_output_pin()->connect(std::shared_ptr<input_pin>(pin_in)))
        }
        else
        {
            filter = create_filter<media_transform>(nullptr,mt_out,nullptr,dir);
            if(filter)
            {
                JIF(pin_out->connect(filter->get_input_pin()))
                return connect(filter->get_output_pin().get(),pin_in,nullptr,mt_in,dir);
            }
            else
            {
                filter = create_filter<media_transform>(nullptr,nullptr,mt_in,dir);
                if(filter)
                {
                    JIF(filter->get_output_pin()->connect(std::shared_ptr<input_pin>(pin_in),mt_in))
                    return connect(pin_out,filter->get_input_pin().get(),mt_out,nullptr,dir);
                }
                else
                    return rc_fail;
            }
        }
    }
    return rt;
}

bool str_cmp(const char* str_1,const char* str_2)
{
    if(str_1 == str_2)
        return true;
    if(nullptr == str_1 || nullptr == str_2)
        return false;
    return 0 == strcmp(str_1,str_2);
}
