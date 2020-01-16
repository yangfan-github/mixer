#ifndef MEDIA_THREAD_POOL_H
#define MEDIA_THREAD_POOL_H
#include <map>

#include <thread>
#include <boost/asio.hpp>

#include "media_frame.h"


struct media_task
{
ERROR_CODE_BEG(200)
    rc_again = rc_base,
    rc_eof,
ERROR_CODE_END
    bool _is_live;
    int64_t _time;
    virtual ret_type process() = 0;
    media_task();
    bool run();
    bool stop();
    bool is_run();
protected:
    std::atomic<int> _run;
};

class media_thread_pool
{
        typedef boost::asio::io_context::executor_type ExecutorType;
    protected:
        DUMP_DEF(media_thread_pool)
        boost::asio::io_context _ioc;
        boost::asio::executor_work_guard<ExecutorType> _work_guard;
        vector<std::thread> _threads;
    public:
        media_thread_pool(size_t count_thread = 0);
        virtual ~media_thread_pool();
        boost::asio::io_context& get_context();
        ret_type post(media_task* task);
        ret_type process(media_task* task);
};

#endif // MEDIA_THREAD_POOL_H
