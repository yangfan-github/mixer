#include "ffmpeg_audio_resample.h"

ffmpeg_audio_resample::ffmpeg_audio_resample()
:_ctxSwr(nullptr)
{
    //ctor
}

ffmpeg_audio_resample::~ffmpeg_audio_resample()
{
    //dtor
    close();
}

CLS_INFO_DEFINE(media_transform,ffmpeg_audio_resample,MAKE_VERSION(0,0,0,0))

uint32_t ffmpeg_audio_resample::cls_priority(const char* info,media_type* mt_input,media_type* mt_output)
{
    if(nullptr == mt_input || nullptr == mt_output)
        return 0;
    if(mt_input->get_major() != mt_output->get_major() || MMT_AUDIO != mt_input->get_major())
        return 0;
    if(false == mt_input->is_compress() || false == mt_output->is_compress())
        return 0;
    return 1;
}

ret_type ffmpeg_audio_resample::set_media_type(input_pin* pin,media_type* mt)
{
    if(nullptr != mt)
    {
        if(mt->get_sub() != MST_PCM)
            return rc_param_invalid;
        if(mt->get_audio_format() <= AMT_NONE || mt->get_audio_format() >= AMT_NB)
            return rc_param_invalid;
        if(mt->get_audio_channel() <= 0)
            return rc_param_invalid;
        if(mt->get_audio_sample_rate() <= 0)
            return rc_param_invalid;
        if(mt->get_audio_frame_size() <= 0)
            return rc_param_invalid;
    }
    return open(mt,_pin_output->get_media_type());
}

ret_type ffmpeg_audio_resample::set_media_type(output_pin* pin,media_type* mt)
{
    if(nullptr != mt)
    {
        if(mt->get_sub() != MST_PCM)
            return rc_param_invalid;
        if(mt->get_audio_format() <= AMT_NONE || mt->get_audio_format() >= AMT_NB)
            return rc_param_invalid;
        if(mt->get_audio_channel() <= 0)
            return rc_param_invalid;
        if(mt->get_audio_sample_rate() <= 0)
            return rc_param_invalid;
        if(mt->get_audio_frame_size() <= 0)
            return rc_param_invalid;
    }
    return open(_pin_input->get_media_type(),mt);
}

ret_type ffmpeg_audio_resample::open(media_type* mt_in,media_type* mt_out)
{
    ret_type rt = rc_ok;
    if(nullptr != mt_in && nullptr != mt_out)
    {
        if(!_mt_resample)
        {
            _mt_resample.reset(new media_type());
            JCHK(_mt_resample,rc_param_invalid)
        }
        JIF(media_type::copy(_mt_resample.get(),mt_out))
        _mt_resample->set_audio_frame_duration(mt_in->get_audio_frame_duration());
    }
    close();
    return rt;
}

ret_type ffmpeg_audio_resample::process(input_pin* pin,media_frame* frame)
{
    if(nullptr == frame)
        return audio_sample(frame);

    ret_type rt;
    media_type* mt_in = _pin_input->get_media_type();
    media_type* mt_out = _pin_output->get_media_type();

    AudioMediaType fmt_in = mt_in->get_audio_format();
    AudioMediaType fmt_out = mt_out->get_audio_format();
    int channel_in = mt_in->get_audio_channel();
    int channel_out = mt_out->get_audio_channel();
    int sample_rate_in = mt_in->get_audio_sample_rate();
    int sample_rate_out = mt_out->get_audio_sample_rate();
    if(fmt_in != fmt_out || channel_in != channel_out || sample_rate_in != sample_rate_out)
    {
        if(nullptr == _ctxSwr)
        {
            JCHK(_ctxSwr = swr_alloc(),rc_new_fail);

            JCHK(0 == av_opt_set_sample_fmt(_ctxSwr, "in_sample_fmt", (AVSampleFormat)fmt_in, 0),rc_param_invalid);
            JCHK(0 == av_opt_set_int(_ctxSwr, "in_channel_layout",    av_get_default_channel_layout(channel_in), 0),rc_param_invalid);
            JCHK(0 == av_opt_set_int(_ctxSwr, "in_sample_rate",       sample_rate_in, 0),rc_param_invalid);

            JCHK(0 == av_opt_set_sample_fmt(_ctxSwr, "out_sample_fmt", (AVSampleFormat)fmt_out, 0),rc_param_invalid);
            JCHK(0 == av_opt_set_int(_ctxSwr, "out_channel_layout",    av_get_default_channel_layout(channel_out), 0),rc_param_invalid);
            JCHK(0 == av_opt_set_int(_ctxSwr, "out_sample_rate",       sample_rate_out, 0),rc_param_invalid);

            int ret;
            char err[AV_ERROR_MAX_STRING_SIZE] = {0};
            JCHKM(0 <= (ret = swr_init(_ctxSwr)),rc_fail,
                FORMAT_STR("swr_init fail,message:%1%",
                %av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))
        }
        uint8_t *dataIn[AV_NUM_DATA_POINTERS];
        int linesizeIn[AV_NUM_DATA_POINTERS];
        JIF(convert_frame_to_array(mt_in,frame,(uint8_t**)dataIn,(int*)linesizeIn))

        std::shared_ptr<media_frame> frame_out(new media_frame());
        JCHK(frame_out,rc_new_fail)
        frame_out->_info = frame->_info;

        uint8_t *dataOut[AV_NUM_DATA_POINTERS];
        int linesizeOut[AV_NUM_DATA_POINTERS];
        JIF(convert_frame_to_array(_mt_resample.get(),frame_out.get(),(uint8_t**)dataOut,(int*)linesizeOut))

		int ret;
		char err[AV_ERROR_MAX_STRING_SIZE] = {0};
		JCHKM(0 <= (ret = swr_convert(_ctxSwr,dataOut,frame_out->_info.samples,(const uint8_t**)dataIn,frame->_info.samples)),
			rc_fail,FORMAT_STR("audio resample frame fail,message:%1%",%av_make_error_string(err,AV_ERROR_MAX_STRING_SIZE,ret)))

        return audio_sample(frame_out.get());
    }
    else
        return audio_sample(frame);
}

