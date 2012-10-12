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

#ifndef UCMHALOUTSTREAM_H
#define UCMHALOUTSTREAM_H

#define LOG_TAG "ucmhal"
#define LOG_NDEBUG 0

#include <stdint.h>
#include <sys/types.h>
#include <cutils/log.h>

#include <string.h>
#include <iostream>
#include <map>
#include <utility>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

namespace UcmHal {

class Dev;
class UseCaseMgr;

class OutStream : public audio_stream_out {
public:
	OutStream(Dev &dev,
	          UseCaseMgr &ucm,
	          audio_io_handle_t handle,
	          audio_devices_t devices,
	          audio_output_flags_t flags,
	          struct audio_config *config);
	~OutStream();

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
	uint32_t get_latency() const;
	int set_volume(float left, float right);
	ssize_t write(const void* buffer, size_t bytes);
	int get_render_position(uint32_t *dsp_frames) const;

private:
	Dev &mDev;
	UseCaseMgr &mUcm;
	pthread_mutex_t mLock;
	bool mStandby;
	audio_devices_t mDevices;
	audio_output_flags_t mFlags;
};

}; // namespace UcmHal

#endif /* UCMHALDEV_H */
