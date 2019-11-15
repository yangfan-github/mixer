#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include <dirent.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/io.h>
#include <memory.h>
#include <cstdlib>
#include <vector>
#include <list>
#include <map>
#include <mutex>

#include <boost/dll.hpp>
using namespace boost::dll;

extern "C"
{
#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#endif

#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libavutil/samplefmt.h>
#include <libavutil/imgutils.h>
}

#include "../inc/media.h"

class filter_deleter
{
    protected:
        shared_library _lib;
        PLUGIN_RELEASE_FILTER_FUNC* _func_release;
    public:
        filter_deleter(const shared_library& lib,PLUGIN_RELEASE_FILTER_FUNC func_release);
        virtual ~filter_deleter();
        void operator()(media_filter* filter);
};


#endif // GLOBAL_H_INCLUDED
