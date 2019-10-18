#ifndef MEDIA_FILTER_H
#define MEDIA_FILTER_H

#include "media_type.h"
#include "media_frame.h"


class media_filter;
class media_pin : public std::enable_shared_from_this<media_pin>
{
    protected:
        std::shared_ptr<media_type> _mt;
        media_filter* _filter;
    public:
        media_pin(media_filter* filter);
        virtual ~media_pin();
        virtual ret_type set_media_type(media_type* mt);
        media_type* get_media_type();
        virtual ret_type deliver(media_frame* frame) = 0;
};

class output_pin;
class input_pin : public media_pin
{
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
        ret_type set_media_type(media_type* mt);
        ret_type deliver(media_frame* frame);
        ret_type connect(output_pin* pin,It it);
        void disconnect();
        bool is_connect();
        bool peek(std::shared_ptr<media_frame>& frame);
        bool pop();
};

class output_pin : public media_pin
{
    protected:
        input_pin::Set _pins;
    public:
        output_pin(media_filter* filter);
        virtual ~output_pin();
        ret_type set_media_type(media_type* mt);
        ret_type deliver(media_frame* frame);
        ret_type connect(std::shared_ptr<input_pin> pin,media_type* mt = nullptr);
        void disconnect(input_pin::It& it);
        void disconnect_all();
        bool is_connect();
};

class media_filter : public std::enable_shared_from_this<media_filter>
{
        friend class input_pin;
        friend class output_pin;
    public:
        media_filter();
        virtual ~media_filter();
    protected:
        virtual ret_type set_media_type(input_pin* pin,media_type* mt);
        virtual ret_type set_media_type(output_pin* pin,media_type* mt);
        virtual ret_type process(input_pin* pin,media_frame* frame);
};

template<class Pin>class pin_deleter
{
    protected:
        std::shared_ptr<media_filter> _filter;
    public:
        pin_deleter(media_filter* filter)
        :_filter(filter){}
        void operator()(Pin* obj)
        {
            _filter.reset();
        }
};

class media_transform : public media_filter
{
    protected:
        std::shared_ptr<input_pin> _pin_input;
        std::shared_ptr<output_pin> _pin_output;
    public:
        media_transform();
        virtual ~media_transform();
        std::shared_ptr<input_pin> get_input_pin();
        std::shared_ptr<output_pin> get_output_pin();
};

class media_source : public media_filter
{
    public:
        media_source();
        virtual ~media_source();
        virtual std::shared_ptr<output_pin> get_pin(uint32_t index) = 0;
        virtual ret_type open(const string& url) = 0;
        virtual ret_type process() = 0;
        virtual void close() = 0;
};

class media_render : public media_filter
{
    public:
        media_render();
        virtual ~media_render();
        virtual std::shared_ptr<input_pin> create_pin(media_type* mt) = 0;
        virtual ret_type open(const string& url) = 0;
        virtual void close() = 0;
};

#endif // MEDIA_FILTER_H
