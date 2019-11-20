// The functions contained in this file are pretty dummy
// and are included only as a placeholder. Nevertheless,
// they *will* get included in the shared library if you
// don't remove them :)
//
// Obviously, you 'll have to write yourself the super-duper
// functions to include in the resulting library...
// Also, it's not necessary to write every function in this file.
// Feel free to add more files in this project. They will be
// included in the resulting library.
#include "global.h"
#include "../inc/ffmpeg.h"
#include  "ffmpeg_source.h"
#include  "ffmpeg_render.h"
#include  "ffmpeg_video_decoder.h"
#include  "ffmpeg_audio_decoder.h"
#include  "ffmpeg_video_encoder.h"
#include  "ffmpeg_audio_encoder.h"
#include  "ffmpeg_video_scale.h"
#include  "ffmpeg_audio_resample.h"

PLUNIN_EXPORT_BEG
av_register_all();
avcodec_register_all();
avfilter_register_all();
avformat_network_init();
PLUNIN_EXPORT_CLS(ffmpeg_source)
PLUNIN_EXPORT_CLS(ffmpeg_render)
PLUNIN_EXPORT_CLS(ffmpeg_video_decoder)
PLUNIN_EXPORT_CLS(ffmpeg_audio_decoder)
PLUNIN_EXPORT_CLS(ffmpeg_video_encoder)
PLUNIN_EXPORT_CLS(ffmpeg_audio_encoder)
PLUNIN_EXPORT_CLS(ffmpeg_video_scale)
PLUNIN_EXPORT_CLS(ffmpeg_audio_resample)
PLUNIN_EXPORT_END

int gcd(int a,int b)
{
    while(a!=b)
    {
       if(a>b)
           a-=b;
       else
           b-=a;
    }
    return a;
}
ret_type convert_packet_to_frame(frame_ptr frame,AVPacket& packet,const AVRational& base,int header_size)
{
    JCHK(frame,rc_param_invalid)

    ret_type rt;
	frame->_info.flag = (AV_PKT_FLAG_KEY|AV_PKT_FLAG_CORRUPT) & packet.flags;
    frame->_info.dts = AV_NOPTS_VALUE == packet.dts ? MEDIA_FRAME_NONE_TIMESTAMP : av_rescale_q(packet.dts,base,FRAME_TIMEBASE);
    frame->_info.pts = AV_NOPTS_VALUE == packet.pts ? MEDIA_FRAME_NONE_TIMESTAMP : av_rescale_q(packet.pts,base,FRAME_TIMEBASE);
    frame->_info.duration = 0 == packet.duration ? 0 : av_rescale_q(packet.duration,base,FRAME_TIMEBASE);
    JIF(frame->alloc(packet.size+header_size))
    memcpy(((uint8_t*)frame->get_buf())+header_size,packet.data,packet.size);
    return rt;
}

ret_type convert_frame_to_packet(AVPacket& packet,const AVRational* base,frame_ptr frame)
{
    JCHK(frame,rc_param_invalid)

	packet.flags = (AV_PKT_FLAG_KEY|AV_PKT_FLAG_CORRUPT) & frame->_info.flag;
	packet.dts = nullptr == base ? frame->_info.dts : (MEDIA_FRAME_NONE_TIMESTAMP == frame->_info.dts ? AV_NOPTS_VALUE : av_rescale_q(frame->_info.dts , FRAME_TIMEBASE, *base));
	packet.pts = nullptr == base ? frame->_info.pts : (MEDIA_FRAME_NONE_TIMESTAMP == frame->_info.pts ? AV_NOPTS_VALUE : av_rescale_q(frame->_info.pts , FRAME_TIMEBASE, *base));
	packet.duration = nullptr == base ? frame->_info.duration : av_rescale_q(frame->_info.duration , FRAME_TIMEBASE, *base);
	packet.data = (uint8_t*)frame->get_buf();
	packet.size = frame->get_len();
	return rc_ok;
}

