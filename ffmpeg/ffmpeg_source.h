#ifndef FFMPEG_SOURCE_H
#define FFMPEG_SOURCE_H
#include "global.h"

class ffmpeg_source : public media_source
{
    protected:
        class stream : public output_pin
        {
            typedef struct ADTSContext {
                int write_adts;
                unsigned int objecttype;
                int sample_rate_index;
                int channel_conf;
                size_t pce_size;
                int apetag;
                int id3v2tag;
                uint8_t pce_data[MAX_PCE_SIZE];
            } ADTSContext;
            protected:
                ffmpeg_source* _source;
                AVStream* _stream;
                int64_t _start;
                int64_t _length;
                int64_t _duration;
                bool _is_global_header;
                AVBitStreamFilterContext* _ctxBSF;
                ADTSContext* _adts;
                int64_t _time;
            public:
                stream(media_filter* filter);
                virtual ~stream();
                ret_type open(AVStream* stream);
                ret_type process(AVPacket& pkt);
                void close();
                ret_type adts_decode_extradata(const uint8_t *buf, size_t size);
                size_t adts_write_frame_header(uint8_t *buf,size_t size);
        };
    protected:
        AVFormatContext* _ctxFormat;
        vector<std::shared_ptr<output_pin>> _pins;
        bool _is_global_header;
        bool _eof;
    public:
        ffmpeg_source();
        virtual ~ffmpeg_source();
        PLUGIN_DECLARE
    protected:
        //media_filter
        ret_type set_media_type(output_pin* pin,media_type* mt);
        ret_type process();
        //media_source
        std::shared_ptr<output_pin> get_pin(uint32_t index);
        ret_type open(const string& url);
        void close();
    private:
};

#endif // FFMPEG_SOURCE_H
