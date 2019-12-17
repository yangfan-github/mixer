#include "global.h"

static const AVCodecDescriptor pcm_descriptor =
{
    (AVCodecID)MST_PCM,
    AVMEDIA_TYPE_AUDIO,
    "pcm",
    "PCM audio",
    AV_CODEC_PROP_LOSSLESS
};

static const AVCodecDescriptor ts_descriptor =
{
    (AVCodecID)MST_MPEG2TS,
    AVMEDIA_TYPE_DATA,
    "ts",
    "mpeg2 ts stream",
    0
};

static const AVCodecDescriptor hls_descriptor =
{
    (AVCodecID)MST_HLS,
    AVMEDIA_TYPE_DATA,
    "m3u8",
    "hls stream",
    0
};

media_type::media_type()
:_major(MMT_UNKNOWN)
,_desc(nullptr)
,_vmt(VMT_NONE)
,_width(0)
,_height(0)
,_ratioX(0)
,_ratioY(0)
,_fps(0.0)
,_amt(AMT_NONE)
,_channel(0)
,_channel_layout(0)
,_sample_rate(0)
,_frame_size(0)
,_is_global_header(false)
,_extra_data(nullptr)
,_extra_size(0)
{
}

media_type::~media_type()
{
    set_extra_data(nullptr,0);
}

void media_type::set_major(MediaMajorType major)
{
    if(major == _major)
        return;

    if(MMT_UNKNOWN >= major ||  MMT_NB <= major)
        _major = MMT_UNKNOWN;
    else
        _major = major;
    if(nullptr != _desc && _desc->type != (AVMediaType)_major)
        set_sub(MST_NONE);
}

MediaMajorType media_type::get_major()
{
    return _major;
}

const char* media_type::get_major_name()
{
    return av_get_media_type_string((AVMediaType)get_major());
}

void media_type::set_sub(MediaSubType sub)
{
    const AVCodecDescriptor* desc = get_descriptor(sub);
    if(desc == _desc)
        return;

    if(nullptr != desc)
    {
        if(desc->type != (AVMediaType)_major)
            set_major((MediaMajorType)desc->type);
    }
    _desc = desc;

}

MediaSubType media_type::get_sub()
{
    return nullptr == _desc ? MST_NONE : (MediaSubType)_desc->id;
}

const char* media_type::get_sub_name()
{
    return _desc == nullptr ? nullptr : _desc->name;
}

int media_type::get_props()
{
    return _desc == nullptr ? 0 : _desc->props;
}

bool media_type::is_compress()
{
    MediaMajorType major = get_major();
    if(MMT_VIDEO == major)
        return get_sub() != MST_RAWVIDEO;
    else if(MMT_AUDIO == major)
        return get_props() != AV_CODEC_PROP_LOSSLESS;
    else
        return false;
}

int64_t media_type::get_duration()
{
    MediaMajorType major = get_major();
    if(MMT_VIDEO == major)
        return get_video_duration();
    else if(MMT_AUDIO == major)
        return get_audio_duration();
    else
        return 0;
}

void media_type::set_duration(int64_t duration)
{
    MediaMajorType major = get_major();
    if(MMT_VIDEO == major)
        set_video_duration(duration);
    else if(MMT_AUDIO == major)
        set_audio_duration(duration);
}

void media_type::set_video_format(VideoMediaType vmt)
{
    if(VMT_YUVJ420P == vmt)
        vmt = VMT_YUV420P;
    else if(VMT_YUVJ422P == vmt)
        vmt = VMT_YUV422P;
    else if(VMT_YUVJ444P == vmt)
        vmt = VMT_YUV444P;
    else if(VMT_YUVJ440P == vmt)
        vmt = VMT_YUV440P;

    if(VMT_NONE < vmt && VMT_NB > vmt)
        _vmt = vmt;
    else
        _vmt = VMT_NONE;
}

VideoMediaType media_type::get_video_format()
{
    return _vmt;
}

void media_type::set_video_width(int width)
{
    _width = MIN_VIDEO_WIDTH <= width ? width : 0;
    if(0 == _ratioX)
        _ratioX = _width;
}

int media_type::get_video_width()
{
    return _width;
}

void media_type::set_video_height(int height)
{
    _height = MIN_VIDEO_HEIGHT <= height ? height : 0;
    if(0 == _ratioY)
        _ratioY = _height;
}

