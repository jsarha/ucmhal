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

	// Fill flag macro map
	MACRO_MAP(mFlagMacroMap, AUDIO_OUTPUT_FLAG_NONE);
	MACRO_MAP(mFlagMacroMap, AUDIO_OUTPUT_FLAG_DIRECT);
	MACRO_MAP(mFlagMacroMap, AUDIO_OUTPUT_FLAG_PRIMARY);
	MACRO_MAP(mFlagMacroMap, AUDIO_OUTPUT_FLAG_FAST);
	MACRO_MAP(mFlagMacroMap, AUDIO_OUTPUT_FLAG_DEEP_BUFFER);
}

MacroMap::~MacroMap() {
}

int MacroMap::toInt(MacroMap_t &map, const char *name, int &out) {
	MacroMap_t::iterator i = map.find(name);
	if (i != map.end()) {
		out = i->second;
		return 0;
	}
	return -1;
}

int MacroMap::mode(const char *name)  {
	int out = 0;
	if (toInt(mModeMacroMap, name, out))
		ALOGE("%s not found from mode macromap", name);
	return out;
}

int MacroMap::device(const char *name) {
	int out = 0;
	if (toInt(mDeviceMacroMap, name, out))
		ALOGE("%s not found from device macromap", name);
	return out;
}

int MacroMap::flag(const char *name)  {
	int out = 0;
	if (toInt(mFlagMacroMap, name, out))
		ALOGE("%s not found from flags macromap", name);
	return out;
}

int MacroMap::toStr(MacroMap_t &map, int val, string &out) {
	MacroMap_t::iterator i = map.begin();
	bool begin = true;
	int count;
	for (;i != map.end(); i++) {
		if (i->second & val) {
			if (begin)
				begin = false;
			else
				out.append("|");
			out.append(i->first);
			count++;
		}
	}
	return count;
}

const char *MacroMap::modeStr(int mode, string &str) {
	toStr(mModeMacroMap, mode, str);
	return str.c_str();
}

const char *MacroMap::deviceStr(int mode, string &str) {
	toStr(mDeviceMacroMap, mode, str);
	return str.c_str();
}

const char *MacroMap::flagStr(int mode, string &str) {
	toStr(mFlagMacroMap, mode, str);
	return str.c_str();
}

}; // namespace UcmHal
