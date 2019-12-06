#ifndef MEDIA_FILTER_H
#define MEDIA_FILTER_H

#include "media_type.h"
#include "media_frame.h"


class media_filter;
typedef std::shared_ptr<media_filter> filter_ptr;

class media_pin
{
    protected:
        DUMP_DEF(media_pin)
        media_ptr _mt;
        media_filter* _filter;
    public:
        media_pin(media_filter* filter);
        virtual ~media_pin();
        virtual ret_type set_media_type(media_ptr mt);
        media_ptr get_media_type();
        filter_ptr get_filter();
        virtual ret_type deliver(frame_ptr frame) = 0;
};

class output_pin;
class input_pin : public media_pin
{
        friend class output_pin;
    public:
        typedef list<std::shared_ptr<input_pin>> Set;
        typedef Set::iterator It;
    protected:
        output_pin* _pin;
        It _it;
        media_frame_buf _buf;
    public:
        input_pin(media_filter* filter);
        virtual ~input_pin();
        ret_type set_media_type(media_ptr mt);
        ret_type deliver(frame_ptr frame);
        void disconnect();
        bool is_connect();
        bool peek(frame_ptr& frame);
        bool pop();
        bool eof();
    protected:
        ret_type connect(output_pin* pin,It it);
};

typedef std::shared_ptr<input_pin> input_pin_ptr;

class output_pin : public media_pin
{
        friend class input_pin;
    protected:
        input_pin::Set _pins;
        bool _is_new_segment;
    public:
        output_pin(media_filter* filter);
        virtual ~output_pin();
        ret_type set_media_type(media_ptr mt);
        void new_segment();
        ret_type deliver(frame_ptr frame);
        ret_type connect(std::shared_ptr<input_pin> pin,media_ptr mt = media_ptr());
        void disconnect_all();
        bool is_connect();
    protected:
        void disconnect(input_pin::It it);
};

typedef std::shared_ptr<output_pin> output_pin_ptr;

class media_filter : public std::enable_shared_from_this<media_filter>
{
        friend class input_pin;
        friend class output_pin;
    protected:
        DUMP_DEF(media_filter)
    public:
        media_filter();
        virtual ~media_filter();
    protected:
        virtual ret_type set_media_type(input_pin* pin,media_ptr mt);
        virtual ret_type set_media_type(output_pin* pin,media_ptr mt);
        virtual ret_type process(input_pin* pin,frame_ptr frame);
        virtual ret_type deliver(output_pin* pin,frame_ptr frame);

};

template<class Pin>class pin_deleter
{
    protected:
        filter_ptr _filter;
    public:
        pin_deleter(filter_ptr filter)
        :_filter(filter){}
        void operator()(Pin* obj)
        {
            _filter.reset();
        }
};

class media_transform : public media_filter
{
    protected:
        input_pin_ptr _pin_input;
        output_pin_ptr _pin_output;
    public:
        media_transform();
        virtual ~media_transform();
        input_pin_ptr get_input_pin();
        output_pin_ptr get_output_pin();
};

typedef std::shared_ptr<media_transform> transform_ptr;

class media_source : public media_filter
{
    public:
        media_source();
        virtual ~media_source();
        virtual output_pin_ptr get_pin(uint32_t index) = 0;
        virtual ret_type open(const string& url) = 0;
        virtual ret_type process() = 0;
        virtual void exit() = 0;
        virtual void close() = 0;
        virtual void set_base(int64_t time) = 0;
        virtual bool is_open() = 0;
        virtual bool is_eof() = 0;
};

typedef std::shared_ptr<media_source> source_ptr;

class media_render : public media_filter
{
    public:
        media_render();
        virtual ~media_render();
        virtual input_pin_ptr create_pin(media_ptr mt) = 0;
        virtual ret_type open(const string& url) = 0;
        virtual void close() = 0;
        virtual bool is_open() = 0;
        virtual bool is_eof() = 0;
        virtual int64_t get_time() = 0;
};

typedef std::shared_ptr<media_render> render_ptr;

#endif // MEDIA_FILTER_H