int media_type::get_video_height()
{
    return _height;
}

void media_type::set_video_ratioX(int ratioX)
{
    _ratioX = 0 < ratioX ? ratioX : 0;
}

int media_type::get_video_ratioX()
{
    return _ratioX;
}

void media_type::set_video_ratioY(int ratioY)
{
    _ratioY = 0 < ratioY ? ratioY : 0;
}

int media_type::get_video_ratioY()
{
    return  _ratioY;
}

void media_type::set_video_fps(double fps)
{
    _fps = 0.0 < fps ? fps : 0.0;
}

double media_type::get_video_fps()
{
    return _fps;
}

ret_type media_type::set_video_duration(int64_t duration)
{
    JCHK(0 <= duration,rc_param_invalid)
    if(0 == duration)
        set_video_fps(0.0);
    else
        set_video_fps((double)ONE_SECOND_UNIT/duration);
    return rc_ok;
}

int64_t media_type::get_video_duration()
{
    return 0.0 < _fps ? int64_t(ONE_SECOND_UNIT / _fps + 0.5) : 0;
}

void media_type::set_audio_format(AudioMediaType amt)
{
    if(AMT_NONE < amt && AMT_NB > amt)
        _amt = amt;
    else
        _amt = AMT_NONE;
}

AudioMediaType media_type::get_audio_format()
{
    return _amt;
}

void media_type::set_audio_channel(int channels)
{
    if(channels != _channel)
    {
        if(0 < channels)
        {
            if(channels != av_get_channel_layout_nb_channels(_channel_layout))
                _channel_layout = av_get_default_channel_layout(channels);
        }
        else
            _channel_layout = 0;
        _channel = channels;
    }
}

int media_type::get_audio_channel()
{
    return _channel;
}

void media_type::set_audio_channel_layout(uint64_t layout)
{
    if(layout != _channel_layout)
    {
        if(0 < layout)
        {
            int channles = av_get_channel_layout_nb_channels(layout);
            if(_channel != channles)
                _channel = channles;
        }
        else
            _channel = 0;
        _channel_layout = layout;
    }
}

uint64_t media_type::get_audio_channel_layout()
{
    return _channel_layout;
}

void media_type::set_audio_sample_rate(int sample_rate)
{
    _sample_rate = 0 < sample_rate ? sample_rate : 0;
}

int media_type::get_audio_sample_rate()
{
    return _sample_rate;
}

void media_type::set_audio_frame_size(int frame_size)
{
    _frame_size = 0 < frame_size ? frame_size : 0;
}

int media_type::get_audio_frame_size()
{
    return _frame_size;
}

ret_type media_type::set_audio_duration(int64_t duration)
{
    JCHK(0 <= duration,rc_param_invalid)
    int frame_size = av_rescale_rnd(_sample_rate, duration, ONE_SECOND_UNIT, AV_ROUND_DOWN);
    set_audio_frame_size(frame_size);
    return rc_ok;
}

int64_t media_type::get_audio_duration()
{
    if(0 >= _sample_rate)
        return 0;
    else
        return av_rescale_rnd(ONE_SECOND_UNIT, _frame_size, _sample_rate, AV_ROUND_UP);
}

void media_type::set_global_header(bool is_global_header)
{
    _is_global_header = is_global_header;
}

bool media_type::get_global_header()
{
    return _is_global_header;
}

ret_type media_type::set_extra_data(uint8_t* data,int size)
{
    if(data == _extra_data)
        _extra_size = size;
    else if(nullptr == data || 0 >= size)
    {
        free(_extra_data);
        _extra_data = nullptr;
        _extra_size = 0;
    }
    else
    {
        JCHK(_extra_data = (uint8_t*)realloc(_extra_data,size),rc_new_fail)
        memcpy(_extra_data,data,size);
        _extra_size = size;
    }
    return rc_ok;
}

uint8_t* media_type::get_extra_data()
{
    return _extra_data;
}

int media_type::get_extra_size()
{
    return _extra_size;
}

void media_type::set_codec_option(const property_tree::ptree& pt)
{
    _codec_option = pt;
}

property_tree::ptree& media_type::get_codec_option()
{
    return _codec_option;
}

media_ptr media_type::create()
{
    return media_ptr(new media_type(),[](media_type* mt){delete mt;});
}