ret_type convert_frame_to_array(media_ptr mt,frame_ptr frame,uint8_t** dst_data,int* dst_linesize)
{
    JCHK(mt,rc_param_invalid)
    JCHK(frame,rc_param_invalid)

    MediaSubType sub = mt->get_sub();
    JCHK(MST_RAWVIDEO == sub || MST_PCM == sub,rc_param_invalid)

    ret_type rt = rc_ok;
    if(MST_RAWVIDEO == sub)
    {
        int szBuf;
        VideoMediaType vmt = mt->get_video_format();
        JCHK(VMT_NONE != vmt,rc_param_invalid)
        int width = mt->get_video_width();
        JCHK(0 < width,rc_param_invalid)
        int height = mt->get_video_height();
        JCHK(0 < height,rc_param_invalid)
        int stride = FFALIGN(width,VIDEO_ALIGN*2);

        JCHK(0 <(szBuf = av_image_get_buffer_size((AVPixelFormat)vmt,stride,height,1)),rc_param_invalid)
        if(0 == frame->get_len())
        {
            JIF(frame->alloc(szBuf))
            frame->_info.stride = stride;
            frame->_info.duration = mt->get_video_duration();
        }
        else
        {
            JCHK(szBuf == (int)frame->get_len(),rc_param_invalid)
        }
        if(nullptr != dst_data && nullptr != dst_linesize)
        {
            JCHK(0 < av_image_fill_arrays(dst_data,dst_linesize,(uint8_t*)frame->get_buf(),(AVPixelFormat)vmt,stride,height,1),rc_fail);
        }
    }
    else if(MST_PCM == sub)
    {
        int szBuf;
        AudioMediaType amt = mt->get_audio_format();
        JCHK(AMT_NONE != amt,rc_param_invalid)
        int channel = mt->get_audio_channel();
        JCHK(0 < channel,rc_param_invalid)
        int frame_size = mt->get_audio_frame_size();
        JCHK(0 < frame_size,rc_param_invalid)
        int sample_rate = mt->get_audio_sample_rate();
        JCHK(0 < sample_rate,rc_param_invalid)

		JCHK(0< (szBuf = av_samples_get_buffer_size(nullptr,channel,frame_size,(AVSampleFormat)amt,AUDIO_ALIGN)),rc_fail);
        if(0 == frame->get_len())
        {
            JIF(frame->alloc(szBuf))
            frame->_info.samples = frame_size;
            frame->_info.duration = mt->get_audio_duration();
        }
        else
        {
            JCHK(szBuf == (int)frame->get_len(),rc_param_invalid)
        }
        if(nullptr != dst_data && nullptr != dst_linesize)
        {
            JCHK(0 < av_samples_fill_arrays(dst_data,dst_linesize,(uint8_t*)frame->get_buf(),channel,frame_size,(AVSampleFormat)amt,AUDIO_ALIGN),rc_fail);
        }
    }
    return rt;
}

ret_type convert_array_to_frame(media_ptr mt,const uint8_t** src_data,const int* src_linesize,frame_ptr frame)
{
    ret_type rt;
    uint8_t *dst_data[4]; int dst_linesize[4];
    JIF(convert_frame_to_array(mt,frame,dst_data,dst_linesize));

    MediaSubType sub = mt->get_sub();
    if(MST_RAWVIDEO == sub)
        av_image_copy(dst_data,dst_linesize,src_data,src_linesize,(AVPixelFormat)mt->get_video_format(),mt->get_video_width(),mt->get_video_height());
    else if(MST_PCM == sub)
        av_samples_copy(dst_data,(uint8_t*const*)src_data,0,0,frame->_info.samples,mt->get_audio_channel(),(AVSampleFormat)mt->get_audio_format());
    return rt;
}

