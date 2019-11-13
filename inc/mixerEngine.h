#ifndef MIXERENGINE_H_INCLUDED
#define MIXERENGINE_H_INCLUDED

extern "C"
{
    void* mixer_create(const char* str_template);
    bool  mixer_run(void* handle,const char* str_task);
    bool  mixer_wait(void* handle,int ms_wait = -1);
    void  mixer_delete(void* handle);
};
#endif // I_MIXERENGINE_H_INCLUDED
