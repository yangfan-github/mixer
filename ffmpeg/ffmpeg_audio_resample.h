#ifndef FFMPEG_AUDIO_RESAMPLE_H
#define FFMPEG_AUDIO_RESAMPLE_H
#include "global.h"

class ffmpeg_audio_resample : public media_transform
{
    protected:
        frame_ptr _frame;
        SwrContext* _ctxSwr;
        media_ptr _mt_resample;
        media_frame::info _info_sample;
    public:
        ffmpeg_audio_resample();
        virtual ~ffmpeg_audio_resample();
        PLUGIN_DECLARE
    protected:
        //media_filter
        ret_type set_media_type(input_pin* pin,media_ptr mt);
        ret_type set_media_type(output_pin* pin,media_ptr mt);
        ret_type process(input_pin* pin,frame_ptr frame);
    private:
        ret_type open(media_ptr mt_in,media_ptr mt_out);
        ret_type audio_sample(frame_ptr frame);
        void close();
};

#endif // FFMPEG_AUDIO_RESAMPLE_H