ret_type convert_avframe_to_frame(media_ptr mt,frame_ptr dest,AVFrame* sour,AVCodecContext* ctxCodec)
{
    dest->_info.flag |= 0 == sour->key_frame ? 0 : MEDIA_FRAME_FLAG_SYNCPOINT;
    dest->_info.dts = AV_NOPTS_VALUE == sour->pkt_pts ? MEDIA_FRAME_NONE_TIMESTAMP : av_rescale_q(sour->pkt_pts , ctxCodec->time_base, FRAME_TIMEBASE);
    dest->_info.pts = AV_NOPTS_VALUE == sour->pkt_dts ? MEDIA_FRAME_NONE_TIMESTAMP : av_rescale_q(sour->pkt_dts , ctxCodec->time_base, FRAME_TIMEBASE);
    return convert_array_to_frame(mt,(const uint8_t**)sour->data,(const int*)sour->linesize,dest);
}

ret_type convert_frame_to_avframe(media_ptr mt,AVFrame* dest,frame_ptr sour,AVCodecContext* ctxCodec)
{
	dest->key_frame = 0 == (MEDIA_FRAME_FLAG_SYNCPOINT&sour->_info.flag) ? 0 : 1;
    dest->pkt_pts = MEDIA_FRAME_NONE_TIMESTAMP == sour->_info.pts ? AV_NOPTS_VALUE : av_rescale_q(sour->_info.pts , FRAME_TIMEBASE, ctxCodec->time_base);
    dest->pkt_dts = MEDIA_FRAME_NONE_TIMESTAMP == sour->_info.dts ? AV_NOPTS_VALUE : av_rescale_q(sour->_info.dts , FRAME_TIMEBASE, ctxCodec->time_base);
	dest->pts = dest->pkt_pts;
	if(AVMEDIA_TYPE_VIDEO == ctxCodec->codec_type)
	{
        dest->width = ctxCodec->width;
        dest->height = ctxCodec->height;
        dest->format = ctxCodec->pix_fmt;
        dest->sample_aspect_ratio = ctxCodec->sample_aspect_ratio;
	}
	else if(AVMEDIA_TYPE_AUDIO == ctxCodec->codec_type)
	{
        dest->format = ctxCodec->sample_fmt;
        dest->nb_samples = ctxCodec->frame_size;
        dest->channels = ctxCodec->channels;
        dest->sample_rate = ctxCodec->sample_rate;
	}
	return convert_frame_to_array(mt,sour,dest->data,dest->linesize);
}

void get_audio_sample_rate(const int* supported_samplerates,int& sample_rate)
{
    int i=0;
    int sample_rate_result = 0,delta = 0;
    do
    {
        int sample_rate_temp = supported_samplerates[i];
        if(0 == sample_rate_temp)
            break;
        int delta_temp = abs(sample_rate_temp - sample_rate);
        if(0 == delta_temp)
        {
            delta = delta_temp;
            sample_rate_result = sample_rate_temp;
            break;
        }
        else if(0 == i || delta_temp < delta)
        {
            delta = delta_temp;
            sample_rate_result = sample_rate_temp;
        }
    }while(0 < ++i);
    sample_rate = sample_rate_result;
}

void get_option(void* obj,property_tree::ptree& pt)
{
    BOOST_FOREACH(property_tree::ptree::value_type &pt_option, pt)
    {
        int rl = -1;
        optional<int> int_value = pt_option.second.get_value_optional<int>();
        if(int_value)
            rl = av_opt_set_int(obj,pt_option.first.c_str(),int_value.value(),0);
        else
        {
            optional<double> float_value = pt_option.second.get_value_optional<double>();
            if(float_value)
                rl = av_opt_set_double(obj,pt_option.first.c_str(),float_value.value(),0);
            else
            {
                optional<string> str_value = pt_option.second.get_value_optional<string>();
                if(str_value)
                {
                    string str = str_value.value();
                    rl = av_opt_set(obj,pt_option.first.c_str(),str.c_str(),0);
                }
            }
        }
        if(0 != rl)
        {
            TRACE(dump::warn,FORMAT_STR("set option:[%1%] fail",%pt_option.first))
        }
    }
}
