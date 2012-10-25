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

/*
 * This file is a shim for use-case.c, and implements and ALSA-like
 * API.  It's intended to replace the API with the shim's
 * implementation.
 */

#ifndef __ALSA_SHIM_H__
#define __ALSA_SHIM_H__

#include <stddef.h> /* size_t */
#include <alloca.h>
/*#include <linux/time.h>*/
#include <time.h>

#include <linux/ioctl.h>
#define __force
#define __bitwise
#define __user
#include <sound/asound.h>
#include "alsa-control.h"

#ifdef __cplusplus
extern "C" {
#endif

       /** Invalid type */
#define SND_CTL_ELEM_TYPE_NONE       SNDRV_CTL_ELEM_TYPE_NONE;
#define SND_CTL_ELEM_TYPE_BOOLEAN    SNDRV_CTL_ELEM_TYPE_BOOLEAN
#define SND_CTL_ELEM_TYPE_INTEGER    SNDRV_CTL_ELEM_TYPE_INTEGER
#define SND_CTL_ELEM_TYPE_ENUMERATED SNDRV_CTL_ELEM_TYPE_ENUMERATED
#define SND_CTL_ELEM_TYPE_BYTES      SNDRV_CTL_ELEM_TYPE_BYTES
#define SND_CTL_ELEM_TYPE_IEC958     SNDRV_CTL_ELEM_TYPE_IEC958
#define SND_CTL_ELEM_TYPE_INTEGER64  SNDRV_CTL_ELEM_TYPE_INTEGER64
#define SND_CTL_ELEM_TYPE_LAST       SNDRV_CTL_ELEM_TYPE_LAST



/** File descriptor to control device */
typedef int snd_ctl_t;

typedef unsigned snd_ctl_elem_t;

/** Read only (flag for open mode) \hideinitializer */
#define SND_CTL_READONLY		0x0004

#define __snd_alloca(ptr,type) do { *ptr = (type##_t *) alloca(type##_sizeof()); memset(*ptr, 0, type##_sizeof()); } while (0)
#define snd_ctl_elem_id_alloca(ptr) __snd_alloca(ptr, snd_ctl_elem_id)
#define snd_ctl_elem_info_alloca(ptr) __snd_alloca(ptr, snd_ctl_elem_info)
#define snd_ctl_elem_value_alloca(ptr) __snd_alloca(ptr, snd_ctl_elem_value)
#define snd_ctl_card_info_alloca(ptr) __snd_alloca(ptr, snd_ctl_card_info)
#define snd_ctl_elem_list_alloca(ptr) __snd_alloca(ptr, snd_ctl_elem_list)

int snd_card_get_index(const char *name);
const char *snd_strerror(int errnum);

int snd_ctl_open(snd_ctl_t **ctl, const char *name, int mode);
int snd_ctl_close(snd_ctl_t *ctl);

int snd_ctl_card_info(snd_ctl_t *ctl, snd_ctl_card_info_t *info);
void snd_ctl_card_info_free(snd_ctl_card_info_t *obj);
int snd_ctl_card_info_malloc(snd_ctl_card_info_t **ptr);
size_t snd_ctl_card_info_sizeof(void);

void snd_ctl_elem_id_free(snd_ctl_elem_id_t *obj);
const char *snd_ctl_elem_id_get_name(const snd_ctl_elem_id_t *obj);
void snd_ctl_elem_id_set_numid(snd_ctl_elem_id_t *obj, unsigned int val);
int snd_ctl_elem_id_malloc(snd_ctl_elem_id_t **ptr);
size_t snd_ctl_elem_id_sizeof(void);
unsigned int snd_ctl_elem_id_get_numid(const snd_ctl_elem_id_t *obj);

int snd_ctl_elem_read(snd_ctl_t *ctl, snd_ctl_elem_value_t *value);
int snd_ctl_elem_write(snd_ctl_t *ctl, snd_ctl_elem_value_t *value);

int snd_ctl_elem_info(snd_ctl_t *ctl, snd_ctl_elem_info_t *info);
unsigned int snd_ctl_elem_info_get_count(const snd_ctl_elem_info_t *obj);
snd_ctl_elem_type_t snd_ctl_elem_info_get_type(const snd_ctl_elem_info_t *obj);
void snd_ctl_elem_info_set_id(snd_ctl_elem_info_t *obj, const snd_ctl_elem_id_t *ptr);
size_t snd_ctl_elem_info_sizeof(void);

int snd_ctl_elem_list(snd_ctl_t *ctl, snd_ctl_elem_list_t *list);
size_t snd_ctl_elem_list_sizeof(void);
int snd_ctl_elem_list_alloc_space(snd_ctl_elem_list_t *obj, unsigned int entries);
void snd_ctl_elem_list_free_space(snd_ctl_elem_list_t *obj);
void snd_ctl_elem_list_free(snd_ctl_elem_list_t *obj);
unsigned int snd_ctl_elem_list_get_count(const snd_ctl_elem_list_t *obj);
int snd_ctl_elem_list_malloc(snd_ctl_elem_list_t **ptr);
void snd_ctl_elem_list_set_offset(snd_ctl_elem_list_t *obj, unsigned int val);
void snd_ctl_elem_list_get_id(const snd_ctl_elem_list_t *obj, unsigned int idx, snd_ctl_elem_id_t *ptr);

size_t snd_ctl_elem_value_sizeof(void);
void snd_ctl_elem_value_set_id(snd_ctl_elem_value_t *obj, const snd_ctl_elem_id_t *ptr);
int snd_ctl_elem_value_get_boolean(const snd_ctl_elem_value_t *obj, unsigned int idx);
void snd_ctl_elem_value_set_boolean(snd_ctl_elem_value_t *obj, unsigned int idx, long val);
unsigned char snd_ctl_elem_value_get_byte(const snd_ctl_elem_value_t *obj, unsigned int idx);
void snd_ctl_elem_value_set_byte(snd_ctl_elem_value_t *obj, unsigned int idx, unsigned char val);
unsigned int snd_ctl_elem_value_get_enumerated(const snd_ctl_elem_value_t *obj, unsigned int idx);
void snd_ctl_elem_value_set_enumerated(snd_ctl_elem_value_t *obj, unsigned int idx, unsigned int val);
long snd_ctl_elem_value_get_integer(const snd_ctl_elem_value_t *obj, unsigned int idx);
void snd_ctl_elem_value_set_integer(snd_ctl_elem_value_t *obj, unsigned int idx, long val);
long long snd_ctl_elem_value_get_integer64(const snd_ctl_elem_value_t *obj, unsigned int idx);
void snd_ctl_elem_value_set_integer64(snd_ctl_elem_value_t *obj, unsigned int idx, long long val);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* __ALSA_SHIM_H__ */
