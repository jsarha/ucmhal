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

#include "UcmHalOutStream.h"
#include "UcmHalDev.h"
#include "UcmHalMacro.h"

namespace UcmHal {

uint32_t out_get_sample_rate(const struct audio_stream *stream) {
	return ((const OutStream *)((ucmhal_out *)stream)->me)->get_sample_rate();
}

int out_set_sample_rate(struct audio_stream *stream, uint32_t rate) {
	return ((ucmhal_out *) stream)->me->set_sample_rate(rate);
}

size_t out_get_buffer_size(const struct audio_stream *stream) {
	return ((const OutStream *)((ucmhal_out *)stream)->me)->get_buffer_size();
}

uint32_t out_get_channels(const struct audio_stream *stream) {
	return ((const OutStream *)((ucmhal_out *)stream)->me)->get_channels();
}

audio_format_t out_get_format(const struct audio_stream *stream) {
	return ((const OutStream *)((ucmhal_out *)stream)->me)->get_format();
}

int out_set_format(struct audio_stream *stream, audio_format_t format) {
	return ((ucmhal_out *) stream)->me->set_format(format);
}

int out_standby(struct audio_stream *stream) {
	return ((ucmhal_out *) stream)->me->standby();
}

int out_dump(const struct audio_stream *stream, int fd) {
	return ((const OutStream *)((ucmhal_out *)stream)->me)->dump(fd);
}

int out_set_parameters(struct audio_stream *stream, const char *kvpairs) {
	return ((ucmhal_out *) stream)->me->set_parameters(kvpairs);
}

char * out_get_parameters(const struct audio_stream *stream,
                          const char *keys) {
	return ((const OutStream *)((ucmhal_out *)stream)->me)->get_parameters(keys);
}

int out_add_audio_effect(const struct audio_stream *stream,
                         effect_handle_t effect) {
	return ((const OutStream *)((ucmhal_out *)stream)->me)->
		add_audio_effect(effect);
}

int out_remove_audio_effect(const struct audio_stream *stream,
                            effect_handle_t effect) {
	return ((const OutStream *)((ucmhal_out *)stream)->me)->
		remove_audio_effect(effect);
}

uint32_t out_get_latency(const struct audio_stream_out *stream) {
	return ((const OutStream *)((ucmhal_out *)stream)->me)->get_latency();
}

int out_set_volume(struct audio_stream_out *stream, float left, float right) {
	return ((ucmhal_out *) stream)->me->set_volume(left, right);
}

ssize_t out_write(struct audio_stream_out *stream, const void* buffer,
                  size_t bytes) {
	return ((ucmhal_out *) stream)->me->write(buffer, bytes);
}

int out_get_render_position(const struct audio_stream_out *stream,
                            uint32_t *dsp_frames) {
	return ((const OutStream *)((ucmhal_out *)stream)->me)->
		get_render_position(dsp_frames);
}

const char *OutStream::supportedParameters[] = {
	AUDIO_PARAMETER_STREAM_ROUTING,
	NULL
};

OutStream::OutStream(Dev &dev,
                     UseCaseMgr &ucm,
                     audio_io_handle_t handle,
                     audio_devices_t devices,
                     audio_output_flags_t flags,
                     struct audio_config *config) :
	Stream(dev, ucm, &mParameters, handle, devices, config, flags),
	mParameters(supportedParameters) {
	memset(&m_out, 0, sizeof(m_out));

	m_out.android_out.common.get_sample_rate = out_get_sample_rate;
	m_out.android_out.common.set_sample_rate = out_set_sample_rate;
	m_out.android_out.common.get_buffer_size = out_get_buffer_size;
	m_out.android_out.common.get_channels = out_get_channels;
	m_out.android_out.common.get_format = out_get_format;
	m_out.android_out.common.set_format = out_set_format;
	m_out.android_out.common.standby = out_standby;
	m_out.android_out.common.dump = out_dump;
	m_out.android_out.common.set_parameters = out_set_parameters;
	m_out.android_out.common.get_parameters = out_get_parameters;
	m_out.android_out.common.add_audio_effect = out_add_audio_effect;
	m_out.android_out.common.remove_audio_effect = out_remove_audio_effect;
	m_out.android_out.get_latency = out_get_latency;
	m_out.android_out.set_volume = out_set_volume;
	m_out.android_out.write = out_write;
	m_out.android_out.get_render_position = out_get_render_position;
	m_out.me = this;

	mParameters.set(AUDIO_PARAMETER_STREAM_ROUTING, devices);
	mParameters.setHook(this, &OutStream::routeUpdateHook,
	                    AUDIO_PARAMETER_STREAM_ROUTING);

	initPcmConfig(mEntry->mOutSettings, config);
}

OutStream::~OutStream() {
	standby();
}

size_t OutStream::get_buffer_size() const {
	LOGFUNC("%s(%p)", __func__, this);
	/* audioflinger expects audio buffers to be a multiple of 16 frames */
	return ((mConfig.period_size/2) & ~0xF) * audio_stream_frame_size(
		const_cast<struct audio_stream *>(&m_out.android_out.common));
}

uint32_t OutStream::get_channels() const {
    LOGFUNC("%s(%p)", __func__, this);
    if (mConfig.channels == 1) {
        return AUDIO_CHANNEL_OUT_MONO;
    } else {
        return AUDIO_CHANNEL_OUT_STEREO;
    }
}

uint32_t OutStream::get_latency() const {
	//LOGFUNC("%s(%p)", __func__, this);
	// TODO Should use pcm_get_latency(), but it is not implemented in tinyalsa.
	//      Something based pcm_get_htimestamp would probably do.
	return (mConfig.period_size * mConfig.period_count * 1000) / mConfig.rate;
}

int OutStream::set_volume(float left, float right) {
	LOGFUNC("%s(%p, %f, %f)", __func__, this, left, right);
	//TODO
	return 0;
}

ssize_t OutStream::write(const void* buffer, size_t bytes) {
	int ret;
	//LOGFUNC("%s(%p, %p, %d)", __func__, this, buffer, bytes);

	AutoMutex lock(mLock);
	do {
		if (mStandby)
			if (startStream())
				return -EBUSY;

		/* If we have more than mMaxThreshold frames in DMA buffer we go
		   to sleep until we have mMinThreshold of frames left in buffer.
		*/
		struct timespec time_stamp;
		unsigned int frames_free;
		if (pcm_get_htimestamp(mPcm, &frames_free, &time_stamp) >= 0) {
			int frames_to_play = pcm_get_buffer_size(mPcm) - frames_free;
			if (frames_to_play > mMaxThreshold) {
				useconds_t time = (((int64_t)(frames_to_play - mMinThreshold)
				                    * 1000000) / mConfig.rate);
				if (time > MIN_WRITE_SLEEP_US)
					usleep(time);
			}
		}
		ret = pcm_mmap_write(mPcm, buffer, bytes);

		if (ret == -EPIPE) {
			/* Recover from an underrun */
			ALOGE("XRUN detected");
			pcm_close(mPcm);
			mPcm = NULL;
			mStandby = 1;
		}
	} while(ret == -EPIPE);

	if (ret != 0) {
		ALOGD("pcm_mmap_write(%p, %p, %d) returned %d",
		      mPcm, buffer, bytes, ret);
		unsigned int usecs = bytes * 1000000 / mFrameSize / mConfig.rate;
		if (usecs >= 1000000L) {
			usecs = 999999L;
		}
		usleep(usecs);
	}

	return bytes;
}

int OutStream::get_render_position(uint32_t *dsp_frames) const {
	LOGFUNC("%s(%p, %p(%d))", __func__, this, dsp_frames, *dsp_frames);
	//TODO
	return 0;
}

/* must be called with output stream mutexes locked */
int OutStream::startStream()
{
	LOGFUNC("%s(%p)", __func__, this);

	assert(mEntry != mUcm.noEntry());
	mUcm.activateEntry(mEntry, this);
	int card = mUcm.getPlaybackCard(mEntry);
	int port = mUcm.getPlaybackPort(mEntry);

	ALOGE("setting playback card=%d port=%d", card, port);

	mPcm = pcm_open(card, port, PCM_OUT | PCM_MMAP, &mConfig);

	if (!pcm_is_ready(mPcm)) {
		ALOGE("cannot open pcm_out driver: %s", pcm_get_error(mPcm));
		LOGD("parameters: .channels = %d .rate = %d .period_size = %d "
			 ".period_count = %d .format = %d .start_threshold = %d "
			 ".stop_threshold = %d .silence_threshold = %d .avail_min = %d",
			 mConfig.channels, mConfig.rate, mConfig.period_size,
			 mConfig.period_count, mConfig.format, mConfig.start_threshold,
			 mConfig.stop_threshold, mConfig.silence_threshold,
			 mConfig.avail_min);

		pcm_close(mPcm);
		mPcm = NULL;
		mUcm.deactivateEntry(mEntry);
		return -EBUSY;
	}

	mStandby = 0;
	return 0;
}

void OutStream::routeUpdateHook() {
	AutoMutex dLock(mDev.mLock);
	AutoMutex sLock(mLock);
	int newDevices = -1;
	mParameters.get(AUDIO_PARAMETER_STREAM_ROUTING, newDevices);
	if (mDevices != newDevices && newDevices != -1) {
		ALOGD("Devices changed from 0x%08x to 0x%08x", mDevices, newDevices);
		mDevices = (audio_devices_t) newDevices;
		mDbgStr.clear();
		deviceUpdatePrepare();
		deviceUpdateFinish();
	}
	else
		ALOGE("Bogus device update 0x%08x", newDevices);
}

}; // namespace UcmHal
