#ifndef DUMP_H
#define DUMP_H

#include <stdint.h>
#include <cstdlib>
#include <string>
#include <boost/format.hpp>

using namespace std;

typedef uint32_t ref_type;
typedef uint32_t ret_type;

#define ERROR_CODE_BEG(base) \
    enum ret_code \
    { \
        rc_base = base, \

#define ERROR_CODE_END \
        rc_count \
    }; \

ERROR_CODE_BEG(0)
    rc_ok = rc_base,
    rc_fail,
    rc_param_invalid,
    rc_state_invalid,
    rc_new_fail,
ERROR_CODE_END

#define IS_FAIL(exp) rc_ok < (exp)
#define IS_OK(exp) rc_ok == (exp)

class dump
{
    public:
        enum level
        {
            debug = 0,
            info,
            warn,
            error,
            count
        } _level;
        const char* level_name[level::count] =
        {
            "debug",
            "info",
            "warn",
            "error",
        };
    protected:
        dump* _dump;
        string _class;
    public:
        dump();
        virtual ~dump();
        void init(dump* dmp,const char* name);
        ret_type check(const char* file,unsigned int line,const char* code,ret_type rt,const string& describe = "null");
        void trace(level lev,const string& describe);
    protected:
        void output(dump* dp,string& cls,dump* dmp,level lev,const string& describe);
};

extern dump g_dump;

template <class T> class dump_imp : public dump
{
public:
    dump_imp()
    {
        _class = typeid(T).name();
        _dump = &g_dump;
    }
    void set_class(const char* name)
    {
        if(nullptr != name)
            _class = name;
    }
};

#define DUMP_DEF(cls) \
    dump_imp<cls> g_dump;

#define FORMAT_STR(m,p) \
    boost::str(boost::format(m)p) \

#define TRACE(lev,desc) g_dump.trace(lev,desc);

#define JCHKR(exp,ret,rtu) \
    if(!(exp)) {g_dump.check(__FILE__,__LINE__,#exp,ret);return rtu;} \

#define JCHKMR(exp,ret,msg,rtu) \
    if(!(exp)) {g_dump.check(__FILE__,__LINE__,#exp,ret,msg);return rtu;} \

#define JCHK(exp,ret) \
    if(!(exp)) return g_dump.check(__FILE__,__LINE__,#exp,ret); \

#define JCHKM(exp,ret,msg) \
    if(!(exp)) return g_dump.check(__FILE__,__LINE__,#exp,ret,msg); \

#define RCHK(exp,ret) \
    if(!(exp)) {g_dump.check(__FILE__,__LINE__,#exp,ret);return;} \

#define RCHKM(exp,ret,msg) \
    if(!(exp)) {g_dump.check(__FILE__,__LINE__,#exp,ret,msg);return;} \

#define LCHK(exp,ret) \
    if(!(exp)) g_dump.check(__FILE__,__LINE__,#exp,ret); \

#define LCHKM(exp,ret,msg) \
    if(!(exp)) g_dump.check(__FILE__,__LINE__,#exp,ret,msg); \

#define LCHKMR(exp,ret,msg,cb) \
    if(!(exp)) g_dump.check(__FILE__,__LINE__,#exp,ret,msg,cb); \

#define JIF(exp) \
    if(IS_FAIL(rt = (exp))) return g_dump.check(__FILE__,__LINE__,#exp,rt); \

#define JIFM(exp,msg) \
    if(IS_FAIL(rt = (exp))) return g_dump.check(__FILE__,__LINE__,#exp,rt,msg); \

#define JIFR(exp,rtu) \
    if(IS_FAIL(rt = (exp))) {g_dump.check(__FILE__,__LINE__,#exp,rt);return rtu;} \

#define JIFMR(exp,msg,rtu) \
    if(IS_FAIL(rt = (exp))) {g_dump.check(__FILE__,__LINE__,#exp,rt,msg);return rtu;} \

#define LIF(exp) \
    if(IS_FAIL(rt = (exp))) g_dump.check(__FILE__,__LINE__,#exp,rt); \

#define LIFM(exp,msg) \
    if(IS_FAIL(rt = (exp))) g_dump.check(__FILE__,__LINE__,#exp,rt,msg); \

#define RIF(exp) \
    if(IS_FAIL(rt = (exp))) {g_dump.check(__FILE__,__LINE__,#exp,rt); return;} \

#define RIFM(exp,msg) \
    if(IS_FAIL(rt = (exp))) {g_dump.check(__FILE__,__LINE__,#exp,rt,msg); return;} \


#endif // DUMP_H
