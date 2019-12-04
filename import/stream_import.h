#ifndef STREAM_IMPORT_H
#define STREAM_IMPORT_H

#include "../inc/media.h"

class stream_import : public media_task
{
    protected:
        string _sour;
        string _dest;
        source_ptr _source;
        render_ptr _render;
        std::timed_mutex _mt_wait;
        std::atomic<bool> _exit;
    public:
        stream_import(const char* sour,const char* dest);
        virtual ~stream_import();
    protected:
        ret_type run();
        ret_type process();
};

extern media_thread_pool g_pool;

#endif // STREAM_IMPORT_H
