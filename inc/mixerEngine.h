#ifndef MIXERENGINE_H_INCLUDED
#define MIXERENGINE_H_INCLUDED

#include "dump.h"
extern "C"
{

    struct mixer
    {
        virtual ret_type load(const char* template_file) = 0;
        virtual ret_type run(const char* task_file) = 0;
    };

    std::shared_ptr<mixer> create();
};
#endif // I_MIXERENGINE_H_INCLUDED
