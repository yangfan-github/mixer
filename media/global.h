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

extern "C"
{
#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#endif

#include <libavformat/avformat.h>
#include <libavutil/samplefmt.h>
#include <libavutil/imgutils.h>
}

#include "../inc/media.h"


#endif // GLOBAL_H_INCLUDED
