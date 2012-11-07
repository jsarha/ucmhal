#include <stdint.h>
#include <stdio.h>

#include <hardware/hardware.h>
#include <system/audio.h>

/* This is a hack to avoid including audio_effet.h */
#define ANDROID_AUDIO_EFFECT_H
typedef void * effect_handle_t;
#include <hardware/audio.h>

extern int ucmhal_adev_open(const hw_module_t* module, const char* name,
							hw_device_t** device);

void play_sine(struct audio_stream_out *out, size_t frames);

int main(int argc, char *argv[]) {
	hw_module_t dummy_module = {};
	const char *dev_name = "audio_hw_if";
	hw_device_t *dev;
	int ret;

	if (argc != 1) {
		fprintf(stderr, "usage: %s \n", argv[0]);
		return -1;
	}

	if ((ret = ucmhal_adev_open(&dummy_module, dev_name, &dev))) {
		fprintf(stderr, "ucmhal_adev_open returned %d \n", ret);
		return -1;
	}

	{
		audio_hw_device_t *adev = (audio_hw_device_t *) dev;
		audio_io_handle_t handle = 0;
		audio_output_flags_t flags = 0;
		struct audio_config config = {
			.sample_rate = 44100,
			.channel_mask = AUDIO_DEVICE_OUT_ALL,
			.format = AUDIO_FORMAT_PCM_SUB_16_BIT
		};
		struct audio_stream_out *stream_out_hf;
		struct audio_stream_out *stream_out_hs;
		char buf[1024];
		fgets(buf, sizeof(buf), stdin);

		if ((ret = adev->open_output_stream(
				 adev, handle, AUDIO_DEVICE_OUT_SPEAKER, flags, &config, &stream_out_hf))) {
			fprintf(stderr, "open_input_stream returned %d \n", ret);
			return -1;
		}

		play_sine(stream_out_hf, 44100);
		fgets(buf, sizeof(buf), stdin);

		stream_out_hf->common.standby(&stream_out_hf->common);

		if ((ret = adev->open_output_stream(
				 adev, handle, AUDIO_DEVICE_OUT_WIRED_HEADSET, flags, &config, &stream_out_hs))) {
			fprintf(stderr, "open_input_stream returned %d \n", ret);
			return -1;
		}

		fgets(buf, sizeof(buf), stdin);
		play_sine(stream_out_hs, 44100);
		fgets(buf, sizeof(buf), stdin);

		adev->close_output_stream(adev, stream_out_hs);

		play_sine(stream_out_hf, 44100);
		// Route to headset
		stream_out_hf->common.set_parameters(
			&stream_out_hf->common, AUDIO_PARAMETER_STREAM_ROUTING "=4;");
		play_sine(stream_out_hf, 44100);
		// Route back to speaker
		stream_out_hf->common.set_parameters(
			&stream_out_hf->common, AUDIO_PARAMETER_STREAM_ROUTING "=2;");
		play_sine(stream_out_hf, 44100);
		fgets(buf, sizeof(buf), stdin);
		adev->set_parameters(adev, AUDIO_PARAMETER_KEY_SCREEN_STATE "=on;");
		fgets(buf, sizeof(buf), stdin);
		adev->set_parameters(adev, AUDIO_PARAMETER_KEY_SCREEN_STATE "=on;");
		fgets(buf, sizeof(buf), stdin);
		adev->set_parameters(adev, AUDIO_PARAMETER_KEY_SCREEN_STATE "=off;");
		fgets(buf, sizeof(buf), stdin);

		adev->close_output_stream(adev, stream_out_hf);

		dev->close(dev);
	}

    return 0;
}

#include <math.h>

int16_t sine_buf[44];
#define LEN(x) (sizeof(x)/sizeof(x[0]))
void init_sine_pcm() {
	int i;
	for (i = 0; i < LEN(sine_buf); i++)
		sine_buf[i] = (int16_t)(sinf((float)(M_PI*2*i)/44)*20000);
}

void sine_pcm(int16_t *buf, size_t len, int *phase) {
	int i;
	for (i = 0; i < len; i++) {
		buf[i] = sine_buf[*phase = (*phase + 1) % LEN(sine_buf)];
		buf[++i] = sine_buf[*phase];
	}
}

void play_sine(struct audio_stream_out *out, size_t frames) {
	int i;
	size_t bufsize = out->common.get_buffer_size(&out->common);
	int16_t buf[bufsize/sizeof(int16_t)];
	int phase = 0;
	init_sine_pcm();

	for (i = 0; i < frames;) {
		sine_pcm(buf, LEN(buf), &phase);
		i += LEN(buf);
		out->write(out, buf, sizeof(buf));
	}
}
