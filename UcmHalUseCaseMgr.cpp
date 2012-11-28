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

#include "UcmHalTypes.h"

#include <map>
#include <utility>

#include "UcmHalUseCaseMgr.h"
#include "UcmHalMacro.h"
#include "UcmHalStream.h"

#include "alsa-control.h"

namespace UcmHal {

void UseCaseMapEntry::dumpPcmSettings(string &str, const pcm_settings &settings) {
	str += " rates: ";
	for (std::set<int>::iterator i = settings.rates.begin();
		 i != settings.rates.end(); i++, str += ", ") appendInt(str, *i);
	str += " channel_counts: ";
	for (std::set<int>::iterator i = settings.channel_counts.begin();
		 i != settings.channel_counts.end(); i++, str += ", ") appendInt(str, *i);
	str += " period_size: ";
	appendInt(str, settings.period_size);
	str += " period_count: ";
	appendInt(str, settings.period_count);
	str += " min_threshold: ";
	appendInt(str, settings.min_threshold);
	str += " max_threshold: ";
	appendInt(str, settings.max_threshold);
}

const string &UseCaseMapEntry::dump() {
	if (!mDump.empty())
		return mDump;

	string devices;
	for (std::set<string>::iterator i = mUcm.devices.begin();
		 i != mUcm.devices.end(); i++, devices += ", ") devices += *i;
	string pcm;
	pcm += " in: ";
	dumpPcmSettings(pcm, mInSettings);
	pcm += " out: ";
	dumpPcmSettings(pcm, mOutSettings);

	char buf[1024];
	snprintf(buf, sizeof(buf),
	         "mode %d devices 0x%08x mask 0x%08x flags 0x%08x mask 0x%08x ->"
	         " verb '%s' mod '%s' dev '%s' pcm %s",
	         mAndroid.mode, mAndroid.devices, mAndroid.devices_mask,
			 mAndroid.flags, mAndroid.flags_mask, mUcm.verb.c_str(),
			 mUcm.modifier.c_str(), devices.c_str(), pcm.c_str());
	buf[sizeof(buf)-1] = '\0';
	mDump = buf;

	return mDump;
}

UseCaseMapEntry::UseCaseMapEntry(const UseCaseMapEntry &o) {
	mAndroid = o.mAndroid;
	mUcm = o.mUcm;
	mInSettings = o.mInSettings;
	mOutSettings = o.mOutSettings;
	mDump = o.mDump;
	mActive = NULL;
}

UseCaseMgr::UseCaseMgr(MacroMap &mm) :
	mucm(NULL), mMM(mm), mAllDevices(0), mCard(-1), mActiveUseCaseCount(0) {
}

UseCaseMgr::~UseCaseMgr() {
	AutoMutex lock(mLock);
	if (mucm)
		snd_use_case_mgr_close(mucm);
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

int UseCaseMgr::findEntry(uclist_t::iterator &i, audio_mode_t mode,
                          audio_devices_t devices, audio_output_flags_t flags) {
	ALOGD("mode %d dev 0x%08x flags 0x%08x", mode, devices, flags);

	for (i = mUCList.begin(); i != mUCList.end(); ++i) {
		if (i->match(mode, devices, flags)) {
			ALOGD("Found: %s (active: %d)", i->dump().c_str(), i->mActive);
			return 0;
		}
	}
	ALOGE("No matching mapping entry found.");
	return -1;
}

int UseCaseMgr::enableDevice(const string &device) {
	ALOGV("Activating device '%s' if not already active", device.c_str());
#ifdef UCMHAL_CHECK_INTEGRITY
	bool activate = true;
	for (auclist_t::iterator i = mActiveUCList.begin();
		 i != mActiveUCList.end(); i++) {
		if ((*i)->mUcm.devices.find(device) != (*i)->mUcm.devices.end()) {
			activate = false;
			break;
		}
	}
	if (activate) {
		uh_assert(!snd_use_case_get_device_status(mucm, device.c_str()));
		uh_assert_se(!snd_use_case_enable_device(mucm, device.c_str()));
	}
#else
	if (!snd_use_case_get_device_status(mucm, device.c_str())) {
		uh_assert_se(!snd_use_case_enable_device(mucm, device.c_str()));
		return 1;
	}
#endif
	return 0;
}

int UseCaseMgr::activateEntry(const uclist_t::iterator &uc, Stream *activator) {
	ALOGE("%s(%s)", __func__, uc->dump().c_str());
	uh_assert(uc != noEntry());
	if (uc->mActive) {
		ALOGE("Entry %s already active by %s", uc->dump().c_str(),
		      uc->mActive->dbgStr().c_str());
		return -1;
	}
	AutoMutex lock(mLock);
	if (mActiveVerb.empty()) {
		LOGV("Activating verb '%s'", uc->mUcm.verb.c_str());
		uh_assert_se(!snd_use_case_set_verb(mucm, uc->mUcm.verb.c_str()));
		mActiveVerb = uc->mUcm.verb;
	}
	else if (mActiveVerb != uc->mUcm.verb) {
		ALOGE("Request for conflicting verb '%s' current '%s'",
		      uc->mUcm.verb.c_str(), mActiveVerb.c_str());
		return -1;
	}
	if (!uc->mUcm.devices.empty()) {
		for (std::set<string>::iterator i = uc->mUcm.devices.begin();
			 i != uc->mUcm.devices.end(); i++)
			enableDevice((*i));
	}
	if (!uc->mUcm.modifier.empty()) {
		uh_assert(0 == snd_use_case_get_modifier_status(
					  mucm, uc->mUcm.modifier.c_str()));
		ALOGV("Activating modifier '%s'", uc->mUcm.modifier.c_str());
		uh_assert_se(!snd_use_case_enable_modifier(mucm, uc->mUcm.modifier.c_str()));
	}

	mActiveUseCaseCount++;
	mActiveUCSet.insert(uc);
	uh_assert(mActiveUseCaseCount == (int)mActiveUCSet.size());
	uc->mActive = activator;
	return 0;
}

int UseCaseMgr::disableDevice(const string &device) {
	ALOGV("Deactivating device '%s' if needed", device.c_str());
	bool deactivate = true;
	for (aucset_t::iterator i = mActiveUCSet.begin();
		 i != mActiveUCSet.end(); i++) {
		if ((*i)->mUcm.devices.find(device) != (*i)->mUcm.devices.end()) {
			deactivate = false;
		}
	}
	if (deactivate) {
		uh_assert(1 == snd_use_case_get_device_status(mucm, device.c_str()));
		uh_assert_se(!snd_use_case_disable_device(mucm, device.c_str()));
		return 1;
	}
	return 0;
}

int UseCaseMgr::deactivateEntry(const uclist_t::iterator &uc) {
	ALOGE("%s(%s)", __func__, uc->dump().c_str());
	if (!uc->mActive) {
		ALOGE("Entry %s already inactive", uc->dump().c_str());
		return -1;
	}
	AutoMutex lock(mLock);
	uc->mActive = NULL;
	mActiveUseCaseCount--;
	mActiveUCSet.erase(uc);
	uh_assert(mActiveUseCaseCount == (int)mActiveUCSet.size());
	if (mActiveUseCaseCount == 0) {
		// Last active entry, just setting verb to "Inactive" should be enough
		ALOGV("Deactivating current verb '%s'", mActiveVerb.c_str());
		uh_assert_se(!snd_use_case_set_verb(mucm, SND_USE_CASE_VERB_INACTIVE));
		mActiveVerb.clear();
	}
	else {
		if (!uc->mUcm.devices.empty()) {
			for (std::set<string>::iterator i = uc->mUcm.devices.begin();
				 i != uc->mUcm.devices.end(); i++)
				disableDevice(*i);
		}
		if (!uc->mUcm.modifier.empty()) {
			uh_assert(1 == snd_use_case_get_modifier_status(
						  mucm, uc->mUcm.modifier.c_str()));
			ALOGV("Deactivating modifier '%s'", uc->mUcm.modifier.c_str());
			uh_assert_se(
				!snd_use_case_disable_modifier(mucm, uc->mUcm.modifier.c_str()));
		}
	}
	uc->mActive = NULL;
	return 0;
}

int UseCaseMgr::getPlaybackCard(const uclist_t::iterator &i) {
	return mCard;
}

int UseCaseMgr::getPlaybackPort(const uclist_t::iterator &i) {
	if (i->mUcm.modifier.empty()) {
		return snd_use_case_get_verb_playback_pcm(mucm);
	}
	return snd_use_case_get_mod_playback_pcm(mucm, i->mUcm.modifier.c_str());
}

int UseCaseMgr::getCaptureCard(const uclist_t::iterator &i) {
	return mCard;
}

int UseCaseMgr::getCapturePort(const uclist_t::iterator &i) {
	if (i->mUcm.modifier.empty()) {
		return snd_use_case_get_verb_capture_pcm(mucm);
	}
	return snd_use_case_get_mod_capture_pcm(mucm, i->mUcm.modifier.c_str());
}

int UseCaseMgr::changeStandby(const uclist_t::iterator &o,
							  const uclist_t::iterator &n) const {
	return 1;
	/* Could be something like bellow, but let's suspend every time for now
	   return (o.mUcm.verb != n.mUcm.verb ||
	   snd_use_case_get_mod_playback_pcm(mucm, n->mUcm.modifier.c_str()) !=
	   snd_use_case_get_mod_playback_pcm(mucm, o->mUcm.modifier.c_str()));
	*/
}

int UseCaseMgr::xmlGetSet(TiXmlElement *list, const char *name,
						  std::set<string> &set) {
	int count = 0;
	for (TiXmlElement *e = list->FirstChildElement(name); e;
		 e = e->NextSiblingElement(name)) {
		set.insert(e->GetText());
		count++;
	}
	return count;
}

int UseCaseMgr::xmlGetSet(TiXmlElement *list, const char *name,
						  std::set<int> &set) {
	int count = 0;
	for (TiXmlElement *e = list->FirstChildElement(name); e;
		 e = e->NextSiblingElement(name)) {
		set.insert(atoi(e->GetText()));
		count++;
	}
	return count;
}

int UseCaseMgr::xmlGetFlags(TiXmlElement *list, int &flags,
							int (MacroMap::*method)(const char *)) {
	int count = 0;
	for (TiXmlElement *e = list->FirstChildElement("flag"); e;
		 e = e->NextSiblingElement("flag")) {
		flags |= (mMM.*method)(e->GetText());
		count++;
	}
	return count;
}

int UseCaseMgr::xmlGetPcmSettings(TiXmlElement *pcm_settings,
								  UseCaseMapEntry::pcm_settings &settings) {
	TiXmlElement *e;
	if ((e = pcm_settings->FirstChildElement("rate"))) {
		settings.rates.clear();
		uh_return_failure_if(!xmlGetSet(e, "list", settings.rates));
		settings.default_rate = atoi(e->FirstChildElement("list")->GetText());
	}
	if ((e = pcm_settings->FirstChildElement("channel_count"))) {
		settings.channel_counts.clear();
		uh_return_failure_if(!xmlGetSet(e, "list", settings.channel_counts));
		settings.default_channels =
			atoi(e->FirstChildElement("list")->GetText());
	}

	if ((e = pcm_settings->FirstChildElement("period_size")) && e->GetText())
		settings.period_size = atoi(e->GetText());
	if ((e = pcm_settings->FirstChildElement("period_count")) && e->GetText())
		settings.period_count = atoi(e->GetText());
	if ((e = pcm_settings->FirstChildElement("min_threshold")) && e->GetText())
		settings.min_threshold = atoi(e->GetText());
	if ((e = pcm_settings->FirstChildElement("max_threshold")) && e->GetText())
		settings.max_threshold = atoi(e->GetText());
	return 0;
}

int UseCaseMgr::xmlGetEntryInput(TiXmlElement *in, UseCaseMapEntry &entry) {
	TiXmlElement *e;

	uh_return_failure_if(!(e = in->FirstChildElement("audio_mode")));
	entry.mAndroid.mode = mMM.mode(e->GetText());

	uh_return_failure_if(!(e = in->FirstChildElement("devices")));
	xmlGetFlags(e, entry.mAndroid.devices, &MacroMap::device);

	uh_return_failure_if(!(e = in->FirstChildElement("devices_mask")));
	xmlGetFlags(e, entry.mAndroid.devices_mask, &MacroMap::device);

	// Get flags if they are there
	entry.mAndroid.flags = 0;
	entry.mAndroid.flags_mask = 0;
	e = in->FirstChildElement("flags");
	if (e) {
		xmlGetFlags(e, entry.mAndroid.flags, &MacroMap::flag);
		uh_return_failure_if(!(e = in->FirstChildElement("flags_mask")));
		xmlGetFlags(e, entry.mAndroid.flags_mask, &MacroMap::flag);
	}
	return 0;
}

int UseCaseMgr::xmlGetEntryOutput(TiXmlElement *out, UseCaseMapEntry &entry) {
	TiXmlElement *e;
	// All child elements are optional.
	e = out->FirstChildElement("verb");
	if (e && e->GetText()) {
		entry.mUcm.verb = e->GetText();
	}

	e = out->FirstChildElement("modifier");
	if (e && e->GetText()) {
		entry.mUcm.modifier = e->GetText();
	}

	// Device. Also a single text element is allowed
	e = out->FirstChildElement("device");
	if (e) {
		entry.mUcm.devices.clear();
		if (0 == xmlGetSet(e, "list", entry.mUcm.devices))
			entry.mUcm.devices.insert(e->GetText());
	}

	e = out->FirstChildElement("in_pcm_settings");
	if (e) {
		uh_return_failure_if(xmlGetPcmSettings(e, entry.mInSettings));
	}

	e = out->FirstChildElement("out_pcm_settings");
	if (e) {
		uh_return_failure_if(xmlGetPcmSettings(e, entry.mOutSettings));
	}

	return 0;
}

int UseCaseMgr::xmlGetEntry(TiXmlElement *row, UseCaseMapEntry &entry) {
	TiXmlElement *in, *out;

	// Input
	uh_return_failure_if(!(in = row->FirstChildElement("input")));
	uh_return_failure_if(xmlGetEntryInput(in, entry));

	// Output
	uh_return_failure_if(!(out = row->FirstChildElement("output")));
	uh_return_failure_if(xmlGetEntryOutput(out, entry));

	return 0;
}

// Rewrite in separate functions...
int UseCaseMgr::loadUseCaseMap(const char *file) {
	TiXmlDocument xDoc;
	TiXmlElement *root, *table, *e;

	ALOGE("%s(file=%s)", __func__, file);

	uh_return_failure_if(!xDoc.LoadFile(file));

	uh_return_failure_if(!(root = xDoc.RootElement()));

	uh_return_failure_if(0 != strcmp(root->Value(), "android_use_case_map"));

	uh_return_failure_if(!(e = root->FirstChildElement("ucm_conf_name")));

	mUcmConfName = e->GetText();
	ALOGD("UCM conf name \"%s\"", mUcmConfName.c_str());

	uh_return_failure_if(!(e = root->FirstChildElement("output_default")));
	uh_return_failure_if(xmlGetEntryOutput(e, mEntryDefault));

	uh_return_failure_if(!(table = root->FirstChildElement("use_case_table")));

	for (e = table->FirstChildElement("row"); e ;
		 e = e->NextSiblingElement("row")) {
		mUCList.push_back(mEntryDefault);
		xmlGetEntry(e, mUCList.back());
		ALOGV("added %s to map", mUCList.back().dump().c_str());
	}
	return 0;
}

}; // namespace UcmHal
