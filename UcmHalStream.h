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

#ifndef UCMHALSTREAM_H
#define UCMHALSTREAM_H

#define LOG_TAG "ucmhal"
#define LOG_NDEBUG 0

#include <stdint.h>
#include <sys/types.h>
#include <cutils/log.h>

#include <string.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

#include <tinyalsa/asoundlib.h>

#include "UcmHalTypes.h"

#include <map>
#include <utility>

namespace UcmHal {

class Dev;
class UseCaseMgr;
class Parameters;

class Stream {
public:
	Stream(Dev &dev,
	       UseCaseMgr &ucm,
	       Parameters *parameters,
	       audio_io_handle_t handle,
	       audio_devices_t devices,
	       struct audio_config *config,
	       audio_output_flags_t flags = AUDIO_OUTPUT_FLAG_NONE);
	~Stream();

	virtual uint32_t get_sample_rate() const;
	virtual int set_sample_rate(uint32_t rate);
	virtual size_t get_buffer_size() const;
	virtual uint32_t get_channels() const = 0;
	virtual audio_format_t get_format() const;
	virtual int set_format(audio_format_t format);
	virtual int standby();
	virtual int dump(int fd) const;
	virtual int set_parameters(const char *kvpairs);
	virtual char * get_parameters(const char *keys) const;
	virtual int add_audio_effect(effect_handle_t effect) const;
	virtual int remove_audio_effect(effect_handle_t effect) const;

	const uclist_t::iterator &getUcmEntry() { return mEntry; }
	const string &dbgStr();

	virtual struct audio_stream *audio_stream() = 0;
	virtual int deviceUpdatePrepare();
	virtual int deviceUpdateFinish();

protected:
	Dev &mDev;
	UseCaseMgr &mUcm;
	Parameters *mParametersPtr;
	Mutex mLock;
	bool mStandby;
	audio_devices_t mDevices;
	audio_output_flags_t mFlags;

   	uclist_t::iterator mEntry;

	pcm_config mConfig;
	int mFrameSize;
	int mMinThreshold;
	int mMaxThreshold;
	pcm *mPcm;

	string mDbgStr;

	void initPcmConfig(UseCaseMapEntry::pcm_settings pcm_settings,
					   struct audio_config *config);
};

}; // namespace UcmHal

#endif /* UCMHALDEV_H */
