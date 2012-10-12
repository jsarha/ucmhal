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
#include <iostream>
#include <map>
#include <utility>

#include <UcmHalUseCaseMgr.h>

namespace UcmHal {
	
UseCaseMgr::UseCaseMgr(MacroMap &mm) : 
	mucm(NULL), mMM(mm), mAllDevices(0) {
}
	
UseCaseMgr::~UseCaseMgr() {
	if (mucm)
		snd_use_case_mgr_close(mucm);
}

int UseCaseMgr::loadConfiguration() {
	static const char *map_file_name = ALSA_USE_CASE_DIR "/android_map.xml";

	loadUseCaseMap(map_file_name);

	mucm = snd_use_case_mgr_open(mUcmConfName);

	// TODO validate configuration

	return 0;
}

int UseCaseMgr::setUseCase(int mode, int devices) {
	mapping request;
	request.mode = mode;
	request.devices = devices;

	LOGD("mode 0x%08x dev 0x%08x", mode, devices);

	std::pair<ucmset_t::iterator, ucmset_t::iterator> range =
		mUCMap.equal_range(request);

	for (ucmset_t::iterator i=range.first; i != range.second; ++i) {
		if ((i->devices & i->devices_mask) == (devices & i->devices_mask)) {
			LOGD("Found: 0x%08x 0x%08x 0x%08x -> verb %s dev %s mod %s",
			     i->ucm_verb.c_str(), i->ucm_device.c_str(),
			     i->ucm_modifier.c_str());

			snd_use_case_set_verb(mucm, i->ucm_verb.c_str());
			snd_use_case_enable_device(mucm, i->ucm_device.c_str());
			snd_use_case_enable_modifier(mucm, i->ucm_modifier.c_str());
			return 0;
		}
	}
	return -1;
}

int UseCaseMgr::loadUseCaseMap(const char *file) {
	TiXmlDocument xDoc;
	TiXmlElement *root, *e;

	LOGE("%s(file=%s)", __func__, file);

	if (! xDoc.LoadFile(file) ) {
		LOGE("Could not open and parse file %s", file);
		return 1;
	}

	root = xDoc.RootElement();

	if (!root) {
		LOGE("File appears to be empty");
		return 1;
	}

	if (0 != strcmp(root->Value(), "android_use_case_map")) {
		LOGE("Root element is not 'android_use_case_map'");
		return 1;
	}
           
	e = root->FirstChildElement("use_case_table");

	if (!e) {
		LOGE("Expected element <use_case_table>");
		return 1;
	}

	for (e = e->FirstChildElement("row"); e ;e = e->NextSiblingElement("row")) {
		TiXmlElement *in, *out, *t, *flag;
		const char* text;
		mapping entry = { 0, 0, 0 };
		
		// Get mode
		in = e->FirstChildElement("input");
		entry.mode = mMM.mode(in->FirstChildElement("audio_mode")->GetText());

		// Get devices
		t = in->FirstChildElement("devices");
		entry.devices = 0;
		for (flag = t->FirstChildElement("flag"); flag; 
		     flag = flag->NextSiblingElement("flag")) {
			entry.devices |= mMM.device(flag->GetText());
		}
		mAllDevices |= entry.devices;

		// Get device mask
		t = in->FirstChildElement("devices_mask");
		entry.devices_mask = 0;
		for (flag = t->FirstChildElement("flag"); flag;
		     flag = flag->NextSiblingElement("flag")) {
			entry.devices_mask |= mMM.device(flag->GetText());
		}

		/* Outputs */
		out = e->FirstChildElement("output");
		text = out->FirstChildElement("verb")->GetText();
		if (text) {
			entry.ucm_verb = text;
		}

		text = out->FirstChildElement("device")->GetText();
		if (text) {
			entry.ucm_device = text; 
		}

		text = out->FirstChildElement("modifier")->GetText();
		if (text) {
			entry.ucm_modifier = text;
		}

		LOGV("adding {%d, %d, %d} => {%s, %s, %s} to map",
		     entry.mode, entry.devices, entry.devices_mask,
		     entry.ucm_verb.c_str(), entry.ucm_device.c_str(), 
		     entry.ucm_modifier.c_str());

		mUCMap.insert(entry);
	}
	return 0;
}

}; // namespace UcmHal 

