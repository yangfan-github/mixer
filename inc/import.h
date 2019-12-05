#ifndef IMPORT_H_INCLUDED
#define IMPORT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

void* import_start(const char* sour,const char* dest);
int import_stop(void* handle);   //return 1:ok 0:fail

#ifdef __cplusplus
}
#endif

#endif // IMPORT_H_INCLUDED
