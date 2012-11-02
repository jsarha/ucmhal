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

#define LOG_TAG "ucmhal"
#define LOG_NDEBUG 0

#include <string.h>

#include <system/audio.h>

#include "UcmHalTypes.h"

#include <map>
#include <utility>

#include "UcmHalMacroMap.h"

namespace UcmHal {

#define MACRO_MAP(map, x) (map) [ #x ] = (x)

MacroMap::MacroMap() {
	// Fill device macro map
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_EARPIECE);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_SPEAKER);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_WIRED_HEADSET);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_WIRED_HEADPHONE);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_BLUETOOTH_SCO);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_BLUETOOTH_A2DP);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_AUX_DIGITAL);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_ANLG_DOCK_HEADSET);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_DGTL_DOCK_HEADSET);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_DEFAULT);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_ALL);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_ALL_A2DP);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_ALL_SCO);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_COMMUNICATION);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_AMBIENT);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_BUILTIN_MIC);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_BLUETOOTH_SCO_HEADSET);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_WIRED_HEADSET);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_AUX_DIGITAL);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_VOICE_CALL);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_BACK_MIC);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_USB_HEADSET);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_DEFAULT);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_ALL);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_ALL_SCO);
#ifdef OMAP_ENHANCEMENT
	MACRO_MAP(mDeviceMacroMap, AUDIO_SOURCE_FM_RADIO_RX);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_OUT_FM_RADIO_TX);
	MACRO_MAP(mDeviceMacroMap, AUDIO_DEVICE_IN_FM_RADIO_RX);
#endif

	// Fill mode macro map
	MACRO_MAP(mModeMacroMap, AUDIO_MODE_INVALID);
	MACRO_MAP(mModeMacroMap, AUDIO_MODE_CURRENT);
	MACRO_MAP(mModeMacroMap, AUDIO_MODE_NORMAL);
	MACRO_MAP(mModeMacroMap, AUDIO_MODE_RINGTONE);
	MACRO_MAP(mModeMacroMap, AUDIO_MODE_IN_CALL);
	MACRO_MAP(mModeMacroMap, AUDIO_MODE_IN_COMMUNICATION);
	MACRO_MAP(mModeMacroMap, AUDIO_MODE_CNT);
	MACRO_MAP(mModeMacroMap, AUDIO_MODE_MAX);
}

MacroMap::~MacroMap() {
}

int MacroMap::device(const char *name) {
	MacroMap_t::iterator i = mDeviceMacroMap.find(name);
	if (i != mDeviceMacroMap.end())
		return i->second;
	ALOGE("%s not found from macromap", name);
	return 0;
}

int MacroMap::mode(const char *name)  {
	MacroMap_t::iterator i = mModeMacroMap.find(name);
	if (i != mModeMacroMap.end())
		return i->second;
	ALOGE("%s not found from macromap", name);
	return 0;
}

}; // namespace UcmHal
