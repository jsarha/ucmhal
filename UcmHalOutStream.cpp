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

namespace UcmHal {

uint32_t out_get_sample_rate(const struct audio_stream *stream) {
	return ((const OutStream *) stream)->get_sample_rate();
}

int out_set_sample_rate(struct audio_stream *stream, uint32_t rate) {
	return ((OutStream *) stream)->set_sample_rate(rate);
}

size_t out_get_buffer_size(const struct audio_stream *stream) {
	return ((const OutStream *) stream)->get_buffer_size();
}

uint32_t out_get_channels(const struct audio_stream *stream) {
	return ((const OutStream *) stream)->get_channels();
}

audio_format_t out_get_format(const struct audio_stream *stream) {
	return ((const OutStream *) stream)->get_format();
}

int out_set_format(struct audio_stream *stream, audio_format_t format) {
	return ((OutStream *) stream)->set_format(format);
}

int out_standby(struct audio_stream *stream) {
	return ((OutStream *) stream)->standby();
}

int out_dump(const struct audio_stream *stream, int fd) {
	return ((const OutStream *) stream)->dump(fd);
}

int out_set_parameters(struct audio_stream *stream, const char *kvpairs) {
	return ((OutStream *) stream)->set_parameters(kvpairs);
}

char * out_get_parameters(const struct audio_stream *stream,
                          const char *keys) {
	return ((const OutStream *) stream)->get_parameters(keys);
}

int out_add_audio_effect(const struct audio_stream *stream,
                         effect_handle_t effect) {
	return ((const OutStream *) stream)->add_audio_effect(effect);
}

int out_remove_audio_effect(const struct audio_stream *stream,
                            effect_handle_t effect) {
	return ((const OutStream *) stream)->remove_audio_effect(effect);
}

uint32_t out_get_latency(const struct audio_stream_out *stream) {
	return ((const OutStream *) stream)->get_latency();
}

int out_set_volume(struct audio_stream_out *stream, float left, float right) {
	return ((OutStream *) stream)->set_volume(left, right);
}

ssize_t out_write(struct audio_stream_out *stream, const void* buffer,
                  size_t bytes) {
	return ((OutStream *) stream)->write(buffer, bytes);
}

int out_get_render_position(const struct audio_stream_out *stream,
                            uint32_t *dsp_frames) {
	return ((const OutStream *) stream)->get_render_position(dsp_frames);
}

OutStream::OutStream(Dev &dev,
                     UseCaseMgr &ucm,
                     audio_io_handle_t handle,
                     audio_devices_t devices,
                     audio_output_flags_t flags,
                     struct audio_config *config) :
	mDev(dev), mUcm(ucm), mStandby(true), mDevices(devices), mFlags(flags) {
	// C-style cast would save couple of cycles
    audio_stream_out *stream = reinterpret_cast<audio_stream_out *>(this);
    memset(stream, 0, sizeof(stream));

    common.get_sample_rate = out_get_sample_rate;
    common.set_sample_rate = out_set_sample_rate;
    common.get_buffer_size = out_get_buffer_size;
    common.get_channels = out_get_channels;
    common.get_format = out_get_format;
    common.set_format = out_set_format;
    common.standby = out_standby;
    common.dump = out_dump;
    common.set_parameters = out_set_parameters;
    common.get_parameters = out_get_parameters;
    common.add_audio_effect = out_add_audio_effect;
    common.remove_audio_effect = out_remove_audio_effect;
    stream->get_latency = out_get_latency;
    stream->set_volume = out_set_volume;
    stream->write = out_write;
    stream->get_render_position = out_get_render_position;

    config->format = get_format();
    config->channel_mask = get_channels();
    config->sample_rate = get_sample_rate();
}

OutStream::~OutStream() {
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
	//TODO
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
	//TODO
	return 0;
}

int OutStream::get_render_position(uint32_t *dsp_frames) const {
	//TODO
	return 0;
}

}; // namespace UcmHal
