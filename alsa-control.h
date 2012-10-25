/*
 * Copyright (C) 2011 Texas Instruments
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

#ifndef __ALSA_CONTROL_H__
#define __ALSA_CONTROL_H__

#include "alsa-global.h"

#include <string.h>
#include <time.h>
/*#include <linux/time.h>*/

#include <linux/ioctl.h>
#define __force
#define __bitwise
#define __user
#include <sound/asound.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Typedefs for kernel structs.  See <sound/asound.h>
 */
typedef struct snd_ctl_card_info snd_ctl_card_info_t;
typedef struct snd_ctl_elem_id snd_ctl_elem_id_t;
typedef struct snd_ctl_elem_info snd_ctl_elem_info_t;
typedef struct snd_ctl_elem_list snd_ctl_elem_list_t;
typedef struct snd_ctl_elem_value snd_ctl_elem_value_t;

int ah_card_max_count();
int ah_card_count();
int ah_card_get_name(int card, char *str, size_t strlen);
int ah_card_find_by_name(const char* name);

int ah_control_open(int card, int mode);
int ah_control_close(int fd);
int ah_card_get_info(int fd, snd_ctl_card_info_t *info);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __ALSA_CONTROL_H__ */