ret_type ffmpeg_audio_resample::audio_sample(media_frame* frame)
{
    if(nullptr == frame)
        return _pin_output->deliver(frame);

    media_type* mt_out = _pin_output->get_media_type();
    int frame_size_in = _mt_resample->get_audio_frame_size();
    int frame_size_out = mt_out->get_audio_frame_size();
    if(frame_size_in != frame_size_out)
    {

        ret_type rt;

		uint8_t *dataIn[AV_NUM_DATA_POINTERS];
		int linesizeIn[AV_NUM_DATA_POINTERS];

		uint8_t *dataOut[AV_NUM_DATA_POINTERS];
		int linesizeOut[AV_NUM_DATA_POINTERS];


        JIF(convert_frame_to_array(_mt_resample.get(),_frame.get(),(uint8_t**)dataIn,(int*)linesizeIn))

        unsigned int offsetIn = 0,deltaIn,deltaOut;

//        if(MEDIA_FRAME_NONE_TIMESTAMP == frame->pts)
//            frame->info.pts = MEDIA_FRAME_NONE_TIMESTAMP == m_info.pts ? 0 : m_info.pts;
//        if(frame->info.pts != frame->info.dts)
//            frame->info.dts = frame->info.pts;
//
//        if(MEDIA_FRAME_NONE_TIMESTAMP != m_info.pts)
//        {
//            int64_t delta = pFrame->info.pts - m_info.pts;
//            if(delta > m_info.duration || delta < -m_info.duration)
//            {
//                LOG(1,"audio sample frame PTS:%ldms timestamp exception prev frame PTS:%ldms delta:%ld",
//                    int(pFrame->info.pts/10000),int(m_info.pts/10000),delta);
//                if(0 > delta)
//                    delta = -m_info.duration/2;
//                m_info.pts += delta;
//                m_info.dts = m_info.pts;
//            }
//        }
//        else
//        {
//            m_info = pFrame->info;
//            m_info.samples = 0;
//        }
        if(MEDIA_FRAME_NONE_TIMESTAMP != _info_sample.pts)
        {
            _info_sample = frame->_info;
            _info_sample.samples = 0;
        }
        while((deltaIn = (frame->_info.samples - offsetIn)) >= (deltaOut = ((unsigned int)frame_size_out - _info_sample.samples)))
        {
            if(!_frame)
            {
                _frame.reset(new media_frame());
                JCHK(_frame,rc_new_fail)
                //m_info.duration = m_spFrameSample->info.duration;
            }
            JIF(convert_frame_to_array(mt_out,_frame.get(),(uint8_t**)dataOut,(int*)linesizeOut))
			JCHK(0 <= av_samples_copy(dataOut,
				dataIn,
				_info_sample.samples,
				offsetIn,
				deltaOut,
				_mt_resample->get_audio_channel(),
				(AVSampleFormat)_mt_resample->get_audio_format()),rc_fail);
            _frame->_info = _info_sample;
            _frame->_info.samples = frame_size_out;
            _pin_output->deliver(_frame.get());
            _frame.reset();
            _info_sample.pts += _info_sample.duration;
            _info_sample.dts += _info_sample.duration;
            offsetIn += deltaOut;
            _info_sample.samples = 0;
        }
        if(deltaIn > 0)
        {
            if(!_frame)
            {
                _frame.reset(new media_frame());
                JCHK(_frame,rc_new_fail)
                _info_sample.duration = _frame->_info.duration;
            }
            JIF(convert_frame_to_array(mt_out,_frame.get(),(uint8_t**)dataOut,(int*)linesizeOut))
			JCHK(0 <= av_samples_copy(dataOut,
				dataIn,
				_info_sample.samples,
				offsetIn,
				deltaIn,
				_mt_resample->get_audio_channel(),
				(AVSampleFormat)_mt_resample->get_audio_format()),rc_fail);
            _info_sample.samples += deltaIn;
        }
        return rc_ok;
    }
    else
        return _pin_output->deliver(frame);
}

void ffmpeg_audio_resample::close()
{
    if(nullptr != _ctxSwr)
    {
        swr_free(&_ctxSwr);
    }
    _info_sample.reset();
    _frame.reset();
    _mt_resample.reset();
}
