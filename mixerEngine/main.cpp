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
#include "stdafx.h"
#include "mixer_engine.h"

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

int mixer_run(void* handle,const char* str_task)
{
    JCHKR(nullptr != handle,rc_param_invalid,false)

    mixer_engine* engine = (mixer_engine*)handle;

    return IS_OK(engine->run(str_task)) ? 1 : 0;
}

int  mixer_wait(void* handle,int ms_wait)
{
    JCHKR(nullptr != handle,rc_param_invalid,false)
    mixer_engine* engine = (mixer_engine*)handle;
    return engine->wait(ms_wait) ? 1 : 0;
}

void  mixer_delete(void* handle)
{
    RCHK(nullptr != handle,rc_param_invalid)
    mixer_engine* engine = (mixer_engine*)handle;
    delete engine;
}

media_thread_pool g_pool;
