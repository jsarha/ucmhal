/*
 * Copyright (C) 2012 Texas Instruments
 *
 * Liberal inspiration drawn from the AOSP code for Toro.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

extern int ucmhal_adev_open(const hw_module_t* module, const char* name,
							hw_device_t** device);

static struct hw_module_methods_t hal_module_methods = {
    .open = ucmhal_adev_open,
};

struct audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "UcmHal",
        .author = "Jyri Sarha <jsarha@ti.com>",
        .methods = &hal_module_methods,
    },
};
