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
	mDev(dev), mUcm(ucm), mStandby(true), mDevices(devices), mFlags(flags),
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

	uh_assert_se(0 == mUcm.findEntry(mDev.mMode, mDevices, mFlags, mEntry));

	mConfig.channels = 2;
	mConfig.rate = DEFAULT_OUT_SAMPLING_RATE;
	mConfig.period_size = LONG_PERIOD_SIZE;
	mConfig.period_count = PLAYBACK_PERIOD_COUNT;
	mConfig.format = PCM_FORMAT_S16_LE;
	mConfig.start_threshold = 0;
	mConfig.stop_threshold = 0;
	mConfig.silence_threshold = 0;

	mFrameSize = 0;
	mWriteMaxThreshold = 0;
	mWriteMinThreshold = 0;
	mPcm = NULL;

	config->format = AUDIO_FORMAT_PCM_16_BIT;
	config->channel_mask = AUDIO_CHANNEL_OUT_STEREO;
	config->sample_rate = get_sample_rate();
}

OutStream::~OutStream() {
	standby();
}

uint32_t OutStream::get_sample_rate() const {
	LOGFUNC("%s(%p)", __func__, this);
	return 44100;
}

int OutStream::set_sample_rate(uint32_t rate) {
	LOGFUNC("%s(%p, %d)", __func__, this, rate);
	return 0;
}

size_t OutStream::get_buffer_size() const {
	LOGFUNC("%s(%p)", __func__, this);
	/* audioflinger expects audio buffers to be a multiple of 16 frames */
	return ((mConfig.period_size/2) & ~0xF) * audio_stream_frame_size(
		const_cast<struct audio_stream *>(&m_out.android_out.common));
}

uint32_t OutStream::get_channels() const {
	return AUDIO_CHANNEL_OUT_STEREO;
}

audio_format_t OutStream::get_format() const {
	return AUDIO_FORMAT_PCM_16_BIT;
}

int OutStream::set_format(audio_format_t format) {
	LOGFUNC("%s(%p, %d)", __func__, this, format);
	return 0;
}

int OutStream::standby() {
	LOGFUNC("%s(%p)", __func__, this);
	// TODO Do we really need the device lock here???
	//AutoMutex dLock(mDev.mLock);
	AutoMutex sLock(mLock);
	if (!mStandby) {
		pcm_close(mPcm);
		mPcm = NULL;
		mUcm.deactivateEntry(mEntry);
		mStandby = 1;
	}
	return 0;
}

int OutStream::dump(int fd) const {
	LOGFUNC("%s(%p)", __func__, this);
	return 0;
}

int OutStream::set_parameters(const char *kvpairs) {
	LOGFUNC("%s(%p, %s)", __func__, this, kvpairs);
	mParameters.updateTrigger(kvpairs);
	return 0;
}

char * OutStream::get_parameters(const char *keys) const {
	LOGFUNC("%s(%p, %s)", __func__, this, keys);
	// TODO this is broken
	return mParameters.toStr();
}

int OutStream::add_audio_effect(effect_handle_t effect) const {
	LOGFUNC("%s(%p, %p)", __func__, this, effect);
	//TODO
	return 0;
}

int OutStream::remove_audio_effect(effect_handle_t effect) const {
	LOGFUNC("%s(%p, %p)", __func__, this, effect);
	//TODO
	return 0;
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

		/* If we have more than mWriteMaxThreshold frames in DMA buffer we go
		   to sleep until we have mWriteMinThreshold of frames left in buffer.
		*/
		struct timespec time_stamp;
		unsigned int frames_free;
		if (pcm_get_htimestamp(mPcm, &frames_free, &time_stamp) >= 0) {
			int frames_to_play = pcm_get_buffer_size(mPcm) - frames_free;
			if (frames_to_play > mWriteMaxThreshold) {
				useconds_t time = (((int64_t)(frames_to_play - mWriteMinThreshold)
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
			mStandby = 0;
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
	mUcm.activateEntry(mEntry);
	int card = mUcm.getPlaybackCard(mEntry);
	int port = mUcm.getPlaybackPort(mEntry);

	ALOGE("setting playback card=%d port=%d", card, port);

	/* default to low power:
	 *	NOTE: PCM_NOIRQ mode is required to dynamically scale avail_min
	 */
	mFrameSize = audio_stream_frame_size(&m_out.android_out.common);
	mWriteMaxThreshold = mConfig.period_size * (mConfig.period_count - 1);
	mWriteMinThreshold = mConfig.period_size;
	mConfig.start_threshold = mConfig.period_size;
	// TODO avail_min is not available in my testenvironment
	// mConfig.avail_min = LONG_PERIOD_SIZE,

	mPcm = pcm_open(card, port, PCM_OUT | PCM_MMAP, &mConfig);

	if (!pcm_is_ready(mPcm)) {
		ALOGE("cannot open pcm_out driver: %s", pcm_get_error(mPcm));
		pcm_close(mPcm);
		mPcm = NULL;
		return -EBUSY;
	}

	mStandby = 0;
	return 0;
}

/* The device lock should be kept between deviceUpdatePrepare and
   deviceUpdateFinish calls */
int OutStream::deviceUpdatePrepare() {
	AutoMutex lock(mLock);
	uclist_t::iterator newEntry;
	uh_assert_se(0 == mUcm.findEntry(mDev.mMode, mDevices, mFlags, newEntry));
	if (mEntry->equal(*newEntry))
		return 0;
	if (mEntry->active()) {
		if (!mStandby && mUcm.changeStandby(mEntry, newEntry))
			standby();
		else
			mUcm.deactivateEntry(mEntry);
	}
	mEntry = newEntry;
	return 0;
}

int OutStream::deviceUpdateFinish() {
	AutoMutex lock(mLock);
	if (!mStandby)
		mUcm.activateEntry(mEntry);
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
		deviceUpdatePrepare();
		deviceUpdateFinish();
	}
	else
		ALOGE("Bogus device update 0x%08x", newDevices);
}

}; // namespace UcmHal
