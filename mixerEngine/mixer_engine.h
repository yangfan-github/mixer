#ifndef MIXER_ENGINE_H
#define MIXER_ENGINE_H

#include "engine_source.h"
#include "engine_tracker.h"
#include "stdafx.h"

class mixer_engine : public media_task
{
    protected:
        typedef list<render_ptr> RenderSet;
        typedef RenderSet::iterator RenderIt;
    protected:
        DUMP_DEF(mixer_engine)
        std::shared_ptr<engine_source> _source;
        RenderSet _renders;
        std::timed_mutex _mt_wait;
        bool _eof;
    public:
        mixer_engine();
        virtual ~mixer_engine();
        ret_type load(const char* template_file);
        ret_type run(const char* task_file);
        bool wait(int ms_wait);
        ret_type process();
};

#endif // MIXER_ENGINE_H
