#include "../inc/media_frame.h"
#include "global.h"
media_frame::info::info()
{
    reset();
}

void media_frame::info::reset()
{
    pts = MEDIA_FRAME_NONE_TIMESTAMP;
    dts = MEDIA_FRAME_NONE_TIMESTAMP;
    flag = 0;
    duration = 0;
    stride = 0;
}

media_frame::media_frame()
:_buf(nullptr)
,_len(0)
{

}

media_frame::~media_frame()
{
    //dtor
    alloc(0);
}

ret_type media_frame::alloc(size_t len)
{
    if(0 < len)
    {
        JCHK(_buf = realloc(_buf,len),rc_new_fail)
        memset(_buf,0,len);
    }
    else
    {
        free(_buf);
        _buf = nullptr;
    }
    _len = len;
    return rc_ok;
}

void* media_frame::get_buf()
{
    return _buf;
}

size_t media_frame::get_len()
{
    return _len;
}

frame_ptr media_frame::create()
{
    return frame_ptr(new media_frame(),[](media_frame* frame){delete frame;});
}

ret_type media_frame::copy(frame_ptr dest,frame_ptr sour)
{
    JCHK(dest,rc_param_invalid)
    JCHK(sour,rc_param_invalid)

    dest->_info = sour->_info;

    ret_type rt;
    int len = sour->get_len();
    JIF(dest->alloc(len))
    if(0 <len)
        memcpy(dest->get_buf(),sour->get_buf(),len);
    return rt;
}

media_frame_buf::media_frame_buf()
:_eof(false)
{

}

media_frame_buf::~media_frame_buf()
{

}

int64_t media_frame_buf::back()
{
    unique_lock<std::mutex> lck(_mt);
    return _frames.empty() ? MEDIA_FRAME_NONE_TIMESTAMP : _frames.back()->_info.dts;
}

int64_t media_frame_buf::front()
{
    unique_lock<std::mutex> lck(_mt);
    return _frames.empty() ? MEDIA_FRAME_NONE_TIMESTAMP : _frames.front()->_info.dts;
}

void media_frame_buf::reset()
{
    _frames.clear();
    _eof = false;
}

bool media_frame_buf::empty()
{
    unique_lock<std::mutex> lck(_mt);
    return _frames.empty();
}

ret_type media_frame_buf::push(const frame_ptr& frame)
{
    unique_lock<std::mutex> lck(_mt);
    if(frame)
    {
        _frames.push_back(frame);
        if(true == _eof)
            _eof = false;
    }
    else
        _eof = true;
    return rc_ok;
}

bool media_frame_buf::peek(frame_ptr& frame)
{
    unique_lock<std::mutex> lck(_mt);
    FrameIt it = _frames.begin();
    if(it != _frames.end())
    {
        frame = *it;
        return true;
    }
    else
    {
        frame.reset();
        return false == _eof;
    }
}

bool media_frame_buf::pop()
{
    unique_lock<std::mutex> lck(_mt);
    FrameIt it = _frames.begin();
    if(it != _frames.end())
    {
        _frames.erase(it);
        return true;
    }
    else
        return false;
}

bool media_frame_buf::is_eof()
{
    return _eof;
}
