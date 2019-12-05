#ifndef MIXERENGINE_H_INCLUDED
#define MIXERENGINE_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

void* mixer_create(const char* str_template);          //null : fail
int  mixer_run(void* handle,const char* str_task);    //0:fail  1:ok
int  mixer_wait(void* handle,int ms_wait);         //0:time out 1:ok  ms_wait = -1 wait no timeout
void  mixer_delete(void* handle);

#ifdef __cplusplus
}
#endif

#endif // I_MIXERENGINE_H_INCLUDED
