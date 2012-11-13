/*
 * Copyright (C) 2012 Texas Instruments
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "UcmHalDev.h"
#include "UcmHalInStream.h"
#include "UcmHalMacro.h"
#include "UcmHalUseCaseMgr.h"

namespace UcmHal {

uint32_t in_get_sample_rate(const struct audio_stream *stream) {
	return ((const InStream *)((ucmhal_in *)stream)->me)->get_sample_rate();
}

int in_set_sample_rate(struct audio_stream *stream, uint32_t rate) {
	return ((ucmhal_in *)stream)->me->set_sample_rate(rate);
}

size_t in_get_buffer_size(const struct audio_stream *stream) {
	return ((const InStream *)((ucmhal_in *)stream)->me)->get_buffer_size();
}

uint32_t in_get_channels(const struct audio_stream *stream) {
	return ((const InStream *)((ucmhal_in *)stream)->me)->get_channels();
}

audio_format_t in_get_format(const struct audio_stream *stream) {
	return ((const InStream *)((ucmhal_in *)stream)->me)->get_format();
}

int in_set_format(struct audio_stream *stream, audio_format_t format) {
	return ((ucmhal_in *)stream)->me->set_format(format);
}

int in_standby(struct audio_stream *stream) {
	return ((ucmhal_in *)stream)->me->standby();
}

int in_dump(const struct audio_stream *stream, int fd) {
	return ((const InStream *)((ucmhal_in *)stream)->me)->dump(fd);
}

int in_set_parameters(struct audio_stream *stream, const char *kvpairs) {
	return ((ucmhal_in *)stream)->me->set_parameters(kvpairs);
}

char * in_get_parameters(const struct audio_stream *stream,
                         const char *keys) {
	return ((const InStream *)((ucmhal_in *)stream)->me)->get_parameters(keys);
}

int in_add_audio_effect(const struct audio_stream *stream,
                        effect_handle_t effect) {
	return ((const InStream *)((ucmhal_in *)stream)->me)->
		add_audio_effect(effect);
}

int in_remove_audio_effect(const struct audio_stream *stream,
                           effect_handle_t effect) {
	return ((const InStream *)((ucmhal_in *)stream)->me)->
		remove_audio_effect(effect);
}

int in_set_gain(struct audio_stream_in *stream, float gain) {
	return ((ucmhal_in *)stream)->me->set_gain(gain);
}

ssize_t in_read(struct audio_stream_in *stream, void* buffer, size_t bytes) {
	return ((ucmhal_in *)stream)->me->read(buffer, bytes);
}

uint32_t in_get_input_frames_lost(struct audio_stream_in *stream) {
	return ((ucmhal_in *)stream)->me->get_input_frames_lost();
}

const char *InStream::supportedParameters[] = {
	AUDIO_PARAMETER_STREAM_INPUT_SOURCE,
	AUDIO_PARAMETER_STREAM_ROUTING,
	NULL
};

InStream::InStream(Dev &dev,
                   UseCaseMgr &ucm,
                   audio_io_handle_t handle,
                   audio_devices_t devices,
                   struct audio_config *config) :
	Stream(dev, ucm, &mParameters, handle, devices, config),
	mParameters(supportedParameters) {
	memset(&m_in, 0, sizeof(m_in));

	m_in.android_in.common.get_sample_rate = in_get_sample_rate;
	m_in.android_in.common.set_sample_rate = in_set_sample_rate;
	m_in.android_in.common.get_buffer_size = in_get_buffer_size;
	m_in.android_in.common.get_channels = in_get_channels;
	m_in.android_in.common.get_format = in_get_format;
	m_in.android_in.common.set_format = in_set_format;
	m_in.android_in.common.standby = in_standby;
	m_in.android_in.common.dump = in_dump;
	m_in.android_in.common.set_parameters = in_set_parameters;
	m_in.android_in.common.get_parameters = in_get_parameters;
	m_in.android_in.common.add_audio_effect = in_add_audio_effect;
	m_in.android_in.common.remove_audio_effect = in_remove_audio_effect;
	m_in.android_in.set_gain = in_set_gain;
	m_in.android_in.read = in_read;
	m_in.android_in.get_input_frames_lost = in_get_input_frames_lost;
	m_in.me = this;

	mConfig.rate = MM_FULL_POWER_SAMPLING_RATE;
	mConfig.period_size = SHORT_PERIOD_SIZE;
	mConfig.period_count = CAPTURE_PERIOD_COUNT;
}

InStream::~InStream() {
}

size_t InStream::get_buffer_size() const {
	LOGFUNC("%s(%p)", __func__, this);
	return mConfig.period_size;
}

int InStream::set_gain(float gain) {
	LOGFUNC("%s(%p)", __func__, this);
	//TODO
	return 0;
}

ssize_t InStream::read(void* buffer, size_t bytes) {
	int ret = 0;

	LOGFUNC("%s(%p, %p, %d)", __func__, this, buffer, bytes);

	AutoMutex lock(mLock);
	if (mStandby) {
		if (startInputStream())
			return -EBUSY;
	}

	ret = pcm_read(mPcm, buffer, bytes);
	ALOGV("pcm_read(%p, %p, %d) returned %d", mPcm, buffer, bytes, ret);

	if (ret >= 0 && mDev.mMicMute)
		memset(buffer, 0, bytes);
	if (ret < 0) {
		ALOGE("pcm_read(%p, %p, %d) returned %d", mPcm, buffer, bytes, ret);
		usleep(bytes * 1000000 /
		       audio_stream_frame_size(&audio_stream_in()->common) /
		       mConfig.rate);
	}
    return bytes;
}


int InStream::startInputStream() {
    LOGFUNC("%s(%p)", __func__, this);

    mUcm.activateEntry(mEntry, this);
    int card = mUcm.getCaptureCard(mEntry);
    int port = mUcm.getCapturePort(mEntry);

    ALOGE("setting capture card=%d port=%d", card, port);

    mPcm = pcm_open(card, port, PCM_IN, &mConfig);
    if (!pcm_is_ready(mPcm)) {
        ALOGE("cannot open pcm_in driver: %s", pcm_get_error(mPcm));
        pcm_close(mPcm);
        mPcm = NULL;
		mUcm.deactivateEntry(mEntry);
        return -ENOMEM;
    }
    mStandby = false;
    return 0;
}

uint32_t InStream::get_input_frames_lost() {
    LOGFUNC("%s(%p)", __func__, this);
	//TODO
	return 0;
}

int InStream::check_parameters(audio_config_t *config)
{
	uh_assert(config);

	if (config->format != AUDIO_FORMAT_PCM_16_BIT) {
		return -EINVAL;
	}

	/* This does not look right to me
	int channel_count = popcount(config->channel_mask);
	if ((channel_count < 1) || (channel_count > 2)) {
		return -EINVAL;
	}
	*/
	if (config->channel_mask != AUDIO_CHANNEL_IN_MONO &&
	    config->channel_mask != AUDIO_CHANNEL_IN_STEREO)
		return -EINVAL;

	switch(config->sample_rate) {
	case 8000:
	case 11025:
	case 16000:
	case 22050:
	case 24000:
	case 32000:
	case 44100:
	case 48000:
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

}; // namespace UcmHal
