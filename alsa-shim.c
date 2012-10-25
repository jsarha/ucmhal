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

#include "alsa-shim.h"

#include <errno.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <stdlib.h>

#include "alsa-control.h"

int snd_card_get_index(const char *name)
{
	return ah_card_find_by_name(name);
}

const char *snd_strerror(int errnum)
{
	return strerror(errnum);
}

int snd_ctl_open(snd_ctl_t **ctl, const char *name, int mode)
{
	int card = ah_card_find_by_name(name);
	int fd;
	if (card < 0)
		return -ENODEV;
	fd = ah_control_open(card, mode);
	if (fd < 0)
		return fd;
	*ctl = calloc(1, 1);
	**ctl = fd;
	return 0;
}

int snd_ctl_close(snd_ctl_t *ctl)
{
	return ah_control_close(*ctl);
}

int snd_ctl_card_info(snd_ctl_t *ctl, snd_ctl_card_info_t *info)
{
	return ah_card_get_info(*ctl, info);
}

void snd_ctl_card_info_free(snd_ctl_card_info_t *obj)
{
	free(obj);
}

int snd_ctl_card_info_malloc(snd_ctl_card_info_t **ptr)
{
	*ptr = malloc(sizeof(snd_ctl_card_info_t));
	if (ptr)
		return 0;
	return ENOMEM;
}

size_t snd_ctl_card_info_sizeof(void)
{
	return sizeof(snd_ctl_card_info_t);
}

int snd_ctl_elem_id_malloc(snd_ctl_elem_id_t **ptr)
{
	snd_ctl_elem_id_t *p = calloc(1, sizeof(snd_ctl_elem_id_t));
	if (!p)
		return -ENOMEM;
	*ptr = p;
	return 0;
}

void snd_ctl_elem_id_free(snd_ctl_elem_id_t *obj)
{
	free(obj);
}

const char *snd_ctl_elem_id_get_name(const snd_ctl_elem_id_t *obj)
{
	return (char*)obj->name;
}

unsigned int snd_ctl_elem_id_get_numid(const snd_ctl_elem_id_t *obj)
{
	return obj->numid;
}

void snd_ctl_elem_id_set_numid(snd_ctl_elem_id_t *obj, unsigned int val)
{
	obj->numid = val;
}

size_t snd_ctl_elem_id_sizeof(void)
{
	return sizeof(snd_ctl_elem_id_t);
}

int snd_ctl_elem_read(snd_ctl_t *ctl, snd_ctl_elem_value_t *value)
{
	int e, rv = 0;
	e = ioctl(*ctl, SNDRV_CTL_IOCTL_ELEM_READ, value);
	if (e)
		rv = errno;
	return rv;
}

int snd_ctl_elem_write(snd_ctl_t *ctl, snd_ctl_elem_value_t *value)
{
	int e, rv = 0;
	e = ioctl(*ctl, SNDRV_CTL_IOCTL_ELEM_WRITE, value);
	if (e)
		rv = errno;
	return rv;
}

int snd_ctl_elem_info(snd_ctl_t *ctl, snd_ctl_elem_info_t *info)
{
	int e, rv = 0;
	e = ioctl(*ctl, SNDRV_CTL_IOCTL_ELEM_INFO, info);
	if (e)
		rv = errno;
	return rv;
}

unsigned int snd_ctl_elem_info_get_count(const snd_ctl_elem_info_t *obj)
{
	return obj->count;
}

snd_ctl_elem_type_t snd_ctl_elem_info_get_type(const snd_ctl_elem_info_t *obj)
{
	return obj->type;
}

void snd_ctl_elem_info_set_id(snd_ctl_elem_info_t *obj, const snd_ctl_elem_id_t *ptr)
{
	memcpy(&obj->id, ptr, sizeof(snd_ctl_elem_id_t));
}

size_t snd_ctl_elem_info_sizeof(void)
{
	return sizeof(snd_ctl_elem_info_t);
}

