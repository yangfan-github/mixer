#ifndef MEDIA_TYPE_H
#define MEDIA_TYPE_H

#include <boost/property_tree/ptree.hpp>
using namespace boost;

#include "dump.h"
#include "media_id.h"

class media_type;
typedef std::shared_ptr<media_type> media_ptr;

class AVCodecDescriptor;
class media_type : public std::enable_shared_from_this<media_type>
{
    public:
        static const int MIN_VIDEO_WIDTH = 16;
        static const int MIN_VIDEO_HEIGHT = 16;
    protected:
        MediaMajorType _major;
        const AVCodecDescriptor* _desc;
        VideoMediaType _vmt;
        int _width;
        int _height;
        int _ratioX;
        int _ratioY;
        double _fps;
        AudioMediaType _amt;
        int _channel;
        uint64_t _channel_layout;
        int _sample_rate;
        int _frame_size;
        bool _is_global_header;
        uint8_t* _extra_data;
        int _extra_size;
        property_tree::ptree _codec_option;
        media_type();
        virtual ~media_type();
    public:
        void set_major(MediaMajorType major);
        MediaMajorType get_major();
        const char* get_major_name();
        void set_sub(MediaSubType sub);
        MediaSubType get_sub();
        const char* get_sub_name();
        int get_props();
        bool is_compress();
        int64_t get_duration();
        void set_duration(int64_t duration);
        void set_video_format(VideoMediaType vmt);
        VideoMediaType get_video_format();
        void set_video_width(int width);
        int get_video_width();
        void set_video_height(int height);
        int get_video_height();
        void set_video_ratioX(int ratioX);
        int get_video_ratioX();
        void set_video_ratioY(int ratioY);
        int get_video_ratioY();
        void set_video_fps(double fps);
        double get_video_fps();
        ret_type set_video_duration(int64_t duration);
        int64_t get_video_duration();
        void set_audio_format(AudioMediaType amt);
        AudioMediaType get_audio_format();
        void set_audio_channel(int channels);
        int get_audio_channel();
        void set_audio_channel_layout(uint64_t layout);
        uint64_t get_audio_channel_layout();
        void set_audio_sample_rate(int sample_rate);
        int get_audio_sample_rate();
        void set_audio_frame_size(int frame_size);
        int get_audio_frame_size();
        ret_type set_audio_duration(int64_t duration);
        int64_t get_audio_duration();
        void set_global_header(bool is_global_header);
        bool get_global_header();
        ret_type set_extra_data(uint8_t* data,int size);
        uint8_t* get_extra_data();
        int get_extra_size();
        void set_codec_option(const property_tree::ptree& pt);
        property_tree::ptree& get_codec_option();
        ret_type load(property_tree::ptree& pt);
        ret_type save(property_tree::ptree& pt);
        static media_ptr create();
        static ret_type copy(media_ptr dest,const media_ptr& sour,bool partial = false);
        static bool compare(const media_ptr& mt1,const media_ptr& mt2);
    protected:
        static MediaMajorType get_major_by_name(const string& name);
        static const AVCodecDescriptor* get_descriptor(MediaSubType type);
        static const AVCodecDescriptor* get_descriptor(const string& name);
};

#endif // MEDIA_TYPE_H
