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

	UcmHal::Dev *dev = new UcmHal::Dev(module);

	if (!dev)
		return -ENOMEM;

	int ret = dev->init_check();
	if (ret)
		delete dev;
	else
		*device = &dev->audio_hw_device()->common;

	return ret;
}

namespace UcmHal {

static int adev_close(hw_device_t *dev) {
	ALOGE("Closing audio device");
	delete ((ucmhal_dev *)dev)->me;
	return 0;
}

static int adev_open_output_stream(audio_hw_device *dev,
                                   audio_io_handle_t handle,
                                   audio_devices_t devices,
                                   audio_output_flags_t flags,
                                   struct audio_config *config,
                                   struct audio_stream_out **stream_out) {
	return ((ucmhal_dev *)dev)->me->open_output_stream(
		handle, devices, flags, config, stream_out);
}

static void adev_close_output_stream(audio_hw_device *dev,
                                     struct audio_stream_out *stream) {
	((ucmhal_dev *)dev)->me->close_output_stream(stream);
}

static int adev_set_parameters(audio_hw_device *dev, const char *kvpairs) {
	return ((ucmhal_dev *)dev)->me->set_parameters(kvpairs);
}

static char *adev_get_parameters(const audio_hw_device *dev, const char *keys) {
	return ((const Dev *)((ucmhal_dev *)dev)->me)->get_parameters(keys);
}

static int adev_init_check(const audio_hw_device *dev) {
	return ((const Dev *)((ucmhal_dev *)dev)->me)->init_check();
}

static int adev_set_voice_volume(audio_hw_device *dev, float volume) {
	return ((ucmhal_dev *)dev)->me->set_voice_volume(volume);
}

static int adev_set_master_volume(audio_hw_device *dev, float volume) {
	return ((ucmhal_dev *)dev)->me->set_master_volume(volume);
}

static int adev_set_mode(audio_hw_device *dev, audio_mode_t mode) {
	return ((ucmhal_dev *)dev)->me->set_mode(mode);
}

static int adev_set_mic_mute(audio_hw_device *dev, bool state) {
	return ((ucmhal_dev *)dev)->me->set_mic_mute(state);
}

static int adev_get_mic_mute(const audio_hw_device *dev, bool *state) {
	return ((const Dev *)((ucmhal_dev *)dev)->me)->get_mic_mute(state);
}

static size_t adev_get_input_buffer_size(const audio_hw_device *dev,
                                         const audio_config *config) {
	return ((const Dev *)((ucmhal_dev *)dev)->me)->get_input_buffer_size(config);
}

static int adev_open_input_stream(audio_hw_device *dev,
                                  audio_io_handle_t handle,
                                  audio_devices_t devices,
                                  struct audio_config *config,
                                  struct audio_stream_in **stream_in) {
	return ((ucmhal_dev *)dev)->me->open_input_stream(
		handle, devices, config, stream_in);
}

static void adev_close_input_stream(audio_hw_device *dev, struct audio_stream_in *stream) {
	((ucmhal_dev *)dev)->me->close_input_stream(stream);
}

static int adev_dump(const audio_hw_device *dev, int fd) {
	return ((ucmhal_dev *)dev)->me->dump(fd);
}

static uint32_t adev_get_supported_devices(const audio_hw_device *dev) {
	return ((const Dev *)((ucmhal_dev *)dev)->me)->get_supported_devices();
}

const char *Dev::supportedParameters[] = {
	AUDIO_PARAMETER_KEY_TTY_MODE,
	AUDIO_PARAMETER_KEY_BT_NREC,
	AUDIO_PARAMETER_KEY_SCREEN_STATE,
	NULL
};

Dev::Dev(const hw_module_t* module) :
	mUcm(mMM),
	mInitStatus(false),
	mMode(AUDIO_MODE_NORMAL),
	mParameters(supportedParameters) {
	memset(&m_dev, 0, sizeof(m_dev));

	m_dev.android_dev.common.tag = HARDWARE_DEVICE_TAG;
	m_dev.android_dev.common.version = AUDIO_DEVICE_API_VERSION_CURRENT;
	m_dev.android_dev.common.module = (struct hw_module_t *) module;
	m_dev.android_dev.common.close = adev_close;

	m_dev.android_dev.get_supported_devices = adev_get_supported_devices;
	m_dev.android_dev.init_check = adev_init_check;
	m_dev.android_dev.set_voice_volume = adev_set_voice_volume;
	m_dev.android_dev.set_master_volume = adev_set_master_volume;
	m_dev.android_dev.set_mode = adev_set_mode;
	m_dev.android_dev.set_mic_mute = adev_set_mic_mute;
	m_dev.android_dev.get_mic_mute = adev_get_mic_mute;
	m_dev.android_dev.set_parameters = adev_set_parameters;
	m_dev.android_dev.get_parameters = adev_get_parameters;
	m_dev.android_dev.get_input_buffer_size = adev_get_input_buffer_size;
	m_dev.android_dev.open_output_stream = adev_open_output_stream;
	m_dev.android_dev.close_output_stream = adev_close_output_stream;
	m_dev.android_dev.open_input_stream = adev_open_input_stream;
	m_dev.android_dev.close_input_stream = adev_close_input_stream;
	m_dev.android_dev.dump = adev_dump;
	m_dev.me = this;

	if (mUcm.loadConfiguration())
		return;

	mParameters.setHook(this, &Dev::screenStateHook,
	                    AUDIO_PARAMETER_KEY_SCREEN_STATE);

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
	mOutStreams.insert(out);
	*stream_out = out->audio_stream_out();
	return 0;
}

void Dev::close_output_stream(struct audio_stream_out *stream) {
	AutoMutex lock(mLock);
	OutStream *out = ((ucmhal_out *) stream)->me;
	uh_assert_se(1 == mOutStreams.erase(out));
	delete out;
}

int Dev::set_parameters(const char *kvpairs) {
	LOGFUNC("%s(%p, %s)", __func__, this, kvpairs);
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
		ALOGE("open_input_stream: Invalid input parameters (%dHz,%dch,0x%08x)",
		      config->sample_rate, popcount(config->channel_mask),
		      config->format);
		return -EINVAL;
	}
	AutoMutex lock(mLock);
	InStream *in = new InStream(*this, mUcm, handle, devices, config);
	if (!in)
		return -ENOMEM;
	mInStreams.insert(in);
	*stream_in = in->audio_stream_in();
	return 0;
}

void Dev::close_input_stream(struct audio_stream_in *stream) {
	AutoMutex lock(mLock);
	InStream *in = ((ucmhal_in *) stream)->me;
	uh_assert_se(1 == mInStreams.erase(in));
	delete in;
}

uint32_t Dev::get_supported_devices() const {
	return mUcm.getSupportedDeivices();
}

int Dev::dump(int fd) const {
	ALOGE("Dump-method not implemented");
	return 0;
}

void Dev::screenStateHook() {
	string value;
	ALOGE("New \"%s\" value \"%s\"", AUDIO_PARAMETER_KEY_SCREEN_STATE,
	      mParameters.get(AUDIO_PARAMETER_KEY_SCREEN_STATE, value).c_str());
}

}; // namespace UcmHal