int snd_ctl_elem_list(snd_ctl_t *ctl, snd_ctl_elem_list_t *list)
{
	int e, rv = 0;
	e = ioctl(*ctl, SNDRV_CTL_IOCTL_ELEM_LIST, list);
	if (e) {
		rv = -errno;
		goto end;
	}
	if (list->count < 0) {
		rv = 0;
		goto end;
	}
	if (snd_ctl_elem_list_alloc_space(list, list->count)) {
		rv = -ENOMEM;
		goto end;
	}
	list->space = list->count;
	e = ioctl(*ctl, SNDRV_CTL_IOCTL_ELEM_LIST, list);
	if (e) {
		rv = -errno;
		goto end;
	}
end:
	return rv;
}

size_t snd_ctl_elem_list_sizeof(void)
{
	return sizeof(snd_ctl_elem_list_t);
}

int snd_ctl_elem_list_alloc_space(snd_ctl_elem_list_t *obj, unsigned int entries)
{
	free(obj->pids);
	obj->pids = calloc(entries, sizeof(snd_ctl_elem_id_t));
	if (!obj->pids)
		return ENOMEM;
	return 0;
}

void snd_ctl_elem_list_free_space(snd_ctl_elem_list_t *obj)
{
	free(obj->pids);
}

void snd_ctl_elem_list_free(snd_ctl_elem_list_t *obj)
{
	free(obj);
}

unsigned int snd_ctl_elem_list_get_count(const snd_ctl_elem_list_t *obj)
{
	return obj->count;
}

int snd_ctl_elem_list_malloc(snd_ctl_elem_list_t **ptr)
{
	snd_ctl_elem_list_t *p = calloc(1, sizeof(snd_ctl_elem_list_t));
	if (!p)
		return ENOMEM;
	*ptr = p;
	return 0;
}

void snd_ctl_elem_list_set_offset(snd_ctl_elem_list_t *obj, unsigned int val)
{
	obj->offset = val;
}

void snd_ctl_elem_list_get_id(const snd_ctl_elem_list_t *obj, unsigned int idx, snd_ctl_elem_id_t *ptr)
{
	*ptr = obj->pids[idx];
}

size_t snd_ctl_elem_value_sizeof(void)
{
	return sizeof(snd_ctl_elem_value_t);
}

void snd_ctl_elem_value_set_id(snd_ctl_elem_value_t *obj, const snd_ctl_elem_id_t *ptr)
{
	memcpy(&obj->id, ptr, sizeof(snd_ctl_elem_id_t));
}

int snd_ctl_elem_value_get_boolean(const snd_ctl_elem_value_t *obj, unsigned int idx)
{
	return obj->value.integer.value[idx];
}

void snd_ctl_elem_value_set_boolean(snd_ctl_elem_value_t *obj, unsigned int idx, long val)
{
	obj->value.integer.value[idx] = val;
}

unsigned char snd_ctl_elem_value_get_byte(const snd_ctl_elem_value_t *obj, unsigned int idx)
{
	return obj->value.bytes.data[idx];
}

void snd_ctl_elem_value_set_byte(snd_ctl_elem_value_t *obj, unsigned int idx, unsigned char val)
{
	obj->value.bytes.data[idx] = val;
}

unsigned int snd_ctl_elem_value_get_enumerated(const snd_ctl_elem_value_t *obj, unsigned int idx)
{
	return obj->value.enumerated.item[idx];
}

void snd_ctl_elem_value_set_enumerated(snd_ctl_elem_value_t *obj, unsigned int idx, unsigned int val)
{
	obj->value.enumerated.item[idx] = val;
}

long snd_ctl_elem_value_get_integer(const snd_ctl_elem_value_t *obj, unsigned int idx)
{
	return obj->value.integer.value[idx];
}

void snd_ctl_elem_value_set_integer(snd_ctl_elem_value_t *obj, unsigned int idx, long val)
{
	obj->value.integer.value[idx] = val;
}

long long snd_ctl_elem_value_get_integer64(const snd_ctl_elem_value_t *obj, unsigned int idx)
{
	return obj->value.integer64.value[idx];
}

void snd_ctl_elem_value_set_integer64(snd_ctl_elem_value_t *obj, unsigned int idx, long long val)
{
	obj->value.integer64.value[idx] = val;
}
