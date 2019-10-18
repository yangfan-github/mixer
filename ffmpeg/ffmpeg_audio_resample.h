#ifndef FFMPEG_AUDIO_RESAMPLE_H
#define FFMPEG_AUDIO_RESAMPLE_H
#include "global.h"

class ffmpeg_audio_resample : public media_transform
{
    protected:
        std::shared_ptr<media_frame> _frame;
        SwrContext* _ctxSwr;
        std::shared_ptr<media_type> _mt_resample;
        media_frame::info _info_sample;
    public:
        ffmpeg_audio_resample();
        virtual ~ffmpeg_audio_resample();
        PLUGIN_DECLARE
    protected:
        //media_filter
        ret_type set_media_type(input_pin* pin,media_type* mt);
        ret_type set_media_type(output_pin* pin,media_type* mt);
        ret_type process(input_pin* pin,media_frame* frame);
    private:
        ret_type open(media_type* mt_in,media_type* mt_out);
        ret_type audio_sample(media_frame* frame);
        void close();
};

#endif // FFMPEG_AUDIO_RESAMPLE_H
