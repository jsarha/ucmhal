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

#ifndef UCMHALINSTREAM_H
#define UCMHALINSTREAM_H

#define LOG_TAG "ucmhal"
#define LOG_NDEBUG 0

#include <stdint.h>
#include <sys/types.h>
#include <cutils/log.h>

#include <string.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

#include "UcmHalTypes.h"

#include <map>
#include <utility>

namespace UcmHal {

class Dev;
class UseCaseMgr;
class InStream;

struct ucmhal_in {
	audio_stream_in android_in;
	InStream *me;
};

class InStream : public audio_stream_in {
public:
	InStream(Dev &dev,
	         UseCaseMgr &ucm,
	         audio_io_handle_t handle,
	         audio_devices_t devices,
	         struct audio_config *config);
	~InStream();

	uint32_t get_sample_rate() const;
	int set_sample_rate(uint32_t rate);
	size_t get_buffer_size() const;
	uint32_t get_channels() const;
	audio_format_t get_format() const;
	int set_format(audio_format_t format);
	int standby();
	int dump(int fd) const;
	int set_parameters(const char *kvpairs);
	char * get_parameters(const char *keys) const;
	int add_audio_effect(effect_handle_t effect) const;
	int remove_audio_effect(effect_handle_t effect) const;
	int set_gain(float gain);
	ssize_t read(void* buffer, size_t bytes);
	uint32_t get_input_frames_lost();

	audio_stream_in *audio_stream_in() { return &m_in.android_in; }
	static int check_parameters(audio_config_t *config);
	int modeUpdate(audio_mode_t mode);
private:
	ucmhal_in m_in;
	Dev &mDev;
	UseCaseMgr &mUcm;
	Mutex mLock;
	bool mStandby;
	audio_devices_t mDevices;
	audio_config_t mConfig;
};

}; // namespace UcmHal

#endif /* UCMHALDEV_H */
