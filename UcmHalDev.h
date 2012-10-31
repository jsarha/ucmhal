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

#ifndef UCMHALDEV_H
#define UCMHALDEV_H

#define LOG_TAG "ucmhal"
#define LOG_NDEBUG 0

#include <cutils/log.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

#include "UcmHalUseCaseMgr.h"
#include "UcmHalTypes.h"
#include "UcmHalParameters.h"

namespace UcmHal {

class OutStream;
class InStream;

class Dev : public audio_hw_device {
public:
	Dev(const hw_module_t* module, hw_device_t** device);
	~Dev();
	// Forward methods from audio_hw_device struct
	int open_output_stream(audio_io_handle_t handle,
						   audio_devices_t devices,
						   audio_output_flags_t flags,
						   struct audio_config *config,
						   struct audio_stream_out **stream_out);
	void close_output_stream(struct audio_stream_out *stream);
	int set_parameters(const char *kvpairs);
	char * get_parameters(const char *keys) const;
	int init_check() const;
	int set_voice_volume(float volume);
	int set_master_volume(float volume);
	int set_mode(audio_mode_t mode);
	int set_mic_mute(bool state);
	int get_mic_mute(bool *state) const;
	size_t get_input_buffer_size(const audio_config *config) const;
	int open_input_stream(audio_io_handle_t handle,
						  audio_devices_t devices,
						  struct audio_config *config,
						  struct audio_stream_in **stream_in);
	void close_input_stream(struct audio_stream_in *stream);
	uint32_t get_supported_devices() const;
	int dump(int fd) const;

	friend class OutStream;
	friend class InStream;
private:
	Mutex mLock;
	MacroMap mMM;
	UseCaseMgr mUcm;
	bool mInitStatus;
	audio_mode_t mMode;

	static const char *supportedParameters[];
	HookedParameters<Dev> mParameters;

	OutStreamSet_t mOutStreams;
	InStreamSet_t mInStreams;
};

}; // namespace UcmHal

#endif /* UCMHALDEV_H */
