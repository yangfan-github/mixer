#ifndef MEDIA_H_INCLUDED
#define MEDIA_H_INCLUDED

#include "media_filter.h"

bool str_cmp(const char* str_1,const char* str_2);

typedef uint32_t OBJ_VERSION;
#define MAKE_VERSION(a,b,c,d) \
( ((OBJ_VERSION)d) | ( ((OBJ_VERSION)c) << 8 ) | ( ((OBJ_VERSION)b) << 16 ) | ( ((OBJ_VERSION)a) << 24 ) )

typedef media_filter* (*CLS_CREATE_FUNC)();
typedef uint32_t (*CLS_PRIORITY_FUNC)(const char* info,media_type* mt_input,media_type* mt_output);
struct class_info
{
    const char* type;
    const char* name;
    uint32_t version;
    CLS_CREATE_FUNC func_create;
    CLS_PRIORITY_FUNC fucn_priority;
};

#ifdef DOM_EXPORTS
#define PLUNIN_API extern "C" __attribute__ ((visibility("default")))
#else
#define PLUNIN_API extern "C" __attribute__ ((visibility("default")))
#endif


typedef void (*PLUGIN_INIT_FUNC)(dump* dmp,const char* name);
typedef bool (*PLUGIN_ENUM_FILTER_FUNC)(uint32_t& index,const char* type,const char* info,media_type* mt_input,media_type* mt_output,CLS_CREATE_FUNC& func,uint32_t& priority);
const char PLUGIN_INIT_FUNC_NAME[] = "init";
const char PLUGIN_ENUM_FILTER_FUNC_NAME[] = "enum_filter";
PLUNIN_API void init(dump* dmp,const char* name = nullptr);
PLUNIN_API bool enum_filter(uint32_t& index,const char* type,const char* info,media_type* mt_input,media_type* mt_output,CLS_CREATE_FUNC& func,uint32_t& priority);

#define CLS_INFO_DEFINE(base,class,version) \
    class_info class::cls = {typeid(base).name(),typeid(class).name(),version,class::cls_create,class::cls_priority}; \
    media_filter* class::cls_create() \
    { \
        return  new class(); \
    } \

#define PLUGIN_DECLARE \
    static class_info cls; \
    static media_filter* cls_create(); \
    static uint32_t cls_priority(const char* info,media_type* mt_input,media_type* mt_output); \

#define PLUNIN_EXPORT_BEG \
extern "C" \
{ \
    vector<class_info*> g_cls; \
    void init(dump* dmp,const char* name) \
    { \
        g_dump.init(dmp,name); \
        if(true == g_cls.empty()) \
        { \

#define PLUNIN_EXPORT_CLS(class) \
        g_cls.push_back(&class::cls); \

#define PLUNIN_EXPORT_END \
        } \
    } \
    bool enum_filter(uint32_t& index,const char* type,const char* info,media_type* mt_input,media_type* mt_output,CLS_CREATE_FUNC& func,uint32_t& priority) \
    { \
        while(index < g_cls.size()) \
        { \
            if(false == str_cmp(g_cls[index]->type,type)) \
            { \
                ++index; \
                continue; \
            } \
            uint32_t new_priority; \
            if(priority >= (new_priority = g_cls[index]->fucn_priority(info,mt_input,mt_output))) \
            { \
                ++index; \
                continue; \
            } \
            priority = new_priority; \
            func = g_cls[index]->func_create; \
            return true; \
        } \
        index = 0; \
        priority = 0; \
        return false; \
    } \
} \

class plugin_filter
{
    protected:
        const char* _type;
        media_filter* _filter;
        void* _handle;
    public:
        plugin_filter(const char* type);
        virtual ~plugin_filter();
        const char* get_type();
        void set_filter(media_filter* filter);
        void set_handle(void* handle);
        template<typename T> T* get_filter()
        {
            return dynamic_cast<T*>(_filter);
        }
        void operator()(media_filter* filter);
};

ret_type create_filter(plugin_filter& filter,const string& dir ,const char* info,media_type* mt_input,media_type* mt_output);

template<typename T>
std::shared_ptr<T> create_filter(const char* info = nullptr,media_type* mt_input = nullptr,media_type* mt_output = nullptr,const string& dir="./")
{
    plugin_filter pf(typeid(T).name());
    create_filter(pf,dir,info,mt_input,mt_output);
    return std::shared_ptr<T>(pf.get_filter<T>(),pf);
}

ret_type connect(output_pin* pin_out,input_pin* pin_in,media_type* mt_out = nullptr,media_type* mt_in = nullptr,const string& dir = "./");

#endif // MEDIA_H_INCLUDED
