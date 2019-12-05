#ifndef IMPORT_H_INCLUDED
#define IMPORT_H_INCLUDED

void* import_start(const char* sour,const char* dest);
int import_stop(void* handle);  //1:ok 0:fail

#endif // IMPORT_H_INCLUDED
