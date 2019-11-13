#ifndef FFMPEG_VIDEO_SCALE_H
#define FFMPEG_VIDEO_SCALE_H
#include "global.h"

class ffmpeg_video_scale : public media_transform
{
    protected:
        SwsContext* _ctxSws;
        frame_ptr _frame;
    public:
        ffmpeg_video_scale();
        virtual ~ffmpeg_video_scale();
        PLUGIN_DECLARE
    protected:
        //media_filter
        ret_type set_media_type(input_pin* pin,media_ptr mt);
        ret_type set_media_type(output_pin* pin,media_ptr mt);
        ret_type process(input_pin* pin,frame_ptr frame);
    private:
        ret_type video_sample(frame_ptr frame);
        void close();
};

#endif // FFMPEG_VIDEO_SCALE_H
