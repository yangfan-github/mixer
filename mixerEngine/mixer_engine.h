#ifndef MIXER_ENGINE_H
#define MIXER_ENGINE_H

#include "engine_source.h"
#include "engine_render.h"
#include "engine_tracker.h"
#include "stdafx.h"

class mixer_engine : public mixer
{
    protected:
        std::shared_ptr<engine_source> _source;
    public:
        mixer_engine();
        virtual ~mixer_engine();
        ret_type load(const char* template_file);
        ret_type run(const char* task_file);
};

#endif // MIXER_ENGINE_H
