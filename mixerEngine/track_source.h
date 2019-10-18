#ifndef TRACK_SOURCE_H
#define TRACK_SOURCE_H

#include "stdafx.h"
#include "engine_tracker.h"
class engine_tracker;
class track_source : public media_filter
{
    public:
        std::shared_ptr<input_pin> _pin_input;
    protected:
        media_frame_buf _buf;
        engine_tracker* _tracker;
        std::shared_ptr<media_source> _source;
    public:
        track_source(engine_tracker* tracker);
        virtual ~track_source();
        //media_source
        ret_type open(const string& url);
        //ret_type process();

        void close();
    protected:
        //media_filter
        ret_type set_media_type(input_pin* pin,media_type* mt);
        ret_type process(media_frame* frame);
};

#endif // TRACK_SOURCE_H