ret_type media_type::copy(media_ptr dest,const media_ptr& sour,bool partial)
{
    JCHK(dest,rc_param_invalid)
    JCHK(sour,rc_param_invalid)

    ret_type rt = rc_ok;
    if(dest == sour)
        return rt;

    if(true == partial)
    {
        if(MMT_UNKNOWN == dest->get_major())
            dest->set_major(sour->get_major());
        else if(MMT_UNKNOWN != sour->get_major())
        {
            JCHK(dest->get_major() == sour->get_major(),rc_param_invalid)
        }
        if(MST_NONE == dest->get_sub())
            dest->set_sub(sour->get_sub());
        if(MMT_VIDEO == dest->get_major())
        {
            if(VMT_NONE == dest->get_video_format())
                dest->set_video_format(sour->get_video_format());
            if(0 == dest->get_video_width())
                dest->set_video_width(sour->get_video_width());
            if(0 == dest->get_video_height())
                dest->set_video_height(sour->get_video_height());
            if(0 == dest->get_video_ratioX())
                dest->set_video_ratioX(sour->get_video_ratioX());
            if(0 == dest->get_video_ratioY())
                dest->set_video_ratioY(sour->get_video_ratioY());
            if(0.0 == dest->get_video_fps())
                dest->set_video_fps(sour->get_video_fps());
        }
        else if(MMT_AUDIO == dest->get_major())
        {
            if(AMT_NONE == dest->get_audio_format())
                dest->set_audio_format(sour->get_audio_format());
            if(0 == dest->get_audio_channel())
                dest->set_audio_channel(sour->get_audio_channel());
            if(0 == dest->get_audio_channel_layout())
                dest->set_audio_channel_layout(sour->get_audio_channel_layout());
            if(0 == dest->get_audio_sample_rate())
                dest->set_audio_sample_rate(sour->get_audio_sample_rate());
            if(0 == dest->get_audio_frame_size())
            {
                int frame_size = sour->get_audio_frame_size();
                int sample_rate =  dest->get_audio_sample_rate();
                int sample_rate_mt = sour->get_audio_sample_rate();
                if(0 < sample_rate && 0 < sample_rate_mt && sample_rate != sample_rate_mt)
                    frame_size =  av_rescale_rnd(frame_size, sample_rate, sample_rate_mt, AV_ROUND_DOWN);
                dest->set_audio_frame_size(frame_size);
            }
        }
        if(nullptr == dest->get_extra_data())
        {
            JIF(dest->set_extra_data(sour->get_extra_data(),sour->get_extra_size()))
        }
    }
    else
    {
        dest->set_major(sour->get_major());
        dest->set_sub(sour->get_sub());
        dest->set_video_format(sour->get_video_format());
        dest->set_video_width(sour->get_video_width());
        dest->set_video_height(sour->get_video_height());
        dest->set_video_ratioX(sour->get_video_ratioX());
        dest->set_video_ratioY(sour->get_video_ratioY());
        dest->set_video_fps(sour->get_video_fps());
        dest->set_audio_format(sour->get_audio_format());
        dest->set_audio_channel(sour->get_audio_channel());
        dest->set_audio_channel_layout(sour->get_audio_channel_layout());
        dest->set_audio_sample_rate(sour->get_audio_sample_rate());
        dest->set_audio_frame_size(sour->get_audio_frame_size());
        dest->set_global_header(sour->get_global_header());
        JIF(dest->set_extra_data(sour->get_extra_data(),sour->get_extra_size()))
        dest->set_codec_option(sour->get_codec_option());
    }
    return rt;
}

bool media_type::compare(const media_ptr& mt1,const media_ptr& mt2)
{
    if(mt1 == mt2)
        return true;
    if(!mt1 || !mt2)
        return false;

    if(mt1->get_major() != mt2->get_major())
        return false;
    if(mt1->get_sub() != mt2->get_sub())
        return false;
    if(MMT_VIDEO == mt1->get_major())
    {
        if(mt1->get_video_format() != mt2->get_video_format())
            return false;
        if(mt1->get_video_width() != mt2->get_video_width())
            return false;
        if(mt1->get_video_height() != mt2->get_video_height())
            return false;
        if(mt1->get_video_ratioX() != mt2->get_video_ratioX())
            return false;
        if(mt1->get_video_ratioY() != mt2->get_video_ratioY())
            return false;
        if(mt1->get_video_fps() != mt2->get_video_fps())
            return false;
    }
    else if(MMT_AUDIO == mt1->get_major())
    {
        if(mt1->get_audio_format() != mt2->get_audio_format())
            return false;
        if(mt1->get_audio_channel() != mt2->get_audio_channel())
            return false;
        if(mt1->get_audio_channel_layout() != mt2->get_audio_channel_layout())
            return false;
        if(mt1->get_audio_sample_rate() != mt2->get_audio_sample_rate())
            return false;
        if(mt1->get_audio_frame_size() != mt2->get_audio_frame_size())
            return false;
    }

    return true;
}

