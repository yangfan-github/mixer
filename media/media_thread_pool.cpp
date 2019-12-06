#include "unistd.h"
#include "../inc/media_thread_pool.h"
#include <boost/thread.hpp>
int64_t get_local_time();

media_task::media_task()
:_is_live(false)
,_time(MEDIA_FRAME_NONE_TIMESTAMP){}

class meida_timer : public std::enable_shared_from_this<meida_timer>
{
    protected:
        media_thread_pool* _pool;
        boost::asio::deadline_timer _timer;
    protected:
        meida_timer(media_thread_pool* pool)
        :_pool(pool)
        ,_timer(pool->get_context())
        {
        }
        ret_type post(media_task* task,int64_t duration)
        {
            JCHK(nullptr != task,rc_param_invalid)
            JCHK(0 <= duration,rc_param_invalid)
            _timer.expires_from_now(boost::posix_time::microseconds(duration));
            _timer.async_wait(std::bind(&meida_timer::process,shared_from_this(),task));
            return rc_ok;
        }
        ret_type process(media_task* task)
        {
            return _pool->process(task);
        }
    public:
        static ret_type post(media_thread_pool* pool,media_task* task,int64_t duration)
        {
            JCHK(nullptr != pool,rc_param_invalid)
            JCHK(nullptr != task,rc_param_invalid)
            JCHK(task->_is_live,rc_param_invalid)
            std::shared_ptr<meida_timer> timer(new meida_timer(pool));
            JCHK(timer,rc_new_fail)
            return timer->post(task,duration);
        }
};

media_thread_pool::media_thread_pool(size_t count_thread)
:_work_guard(boost::asio::make_work_guard(_ioc))
{
    if(0 == count_thread)
    {
        int enableCPUNum_ = sysconf(_SC_NPROCESSORS_ONLN);
        count_thread = (size_t)enableCPUNum_ * 2 - 1;
        TRACE(dump::info,FORMAT_STR("get system enable cpu count:%1% thread count:%2%",%enableCPUNum_%count_thread))
    }

    _threads.reserve(count_thread);
    for (size_t i = 0; i < count_thread; ++i)
    {
        _threads.emplace_back(boost::bind(&boost::asio::io_context::run,&_ioc));
    }
}

media_thread_pool::~media_thread_pool()
{
    //dtor
    g_dump.set_class("media_thread_pool");
    _ioc.stop();

    for (auto& t : _threads)
    {
        t.join();
    }

    TRACE(dump::info,"media thread pool join all")
}

boost::asio::io_context& media_thread_pool::get_context()
{
    return _ioc;
}

ret_type media_thread_pool::post(media_task* task)
{
    JCHK(nullptr != task,rc_param_invalid)
    if(true == task->_is_live)
    {
        int64_t now = get_local_time();
        if(task->_time > now)
        {
            int64_t duration = (task->_time - now)/10;
            return meida_timer::post(this,task,duration);
        }
    }
    boost::asio::post(_ioc,std::bind(&media_thread_pool::process,this,task));
    return rc_ok;
}

ret_type media_thread_pool::process(media_task* task)
{
    ret_type rt = task->process();
    if(IS_OK(rt))
        post(task);
    return rt;
}
