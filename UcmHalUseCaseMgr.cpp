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

#include "UcmHalUseCaseMgr.h"
#include "UcmHalMacro.h"

#include "alsa-control.h"

namespace UcmHal {

const string &UseCaseMapEntry::dump() {
	if (!mDump.empty())
		return mDump;

	char buf[1024];
	snprintf(buf, sizeof(buf),
			 "mode %d devices 0x%08x mask 0x%08x -> verb '%s' dev '%s' mod '%s'",
			 mMode, mDevices, mDevicesMask, mUcmVerb.c_str(),
			 mUcmDevice.c_str(), mUcmModifier.c_str());

	mDump = buf;

	return mDump;
}

UseCaseMgr::UseCaseMgr(MacroMap &mm) :
	mucm(NULL), mMM(mm), mAllDevices(0), mCard(-1), mActiveUseCaseCount(0) {
	pthread_mutex_init(&mLock, NULL);
}

UseCaseMgr::~UseCaseMgr() {
	if (mucm)
		snd_use_case_mgr_close(mucm);
	pthread_mutex_destroy(&mLock);
}

int UseCaseMgr::loadConfiguration() {
	static const char *map_file_name = ALSA_USE_CASE_DIR "/android_map.xml";

	if (loadUseCaseMap(map_file_name))
		return -1;

	mucm = snd_use_case_mgr_open(mUcmConfName.c_str());
	if (mucm == NULL)
		return -1;

	// TODO validate map vs. ucm configuration
	mCard = ah_card_find_by_name(mUcmConfName.c_str());
	uh_assert(mCard >= 0);

	return 0;
}

int UseCaseMgr::findEntry(audio_mode_t mode, audio_devices_t devices,
						  audio_output_flags_t flags, uclist_t::iterator &i) {
	LOGD("mode 0x%08x dev 0x%08x flags 0x%08x", mode, devices, flags);

	for (i = mUCList.begin(); i != mUCList.end(); ++i) {
		if (i->match(mode, devices)) {
			LOGD("Found: %s (active: %d)", i->dump().c_str(), i->mActive);
			return 0;
		}
	}
	return -1;
}

int UseCaseMgr::activateEntry(const uclist_t::iterator &i) {
	LOGE("%s(%s)", __func__, i->dump().c_str());
	if (i->mActive) {
		LOGE("Entry %s already active", i->dump().c_str());
		return -1;
	}
	AutoMutex lock(mLock);
	if (mActiveVerb.empty() || mActiveVerb == SND_USE_CASE_VERB_INACTIVE) {
		LOGV("Activating verb '%s'", i->mUcmVerb.c_str());
		uh_assert_se(!snd_use_case_set_verb(mucm, i->mUcmVerb.c_str()));
		// Only enable devices when enabling a verb
		// TODO more complere support would maintain a union of devices from
		//		active usecases.
		LOGV("Activating device '%s'", i->mUcmDevice.c_str());
		uh_assert_se(!snd_use_case_enable_device(mucm, i->mUcmDevice.c_str()));
	}
	else if (mActiveVerb != i->mUcmVerb) {
		LOGE("Request for conflicting verb '%s' current '%s'",
			 i->mUcmVerb.c_str(), mActiveVerb.c_str());
		return -1;
	}

	if (!i->mUcmModifier.empty()) {
		LOGV("Activating modifier '%s'", i->mUcmModifier.c_str());
		uh_assert_se(!snd_use_case_enable_modifier(mucm, i->mUcmModifier.c_str()));
	}

	mActiveUseCaseCount++;
	mActiveVerb = i->mUcmVerb;
	i->mActive = 1;
	return 0;
}

int UseCaseMgr::deactivateEntry(const uclist_t::iterator &i) {
	LOGE("%s(%s)", __func__, i->dump().c_str());
	if (!i->mActive) {
		LOGE("Entry %s already inactive", i->dump().c_str());
		return -1;
	}
	AutoMutex lock(mLock);
	mActiveUseCaseCount--;
	uh_assert(mActiveUseCaseCount >= 0);
	if (mActiveUseCaseCount == 0) {
		// Last active entry, just setting verb to "Inactive" should be enough
		LOGV("Deactivating current verb '%s'", mActiveVerb.c_str());
		uh_assert_se(!snd_use_case_set_verb(mucm, SND_USE_CASE_VERB_INACTIVE));
		mActiveVerb = SND_USE_CASE_VERB_INACTIVE;
	}
	else {
		// TODO more complere support would maintain a union of devices from
		//		active usecases.
		if (!i->mUcmModifier.empty()) {
			LOGV("Deactivating modifier '%s'", i->mUcmModifier.c_str());
		uh_assert_se(
				!snd_use_case_disable_modifier(mucm, i->mUcmModifier.c_str()));
		}
	}
	i->mActive = 0;
	return 0;
}

int UseCaseMgr::getPlaybackCard(const uclist_t::iterator &i) {
	return mCard;
}

int UseCaseMgr::getPlaybackPort(const uclist_t::iterator &i) {
	if (i->mUcmModifier.empty()) {
		return snd_use_case_get_verb_playback_pcm(mucm);
	}
	return snd_use_case_get_mod_playback_pcm(mucm, i->mUcmModifier.c_str());
}

int UseCaseMgr::loadUseCaseMap(const char *file) {
	TiXmlDocument xDoc;
	TiXmlElement *root, *e;

	LOGE("%s(file=%s)", __func__, file);

	if (!xDoc.LoadFile(file)) {
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

	e = root->FirstChildElement("ucm_conf_name");
	if (!e) {
		LOGE("Expected element <ucm_conf_name>");
		return 1;
	}
	// TODO should be more efficient to call e->ValueStr() instead of e->GetText()
	mUcmConfName = e->GetText();
	LOGD("UCM conf name \"%s\"", mUcmConfName.c_str());

	e = root->FirstChildElement("use_case_table");
	if (!e) {
		LOGE("Expected element <use_case_table>");
		return 1;
	}

	for (e = e->FirstChildElement("row"); e ;e = e->NextSiblingElement("row")) {
		TiXmlElement *in, *out, *t, *flag;
		const char* text;
		UseCaseMapEntry entry;

		// Get mode
		in = e->FirstChildElement("input");
		entry.mMode = mMM.mode(in->FirstChildElement("audio_mode")->GetText());

		// Get devices
		t = in->FirstChildElement("devices");
		entry.mDevices = 0;
		for (flag = t->FirstChildElement("flag"); flag;
			 flag = flag->NextSiblingElement("flag")) {
			entry.mDevices |= mMM.device(flag->GetText());
		}
		mAllDevices |= entry.mDevices;

		// Get device mask
		t = in->FirstChildElement("devices_mask");
		entry.mDevicesMask = 0;
		for (flag = t->FirstChildElement("flag"); flag;
			 flag = flag->NextSiblingElement("flag")) {
			entry.mDevicesMask |= mMM.device(flag->GetText());
		}

		/* Outputs */
		out = e->FirstChildElement("output");
		text = out->FirstChildElement("verb")->GetText();
		if (text) {
			entry.mUcmVerb = text;
		}

		text = out->FirstChildElement("device")->GetText();
		if (text) {
			entry.mUcmDevice = text;
		}

		text = out->FirstChildElement("modifier")->GetText();
		if (text) {
			entry.mUcmModifier = text;
		}

		LOGV("adding %s to map", entry.dump().c_str());

		mUCList.push_back(entry);
	}
	return 0;
}

}; // namespace UcmHal
