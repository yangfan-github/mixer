#ifndef MIXERENGINE_H_INCLUDED
#define MIXERENGINE_H_INCLUDED

void* mixer_create(const char* str_template);          //null : fail
int  mixer_run(void* handle,const char* str_task);    //0:fail  1:ok
int  mixer_wait(void* handle,int ms_wait = -1);         //0:time out 1:ok
void  mixer_delete(void* handle);

#endif // I_MIXERENGINE_H_INCLUDED
