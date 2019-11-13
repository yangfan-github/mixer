#ifndef ENGINE_THREAD_POOL_H
#define ENGINE_THREAD_POOL_H
#include <map>
#include <boost/asio.hpp>

#include "../inc/media.h"


struct engine_task
{

ERROR_CODE_BEG(200)
    rc_again = rc_base,
    rc_eof,
ERROR_CODE_END

    int64_t _time;
    virtual ret_type process() = 0;
    engine_task();
};

class engine_thread_pool
{
        typedef multimap<int64_t,engine_task*,std::less<int64_t>> TaskSet;
        typedef TaskSet::iterator TaskIt;
        typedef pair<TaskSet::key_type,TaskSet::mapped_type> TaskPair;
    protected:
        boost::asio::thread_pool _pool;
        TaskSet _tasks;
        bool _eof;
        std::mutex _mt_tasks;
        std::mutex _mt_empty;
    public:
        engine_thread_pool(size_t tread_count);
        virtual ~engine_thread_pool();
        ret_type push(engine_task* task);
    protected:
        engine_task* pop();
    protected:
        void process();
};

extern engine_thread_pool g_pool;

#endif // ENGINE_THREAD_POOL_H
