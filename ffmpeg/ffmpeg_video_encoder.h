#ifndef FFMPEG_VIDEO_ENCODER_H
#define FFMPEG_VIDEO_ENCODER_H
#include "global.h"

class ffmpeg_video_encoder : public media_transform
{
    protected:
        AVCodecContext* _ctxCodec;
    public:
        ffmpeg_video_encoder();
        virtual ~ffmpeg_video_encoder();
        PLUGIN_DECLARE
    protected:
        //media_filter
        ret_type set_media_type(input_pin* pin,media_type* mt);
        ret_type set_media_type(output_pin* pin,media_type* mt);
    private:
        ret_type open(media_type* mt);
        ret_type process(input_pin* pin,media_frame* frame);
        void close();
};

#endif // FFMPEG_VIDEO_ENCODER_H
