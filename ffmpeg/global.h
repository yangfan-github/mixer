#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

#include <cstdlib>
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <mutex>
#include <typeinfo>
#include <boost/any.hpp>
#include <boost/foreach.hpp>
#include "../inc/media.h"
#include "../inc/media_filter.h"

extern "C"
{
#ifdef __cplusplus
#define __STDC_CONSTANT_MACROS
#endif

#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/hevc.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/internal.h>
#include <libavcodec/mpeg4audio.h>
#include <libavcodec/get_bits.h>
#include <libavcodec/put_bits.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}

const AVRational FRAME_TIMEBASE = {1,ONE_SECOND_UNIT};
const AVRational VIDEO_TIMEBASE = {1,10000};

const int VIDEO_ALIGN = 16;
const int AUDIO_ALIGN = 4;

int gcd(int a,int b);
ret_type convert_packet_to_frame(frame_ptr frame,AVPacket& packet,const AVRational& base,int header_size = 0);
ret_type convert_frame_to_packet(AVPacket& packet,const AVRational* base,frame_ptr frame);
ret_type convert_frame_to_array(media_ptr mt,frame_ptr frame,uint8_t** dst_data,int* dst_linesize);
ret_type convert_array_to_frame(media_ptr mt,const uint8_t** src_data,const int* src_linesize,frame_ptr frame);
ret_type convert_avframe_to_frame(media_ptr mt,frame_ptr dest,AVFrame* sour,AVCodecContext* ctxCodec);
ret_type convert_frame_to_avframe(media_ptr mt,AVFrame* dest,frame_ptr sour,AVCodecContext* ctxCodec);
void get_audio_sample_rate(const int* supported_samplerates,int& sample_rate);
void get_option(AVCodecContext* ctx,property_tree::ptree& pt);

#endif // GLOBAL_H_INCLUDED
