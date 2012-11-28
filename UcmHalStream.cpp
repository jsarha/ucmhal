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

#include "UcmHalTypes.h"

#include <typeinfo>

#include "UcmHalDev.h"
#include "UcmHalStream.h"
#include "UcmHalMacro.h"
#include "UcmHalUseCaseMgr.h"

namespace UcmHal {

void Stream::initPcmConfig(UseCaseMapEntry::pcm_settings pcm_settings,
						   struct audio_config *config) {
	memset(&mConfig, 0, sizeof(mConfig));
	if (pcm_settings.channel_counts.find(popcount(config->channel_mask)) ==
		pcm_settings.channel_counts.end()) {
		LOGV("Requested channel count %d not found, using default %d",
			 popcount(config->channel_mask), pcm_settings.default_channels);
		mConfig.channels = pcm_settings.default_channels;
	}
	else {
		LOGV("Requested channel count %d found", popcount(config->channel_mask));
		mConfig.channels = popcount(config->channel_mask);
	}

	if (pcm_settings.rates.find(config->sample_rate) ==
		pcm_settings.rates.end()) {
		LOGV("Requested rate %d not found, using default %d",
			 config->sample_rate, pcm_settings.default_rate);
		mConfig.rate = pcm_settings.default_rate;
	}
	else {
		LOGV("Requested rate %d found", config->sample_rate);
		mConfig.rate = config->sample_rate;
	}


	LOGD("pcm config req: channels %d rate %d got: channels %d rate %d",
		 popcount(config->channel_mask), config->sample_rate,
		 mConfig.channels, mConfig.rate);

	mConfig.period_size = pcm_settings.period_size;
	mConfig.period_count = pcm_settings.period_count;
	mConfig.format = PCM_FORMAT_S16_LE;
	mConfig.start_threshold = pcm_settings.period_size;
	mConfig.stop_threshold = 0;
	mConfig.silence_threshold = 0;
	mConfig.avail_min = pcm_settings.min_threshold;

	LOGD("parameters: .channels = %d .rate = %d .period_size = %d "
		 ".period_count = %d .format = %d .start_threshold = %d "
		 ".stop_threshold = %d .silence_threshold = %d .avail_min = %d",
		 mConfig.channels, mConfig.rate, mConfig.period_size,
		 mConfig.period_count, mConfig.format, mConfig.start_threshold,
		 mConfig.stop_threshold, mConfig.silence_threshold,
		 mConfig.avail_min);

	mMinThreshold = pcm_settings.min_threshold;
	mMaxThreshold = pcm_settings.max_threshold;

	config->channel_mask = get_channels();
	config->sample_rate = get_sample_rate();
	config->format = get_format();

	mFrameSize = audio_stream_frame_size(audio_stream());
}

Stream::Stream(Dev &dev,
               UseCaseMgr &ucm,
               Parameters *parameters,
               audio_io_handle_t handle,
               audio_devices_t devices,
               struct audio_config *config,
               audio_output_flags_t flags) :
	mDev(dev), mUcm(ucm), mParametersPtr(parameters), mStandby(true),
	mDevices(devices), mFlags(flags), mFrameSize(0), mPcm(NULL) {

	uh_assert_se(0 == mUcm.findEntry(mEntry, mDev.mMode, mDevices, mFlags));
}

Stream::~Stream() {
}

uint32_t Stream::get_sample_rate() const {
    LOGFUNC("%s(%p)", __func__, this);
	return mConfig.rate;
}

int Stream::set_sample_rate(uint32_t rate) {
	LOGFUNC("%s(%p, %d)", __func__, this, rate);
	return 0;
}

uint32_t Stream::get_buffer_size() const {
	LOGFUNC("%s(%p)", __func__, this);
	return mConfig.period_size;
}

audio_format_t Stream::get_format() const {
    LOGFUNC("%s(%p)", __func__, this);
	return AUDIO_FORMAT_PCM_16_BIT;
}

int Stream::set_format(audio_format_t format) {
	LOGFUNC("%s(%p, %d)", __func__, this, format);
	return 0;
}

int Stream::standby() {
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

int Stream::dump(int fd) const {
	LOGFUNC("%s(%p, %d)", __func__, this, fd);
	return 0;
}

int Stream::set_parameters(const char *kvpairs) {
	LOGFUNC("%s(%p, %s)", __func__, this, kvpairs);
	mParametersPtr->updateTrigger(kvpairs);
	return 0;
}

char * Stream::get_parameters(const char *keys) const {
	LOGFUNC("%s(%p, %s)", __func__, this, keys);
	//TODO should be filtered according to keys
	return mParametersPtr->toStr();
}

int Stream::add_audio_effect(effect_handle_t effect) const {
	LOGFUNC("%s(%p, %d)", __func__, this, effect);
	return 0;
}

int Stream::remove_audio_effect(effect_handle_t effect) const {
	LOGFUNC("%s(%p, %d)", __func__, this, effect);
	return 0;
}

const string &Stream::dbgStr() {
	if (mDbgStr.empty()) {
		mDbgStr.append(typeid(*this).name());
		mDbgStr.append(" devices: ");
		mDev.mMM.deviceStr(mDevices, mDbgStr);
		if (mFlags) {
			mDbgStr.append(" flags : ");
			mDev.mMM.flagStr(mFlags, mDbgStr);
		}
	}
	return mDbgStr;
}

/* The device lock should be kept between deviceUpdatePrepare and
   deviceUpdateFinish calls */
int Stream::deviceUpdatePrepare() {
	AutoMutex lock(mLock);
	uclist_t::iterator newEntry;
	uh_assert_se(mUcm.findEntry(newEntry, mDev.mMode, mDevices));
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

int Stream::deviceUpdateFinish() {
	AutoMutex lock(mLock);
	if (!mStandby)
		mUcm.activateEntry(mEntry, this);
	return 0;
}

}; // namespace UcmHal