ret_type media_type::load(property_tree::ptree& pt)
{
    optional<string> major = pt.get_optional<string>("major");
    if(major)
        set_major(get_major_by_name(major.value()));

    optional<string> sub = pt.get_optional<string>("sub");
    if(sub)
    {
        _desc = get_descriptor(sub.value());
        if(nullptr != _desc)
            _major = (MediaMajorType)_desc->type;
    }

    optional<string> video_format = pt.get_optional<string>("video_format");
    if(video_format)
    {
        set_video_format((VideoMediaType)av_get_pix_fmt(video_format.value().c_str()));
    }

    optional<int> video_width = pt.get_optional<int>("video_width");
    if(video_width)
    {
        set_video_width(video_width.value());
    }

    optional<int> video_height= pt.get_optional<int>("video_height");
    if(video_height)
    {
        set_video_height(video_height.value());
    }

    optional<int> video_ratiox = pt.get_optional<int>("video_ratiox");
    if(video_ratiox)
    {
        set_video_ratioX(video_ratiox.value());
    }

    optional<int> video_ratioy = pt.get_optional<int>("video_ratioy");
    if(video_ratioy)
    {
        set_video_ratioY(video_ratioy.value());
    }

    optional<double> video_fps = pt.get_optional<double>("video_fps");
    if(video_fps)
    {
        set_video_fps(video_fps.value());
    }

    optional<string> audio_format = pt.get_optional<string>("audio_format");
    if(audio_format)
    {
        set_audio_format((AudioMediaType)av_get_sample_fmt(audio_format.value().c_str()));
    }

    optional<int> audio_channel = pt.get_optional<int>("audio_channel");
    if(audio_channel)
    {
        set_audio_channel(audio_channel.value());
    }

    optional<string> audio_channel_layout = pt.get_optional<string>("audio_channel_layout");
    if(audio_channel_layout)
    {
        set_audio_channel_layout(av_get_channel_layout(audio_channel_layout.value().c_str()));
    }

    optional<int> audio_sample_rate = pt.get_optional<int>("audio_sample_rate");
    if(audio_sample_rate)
    {
        set_audio_sample_rate(audio_sample_rate.value());
    }

    optional<int> audio_frame_size = pt.get_optional<int>("audio_frame_size");
    if(audio_frame_size)
    {
        set_audio_frame_size(audio_frame_size.value());
    }

    optional<property_tree::ptree&> opt = pt.get_child_optional("codec");
    if(opt)
        _codec_option = opt.value();
    else
        _codec_option.clear();
    return rc_ok;
}

ret_type media_type::save(property_tree::ptree& pt)
{
    return rc_ok;
}

MediaMajorType media_type::get_major_by_name(const string& name)
{
    if(!name.empty())
    {
        for(int i = AVMEDIA_TYPE_UNKNOWN+1 ; i < AVMEDIA_TYPE_NB ; ++i)
        {
            if(name == av_get_media_type_string((AVMediaType)i))
                return (MediaMajorType)i;
        }
    }
    return MMT_UNKNOWN;
}

const AVCodecDescriptor* media_type::get_descriptor(MediaSubType type)
{
    if(MST_PCM == type)
        return &pcm_descriptor;
    else if(MST_MPEG2TS == type)
        return &ts_descriptor;
    else if(MST_HLS == type)
        return &hls_descriptor;
    else
        return avcodec_descriptor_get((AVCodecID)type);
}

const AVCodecDescriptor* media_type::get_descriptor(const string& name)
{
    if(name == pcm_descriptor.name)
        return &pcm_descriptor;
    else if(name == ts_descriptor.name)
        return &ts_descriptor;
    else if(name == hls_descriptor.name)
        return &hls_descriptor;
    else
        return avcodec_descriptor_get_by_name(name.c_str());
}
