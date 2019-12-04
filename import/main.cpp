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

#include "stream_import.h"

void* import_start(const char* sour,const char* dest)
{
    JCHKR(nullptr != sour,rc_param_invalid,nullptr)
    JCHKR(nullptr != dest,rc_param_invalid,nullptr)
    return new stream_import(sour,dest);
}

bool import_stop(void* handle)
{
    JCHKR(nullptr != handle,rc_param_invalid,false)
    stream_import* si = (stream_import*)handle;
    delete si;
    return true;
}

media_thread_pool g_pool;
