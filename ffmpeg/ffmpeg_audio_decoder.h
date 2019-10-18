#ifndef FFMPEG_AUDIO_DECODER_H
#define FFMPEG_AUDIO_DECODER_H
#include "global.h"

class ffmpeg_audio_decoder : public media_transform
{
    protected:
        AVCodecContext* _ctxCodec;
        AVFrame* _avframe;
        AVPacket _pkt;
    public:
        ffmpeg_audio_decoder();
        virtual ~ffmpeg_audio_decoder();
        PLUGIN_DECLARE
    protected:
        //media_filter
        ret_type set_media_type(input_pin* pin,media_type* mt);
        ret_type set_media_type(output_pin* pin,media_type* mt);
        ret_type process(input_pin* pin,media_frame* frame);
    private:
        ret_type open(media_type* mt);
        ret_type audio_decode(media_frame* frame);
        void close();
};

#endif // FFMPEG_AUDIO_DECODER_H
