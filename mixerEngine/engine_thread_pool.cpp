#include "engine_thread_pool.h"

engine_task::engine_task()
:_time(MEDIA_FRAME_NONE_TIMESTAMP){}

engine_thread_pool::engine_thread_pool(size_t tread_count)
:_pool(tread_count)
,_eof(false)
{
    //ctor
    _mt_empty.lock();
    for(size_t index = 0 ; index < tread_count ; ++index)
    {
        boost::asio::post(_pool, std::bind(&engine_thread_pool::process,this));
    }
}

engine_thread_pool::~engine_thread_pool()
{
    //dtor
    if(false == _eof)
    {
        if(!_mt_empty.try_lock())
            _mt_empty.unlock();
        _eof = true;
    }
    _pool.join();
}

ret_type engine_thread_pool::push(engine_task* task)
{
    JCHK(nullptr != task,rc_param_invalid)
    unique_lock<std::mutex> lck(_mt_tasks);
    if(_tasks.empty())
        _mt_empty.unlock();
    _tasks.insert(TaskPair(task->_time,task));
    return rc_ok;
}

engine_task* engine_thread_pool::pop()
{
    engine_task* task = nullptr;
    while(true)
    {
        {
            unique_lock<std::mutex> lck(_mt_tasks);
            TaskIt it = _tasks.begin();
            if(it != _tasks.end())
            {
                 task = it->second;
                _tasks.erase(it);
                if(_tasks.empty() && !_eof)
                    _mt_empty.lock();
                break;
            }
            else if(_eof)
            {
                break;
            }
        }
        {
            unique_lock<std::mutex> lck(_mt_empty);
        }
    }
    return task;
}

void engine_thread_pool::process()
{
    engine_task* task;
    while(nullptr != (task = pop()))
    {
        if(rc_ok == task->process())
            push(task);
    }
}

engine_thread_pool g_pool(2);
