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

InStream::InStream(Dev &dev,
                   UseCaseMgr &ucm,
                   audio_io_handle_t handle,
                   audio_devices_t devices,
                   struct audio_config *config) :
	mDev(dev), mUcm(ucm), mStandby(true), mDevices(devices) {
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

	// FIXME this is a hack... I'll fix it when I have a proper configuration 
	// with separate playback and capture devices ready.
	uh_assert_se(mUcm.findEntry(mDev.mMode, AUDIO_DEVICE_OUT_SPEAKER, 0, mEntry));
	
	mConfig.channels = popcount(config->channel_mask);
	mConfig.rate = MM_FULL_POWER_SAMPLING_RATE;
	mConfig.period_size = SHORT_PERIOD_SIZE;
	mConfig.period_count = CAPTURE_PERIOD_COUNT;
	mConfig.format = PCM_FORMAT_S16_LE;
	mConfig.start_threshold = 0;
	mConfig.stop_threshold = 0;
	mConfig.silence_threshold = 0;
}

InStream::~InStream() {
}

uint32_t InStream::get_sample_rate() const {
    LOGFUNC("%s(%p)", __FUNCTION__, this);
	return mConfig.rate;
}

int InStream::set_sample_rate(uint32_t rate) {
	LOGFUNC("%s(%p, %d)", __FUNCTION__, this, rate);
	return 0;
}

size_t InStream::get_buffer_size() const {
	//TODO
	return 0;
}

uint32_t InStream::get_channels() const {
    LOGFUNC("%s(%p)", __FUNCTION__, this);
	return mConfig.channel_mask;
}

audio_format_t InStream::get_format() const {
    LOGFUNC("%s(%p)", __FUNCTION__, this);
	return mConfig.format;
}

int InStream::set_format(audio_format_t format) {
	LOGFUNC("%s(%p, %d)", __FUNCTION__, this, format);
	return 0;
}

int InStream::standby() {
	//TODO
	return 0;
}

int InStream::dump(int fd) const {
	LOGFUNC("%s(%p, %d)", __FUNCTION__, this, fd);
	return 0;
}

int InStream::set_parameters(const char *kvpairs) {
	//TODO
	return 0;
}

char * InStream::get_parameters(const char *keys) const {
	//TODO
	return 0;
}

int InStream::add_audio_effect(effect_handle_t effect) const {
	LOGFUNC("%s(%p, %d)", __FUNCTION__, this, effect);
	return 0;
}

int InStream::remove_audio_effect(effect_handle_t effect) const {
	LOGFUNC("%s(%p, %d)", __FUNCTION__, this, effect);
	return 0;
}

int InStream::set_gain(float gain) {
	//TODO
	return 0;
}

ssize_t InStream::read(void* buffer, size_t bytes) {
    int ret = 0;
    size_t frames_rq = bytes / 
	    audio_stream_frame_size(&audio_stream_in()->common);

    LOGFUNC("%s(%p, %p, %d)", __FUNCTION__, stream, buffer, bytes);

    /* acquiring hw device mutex systematically is useful if a low priority thread is waiting
     * on the input stream mutex - e.g. executing select_mode() while holding the hw device
     * mutex
     */
    mDev.mLock.lock();
    mLock.lock();
    if (mStandby) {
	    ret = startInputStream();
	    if (ret == 0)
		    mStandby = 0;
    }
    mDev.mLock.unlock();

    if (ret < 0)
        goto exit;

    ret = pcm_read(in->pcm, buffer, bytes);

    if (ret > 0)
        ret = 0;

    if (ret == 0 && adev->mic_mute)
        memset(buffer, 0, bytes);

exit:
    if (ret < 0)
        usleep(bytes * 1000000 / audio_stream_frame_size(&stream->common) /
               in_get_sample_rate(&stream->common));

    pthread_mutex_unlock(&in->lock);
    return bytes;
}


int InStream::startInputStream() {
    int ret = 0;

    LOGFUNC("%s(%p)", __FUNCTION__, this);
    // FIXME This is a hack
    if (mUcm.activeVerb().empty()) {
	    mUcm.activateEntry(mEntry);
    }
    int card = mUcm.getCaptureCard(mEntry);
    int port = mUcm.getCapturePort(mEntry);

    LOGE("setting capture card=%d port=%d", card, port);

    mPcm = pcm_open(card, port, PCM_IN, &in->config);
    if (!pcm_is_ready(in->pcm)) {
        LOGE("cannot open pcm_in driver: %s", pcm_get_error(in->pcm));
        pcm_close(in->pcm);
        return -ENOMEM;
    }
    return 0;
}

uint32_t InStream::get_input_frames_lost() {
	//TODO
	return 0;
}

/* The device lock should be kept between deviceUpdatePrepare and
   deviceUpdateFinish calls */
int InStream::deviceUpdatePrepare() {
// TODO
/*
	AutoMutex lock(mLock);
	uclist_t::iterator newEntry;
	uh_assert_se(mUcm.findEntry(mDev.mMode, mDevices, mFlags, newEntry));
	if (mEntry->equal(*newEntry))
		return 0;
	if (mEntry->active()) {
		if (!mStandby && mUcm.changeStandby(mEntry, newEntry))
			standby();
		else
			mUcm.deactivateEntry(mEntry);
	}
	mEntry = newEntry;
*/
	return 0;
}

int InStream::deviceUpdateFinish() {
// TODO
/*
	AutoMutex lock(mLock);
	mUcm.activateEntry(mEntry);
*/
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
