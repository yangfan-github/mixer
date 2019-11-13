#ifndef FFMPEG_AUDIO_ENCODER_H
#define FFMPEG_AUDIO_ENCODER_H
#include "global.h"

class ffmpeg_audio_encoder : public media_transform
{
    protected:
        AVCodecContext* _ctxCodec;
    public:
        ffmpeg_audio_encoder();
        virtual ~ffmpeg_audio_encoder();
        PLUGIN_DECLARE
    protected:
        //media_filter
        ret_type set_media_type(input_pin* pin,media_ptr mt);
        ret_type set_media_type(output_pin* pin,media_ptr mt);
    private:
        ret_type open(media_type* mt);
        ret_type process(input_pin* pin,frame_ptr frame);
        void close();
};

#endif // FFMPEG_AUDIO_ENCODER_H
