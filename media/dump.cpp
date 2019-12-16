#include "../inc/dump.h"
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
using namespace boost::posix_time;

dump g_dump;

dump::dump()
:_level(level::info)
,_dump(nullptr)
{
    //ctor
}

dump::~dump()
{
    //dtor
}

void dump::init(dump* dmp,const char* name)
{
    if(dmp != this)
    {
        _dump = dmp;
        if(nullptr != name)
            _class = name;
        else
            _class = "main";
    }
}

ret_type dump::check(const char* file,unsigned int line,const char* code,ret_type rt,const string& describe)
{
    dump* dp = nullptr == _dump ? this : _dump;
    dp->output(dp,_class,this,level::error,
        FORMAT_STR("{\"file\":\"%1%\",\"line\":%2%,\"code\":\"%3%\",\"return\":{\"code\":%4%,\"describe\":\"%5%\"}}",
        %file%line%code%rt%describe));
    return rt;
}

void dump::trace(level lev,const string& describe)
{
    dump* dp = nullptr == _dump ? this : _dump;
    dp->output(dp,_class,this,lev,FORMAT_STR("{\"describe\":\"%1%\"}",%describe));
}

void dump::output(dump* dp,string& cls,dump* dmp,level lev,const string& describe)
{
    if(this != dp)
    {
        cls.insert(0,"/");
        cls.insert(0,_class);
    }
    if(nullptr != _dump)
        _dump->output(this,cls,dmp,lev,describe);
    else if(lev >= _level)
    {
        string s = FORMAT_STR("{\"time\":\"%1%\",\"level\":\"%2%\",\"obj\":\"%3%[%4%]\",\"%2%\":%5%}",
            %to_iso_extended_string(second_clock::local_time())
            %level_name[lev]
            %cls
            %dmp
            %describe);
        if(lev == error)
            cerr << s << endl;
        else
            clog << s << endl;
    }
}
