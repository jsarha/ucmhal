#include <stdint.h>
#include <stdio.h>

#include <hardware/hardware.h>
#include <system/audio.h>
//#include <hardware/audio.h>

extern int ucmhal_adev_open(const hw_module_t* module, const char* name,
							hw_device_t** device);

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

    return 0;
}
