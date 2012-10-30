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

#include "UcmHalDev.h"
#include "UcmHalOutStream.h"
#include "UcmHalInStream.h"
#include "UcmHalMacro.h"

extern "C"
int ucmhal_adev_open(const hw_module_t* module, const char* name,
					 hw_device_t** device)
{
	if (strcmp(name, AUDIO_HARDWARE_INTERFACE) != 0)
		return -EINVAL;

	UcmHal::Dev *dev = new UcmHal::Dev(module, device);

	if (!dev)
		return -ENOMEM;

	int ret = dev->init_check();
	if (ret)
		delete dev;

	return ret;
}

namespace UcmHal {

static int adev_close(hw_device_t *device) {
	Dev *dev = (Dev *) device;
	LOGE("Closing audio device");
	delete dev;
	return 0;
}

static int adev_open_output_stream(audio_hw_device *dev,
                                   audio_io_handle_t handle,
                                   audio_devices_t devices,
                                   audio_output_flags_t flags,
                                   struct audio_config *config,
                                   struct audio_stream_out **stream_out) {
	return ((Dev *)dev)->open_output_stream(handle, devices, flags, config,
											stream_out);
}

static void adev_close_output_stream(audio_hw_device *dev,
                                     struct audio_stream_out *stream) {
	((Dev *)dev)->close_output_stream(stream);
}

static int adev_set_parameters(audio_hw_device *dev, const char *kvpairs) {
	return ((Dev *)dev)->set_parameters(kvpairs);
}

static char *adev_get_parameters(const audio_hw_device *dev, const char *keys) {
	return ((const Dev *)dev)->get_parameters(keys);
}

static int adev_init_check(const audio_hw_device *dev) {
	return ((const Dev *)dev)->init_check();
}

static int adev_set_voice_volume(audio_hw_device *dev, float volume) {
	return ((Dev *)dev)->set_voice_volume(volume);
}

static int adev_set_master_volume(audio_hw_device *dev, float volume) {
	return ((Dev *)dev)->set_master_volume(volume);
}

static int adev_set_mode(audio_hw_device *dev, audio_mode_t mode) {
	return ((Dev *)dev)->set_mode(mode);
}

static int adev_set_mic_mute(audio_hw_device *dev, bool state) {
	return ((Dev *)dev)->set_mic_mute(state);
}

static int adev_get_mic_mute(const audio_hw_device *dev, bool *state) {
	return ((const Dev *)dev)->get_mic_mute(state);
}

static size_t adev_get_input_buffer_size(const audio_hw_device *dev,
										 const audio_config *config) {
	return ((const Dev *)dev)->get_input_buffer_size(config);
}

static int adev_open_input_stream(audio_hw_device *dev,
                                  audio_io_handle_t handle,
                                  audio_devices_t devices,
                                  struct audio_config *config,
                                  struct audio_stream_in **stream_in) {
	return ((Dev *)dev)->open_input_stream(handle, devices, config, stream_in);
}

static void adev_close_input_stream(audio_hw_device *dev, struct audio_stream_in *stream) {
	((Dev *)dev)->close_input_stream(stream);
}

static int adev_dump(const audio_hw_device *dev, int fd) {
	return ((Dev *)dev)->dump(fd);
}

static uint32_t adev_get_supported_devices(const audio_hw_device *dev) {
	return ((const Dev *)dev)->get_supported_devices();
}

const char *Dev::supportedParameters[] = { 
	AUDIO_PARAMETER_KEY_TTY_MODE,
	AUDIO_PARAMETER_KEY_BT_NREC,
	AUDIO_PARAMETER_KEY_SCREEN_STATE,
	NULL
};

Dev::Dev(const hw_module_t* module, hw_device_t** device) :
	mUcm(mMM),
	mInitStatus(false),
	mMode(AUDIO_MODE_NORMAL),
	mParameters(supportedParameters) {
	// C++ compiler does not allow direct assignment of function pointer members
	audio_hw_device *hw_device = reinterpret_cast<audio_hw_device *>(this);
	memset(hw_device, 0, sizeof(hw_device));

	common.tag = HARDWARE_DEVICE_TAG;
	common.version = AUDIO_DEVICE_API_VERSION_CURRENT;
	common.module = (struct hw_module_t *) module;
	common.close = adev_close;

	hw_device->get_supported_devices = adev_get_supported_devices;
	hw_device->init_check = adev_init_check;
	hw_device->set_voice_volume = adev_set_voice_volume;
	hw_device->set_master_volume = adev_set_master_volume;
	hw_device->set_mode = adev_set_mode;
	hw_device->set_mic_mute = adev_set_mic_mute;
	hw_device->get_mic_mute = adev_get_mic_mute;
	hw_device->set_parameters = adev_set_parameters;
	hw_device->get_parameters = adev_get_parameters;
	hw_device->get_input_buffer_size = adev_get_input_buffer_size;
	hw_device->open_output_stream = adev_open_output_stream;
	hw_device->close_output_stream = adev_close_output_stream;
	hw_device->open_input_stream = adev_open_input_stream;
	hw_device->close_input_stream = adev_close_input_stream;
	hw_device->dump = adev_dump;
	*device = &common;

	if (mUcm.loadConfiguration())
		return;

	mInitStatus = true;
}

Dev::~Dev() {
}

int Dev::open_output_stream(audio_io_handle_t handle,
                            audio_devices_t devices,
                            audio_output_flags_t flags,
                            struct audio_config *config,
                            struct audio_stream_out **stream_out) {
	AutoMutex lock(mLock);
	OutStream *out = new OutStream(*this, mUcm, handle, devices, flags, config);
	if (!out)
		return -ENOMEM;
	*stream_out = reinterpret_cast<audio_stream_out *>(out);
	mOutStreams.insert(out);
	return 0;
}

void Dev::close_output_stream(struct audio_stream_out *stream) {
	AutoMutex lock(mLock);
	OutStream *out = (OutStream *) stream;
	uh_assert_se(1 == mOutStreams.erase(out));
	delete out;
}

int Dev::set_parameters(const char *kvpairs) {
    LOGFUNC("%s(%p, %s)", this, kvpairs);
    mParameters.update(kvpairs);
    return 0;
}

char * Dev::get_parameters(const char *keys) const {
	return mParameters.toStr();
}

int Dev::init_check() const {
	if (mInitStatus)
		return 0;
	return -EINVAL;
}

int Dev::set_voice_volume(float volume) {
	return -ENOSYS;
}

int Dev::set_master_volume(float volume) {
	return -ENOSYS;
}

int Dev::set_mode(audio_mode_t mode) {
	AutoMutex lock(mLock);
	if (mMode != mode) {
		for (OutStreamSet_t::iterator i = mOutStreams.begin();
			 i != mOutStreams.begin(); i++)
			(*i)->modeUpdate(mode);
		for (InStreamSet_t::iterator i = mInStreams.begin();
			 i != mInStreams.begin(); i++)
			(*i)->modeUpdate(mode);
	}
	return 0;
}

int Dev::set_mic_mute(bool state) {
	return -ENOSYS;
}

int Dev::get_mic_mute(bool *state) const {
	return -ENOSYS;
}

size_t Dev::get_input_buffer_size(const audio_config *config) const {
	return 0;
}

int Dev::open_input_stream(audio_io_handle_t handle,
						   audio_devices_t devices,
						   struct audio_config *config,
						   struct audio_stream_in **stream_in) {
	if (InStream::check_parameters(config)) {
		LOGE("open_input_stream: Invalid input parameters (%dHz,%dch,0x08x)",
			 config->sample_rate, popcount(config->channel_mask),
			 config->format);
		return -EINVAL;
	}
	AutoMutex lock(mLock);
	InStream *in = new InStream(*this, mUcm, handle, devices, config);
	if (!in)
		return -ENOMEM;
	mInStreams.insert(in);
	*stream_in = reinterpret_cast<audio_stream_in *>(in);
	return 0;
}

void Dev::close_input_stream(struct audio_stream_in *stream) {
	AutoMutex lock(mLock);
	InStream *in = (InStream *) stream;
	uh_assert_se(1 == mInStreams.erase(in));
	delete in;
}

uint32_t Dev::get_supported_devices() const {
	return mUcm.getSupportedDeivices();
}

int Dev::dump(int fd) const {
	LOGE("Dump-method not implemented");
	return 0;
}

}; // namespace UcmHal
