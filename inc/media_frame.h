#ifndef MEDIA_FRAME_H
#define MEDIA_FRAME_H
#include "dump.h"
#include <mutex>
#include <list>

class media_frame;
typedef std::shared_ptr<media_frame> frame_ptr;

const int64_t ONE_SECOND_UNIT = 10000000;
const int64_t MEDIA_FRAME_NONE_TIMESTAMP  = 0x8000000000000001;                         //None Timestamp

const uint8_t MEDIA_FRAME_FLAG_SYNCPOINT      = 1;
const uint8_t MEDIA_FRAME_FLAG_CORRUPT         = MEDIA_FRAME_FLAG_SYNCPOINT<<1;     //data corrupt
const uint8_t MEDIA_FRAME_FLAG_NEWSEGMENT     = MEDIA_FRAME_FLAG_CORRUPT<<1;       //New segment
const uint8_t MEDIA_FRAME_FLAG_MEDIA_CHANGE  = MEDIA_FRAME_FLAG_NEWSEGMENT<<1;   //media change

class media_frame : public std::enable_shared_from_this<media_frame>
{
    protected:
        void* _buf;
        size_t _len;
    public:
        struct info
        {
            int64_t pts;
            int64_t dts;
            uint8_t flag;
            int64_t duration;
            union
            {
                int32_t index;
                int32_t stride;
                int32_t samples;
            };
            info();
            void reset();
        } _info;
        media_frame();
        virtual ~media_frame();
    public:
        ret_type alloc(size_t len);
        void* get_buf();
        size_t get_len();
        static frame_ptr create();
        static ret_type copy(frame_ptr dest,frame_ptr sour);
    protected:
    private:
};

class media_frame_buf
{
    protected:
        typedef std::list<frame_ptr> FrameSet;
        typedef FrameSet::iterator FrameIt;
    protected:
        FrameSet _frames;
        std::mutex _mt;
        bool _eof;
    public:
        media_frame_buf();
        virtual ~media_frame_buf();
        int64_t back();
        int64_t front();
        void reset();
        bool empty();
        ret_type push(const frame_ptr& frame);
        bool peek(frame_ptr& frame);
        bool pop();
        bool is_eof();
};
#endif // MEDIA_FRAME_H
