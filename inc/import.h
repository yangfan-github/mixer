#ifndef IMPORT_H_INCLUDED
#define IMPORT_H_INCLUDED

extern "C"
{
    void* import_start(const char* sour,const char* dest);
    bool import_stop(void* handle);
};

#endif // IMPORT_H_INCLUDED
