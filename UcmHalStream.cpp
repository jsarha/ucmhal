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
	// These are only defaults
	mConfig.channels = popcount(config->channel_mask);
	mConfig.rate = config->sample_rate;
	mConfig.period_size = 0;
	mConfig.period_count = 0;
	mConfig.format = PCM_FORMAT_S16_LE;
	mConfig.start_threshold = 0;
	mConfig.stop_threshold = 0;
	mConfig.silence_threshold = 0;
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

uint32_t Stream::get_channels() const {
    LOGFUNC("%s(%p)", __func__, this);
    if (mConfig.channels == 1) {
        return AUDIO_CHANNEL_IN_MONO;
    } else {
        return AUDIO_CHANNEL_IN_STEREO;
    }
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
