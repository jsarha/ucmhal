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

OutStream::OutStream(Dev &dev,
					 UseCaseMgr &ucm,
					 audio_io_handle_t handle,
					 audio_devices_t devices,
					 audio_output_flags_t flags,
					 struct audio_config *config) :
	mDev(dev), mUcm(ucm), mStandby(true), mDevices(devices), mFlags(flags) {
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

	uh_assert_se(mUcm.findEntry(mDev.mMode, mDevices, mFlags, mEntry));

	mConfig.channels = 2;
	mConfig.rate = DEFAULT_OUT_SAMPLING_RATE;
	mConfig.period_size = LONG_PERIOD_SIZE;
	mConfig.period_count = PLAYBACK_PERIOD_COUNT;
	mConfig.format = PCM_FORMAT_S16_LE;
	mConfig.start_threshold = 0;
	mConfig.stop_threshold = 0;
	mConfig.silence_threshold = 0;

	mWriteThreshold = 0;
	mPcm = NULL;

	config->format = AUDIO_FORMAT_PCM_16_BIT;
	config->channel_mask = AUDIO_CHANNEL_OUT_STEREO;
	config->sample_rate = get_sample_rate();
}

OutStream::~OutStream() {
	standby();
}

uint32_t OutStream::get_sample_rate() const {
	return 44100;
}

int OutStream::set_sample_rate(uint32_t rate) {
	return 0;
}

size_t OutStream::get_buffer_size() const {
	//TODO
	return 0;
}

uint32_t OutStream::get_channels() const {
	return AUDIO_CHANNEL_OUT_STEREO;
}

audio_format_t OutStream::get_format() const {
	return AUDIO_FORMAT_PCM_16_BIT;
}

int OutStream::set_format(audio_format_t format) {
	return 0;
}

int OutStream::standby() {
	LOGFUNC("%s(%p)", __FUNCTION__, this);
	// TODO Do we really need the device lock here???
	AutoMutex dLock(mDev.mLock);
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
	//TODO
	return 0;
}

int OutStream::set_parameters(const char *kvpairs) {
	//TODO
	return 0;
}

char * OutStream::get_parameters(const char *keys) const {
	//TODO
	return 0;
}

int OutStream::add_audio_effect(effect_handle_t effect) const {
	//TODO
	return 0;
}

int OutStream::remove_audio_effect(effect_handle_t effect) const {
	//TODO
	return 0;
}

uint32_t OutStream::get_latency() const {
	//TODO
	return 0;
}

int OutStream::set_volume(float left, float right) {
	//TODO
	return 0;
}

ssize_t OutStream::write(const void* buffer, size_t bytes) {
	int ret;
	size_t frame_size = audio_stream_frame_size(&audio_stream_out()->common);
	size_t out_frames = bytes / frame_size;
	bool force_input_standby = false;
	int kernel_frames;

	LOGFUNC("%s(%p, %p, %d)", __FUNCTION__, this, buffer, bytes);

do_over:
	/* acquiring hw device mutex systematically is useful if a low priority thread is waiting
	 * on the output stream mutex - e.g. executing select_mode() while holding the hw device
	 * mutex
	 */
	{
		AutoMutex lock(mDev.mLock);
		mLock.lock();
		if (mStandby) {
			ret = startStream();
			if (ret != 0)
				goto exit;
			/* a change in output device may change the microphone selection */
			// TODO
		}
	}

	/* do not allow more than out->write_threshold frames in kernel pcm driver buffer */
	do {
		struct timespec time_stamp;

		if (pcm_get_htimestamp(mPcm, (unsigned int *)&kernel_frames, &time_stamp) < 0)
			break;
		kernel_frames = pcm_get_buffer_size(mPcm) - kernel_frames;
		if (kernel_frames > mWriteThreshold) {
			unsigned long time = (unsigned long)
				(((int64_t)(kernel_frames - mWriteThreshold) * 1000000) /
				 MM_FULL_POWER_SAMPLING_RATE);
			if (time < MIN_WRITE_SLEEP_US)
				time = MIN_WRITE_SLEEP_US;
			usleep(time);
		}
	} while (kernel_frames > mWriteThreshold);

	ret = pcm_mmap_write(mPcm, buffer, out_frames * frame_size);

exit:
	if (ret != 0) {
		unsigned int usecs = bytes * 1000000 / frame_size /
			out_get_sample_rate(&audio_stream_out()->common);
		if (usecs >= 1000000L) {
			usecs = 999999L;
		}
		usleep(usecs);
	}

	mLock.unlock();

	if (ret == -EPIPE) {
		/* Recover from an underrun */
		LOGE("XRUN detected");
		standby();
		goto do_over;
	}

	if (force_input_standby) {
		// TODO
		LOGE("force_input_standby not implemented");
	}

	return bytes;
}

int OutStream::get_render_position(uint32_t *dsp_frames) const {
	//TODO
	return 0;
}

/* must be called with hw device and output stream mutexes locked */
int OutStream::startStream()
{
	LOGFUNC("%s(%p)", __FUNCTION__, this);

	assert(mEntry != mUcm.noEntry());
	mUcm.activateEntry(mEntry);
	int card = mUcm.getPlaybackCard(mEntry);
	int port = mUcm.getPlaybackPort(mEntry);

	LOGE("setting playback card=%d port=%d", card, port);

	/* default to low power:
	 *	NOTE: PCM_NOIRQ mode is required to dynamically scale avail_min
	 */
	mWriteThreshold = PLAYBACK_PERIOD_COUNT * LONG_PERIOD_SIZE;
	mConfig.start_threshold = SHORT_PERIOD_SIZE * 2;
	// TODO avail_min is not available in my testenvironment
	// mConfig.avail_min = LONG_PERIOD_SIZE,

	mPcm = pcm_open(card, port, PCM_OUT | PCM_MMAP, &mConfig);

	if (!pcm_is_ready(mPcm)) {
		LOGE("cannot open pcm_out driver: %s", pcm_get_error(mPcm));
		pcm_close(mPcm);
		mPcm = NULL;
		return -ENOMEM;
	}

	mStandby = 0;
	return 0;
}

int OutStream::modeUpdate(audio_mode_t mode) {
	AutoMutex lock(mLock);
	uclist_t::iterator newEntry;
	uh_assert_se(mUcm.findEntry(mode, mDevices, mFlags, newEntry));
	if (mEntry->active()) {
		if (mEntry->equal(*newEntry))
			return 0;
		if (mUcm.changeStandby(mEntry, newEntry))
			standby();
		else
			mUcm.deactivateEntry(mEntry);
		mUcm.deactivateEntry(mEntry);
	}
	return 0;
}

}; // namespace UcmHal
