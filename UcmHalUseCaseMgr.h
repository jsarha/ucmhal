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

#ifndef UCMHALUSECASEMGR_H
#define UCMHALUSECASEMGR_H

#include <stdint.h>
#include <sys/types.h>
#include <cutils/log.h>

#include <tinyxml.h>

#include <hardware/hardware.h>
#include <system/audio.h>
#include <hardware/audio.h>

#include "use-case.h"

#include "UcmHalTypes.h"

#include <list>
#include <set>

#include "UcmHalMacroMap.h"

namespace UcmHal {

class Stream;

class UseCaseMapEntry {
private:
	struct android {
		int mode;
		int devices;
		int devices_mask;
		int flags;
		int flags_mask;
	} mAndroid;
	struct ucm {
		string verb;
		std::set<string> devices;
		string modifier;
	} mUcm;
	struct pcm_settings {
		std::set<int> rates;
		int default_rate;
		std::set<int> channel_counts;
		int default_channels;
		int period_size;
		int period_count;
		int min_threshold;
		int max_threshold;
	};
	pcm_settings mInSettings;
	pcm_settings mOutSettings;

	string mDump;
	Stream *mActive;
public:
	UseCaseMapEntry() {
		memset(&mAndroid, 0, sizeof(mAndroid));
		mInSettings.period_size = 0;
		mInSettings.period_count = 0;
		mInSettings.min_threshold = 0;
		mInSettings.max_threshold = 0;
		mOutSettings.period_size = 0;
		mOutSettings.period_count = 0;
		mOutSettings.min_threshold = 0;
		mOutSettings.max_threshold = 0;
	}
	UseCaseMapEntry(const UseCaseMapEntry &o);
	int match(const int mode, const int devices, const int flags) {
		return (mAndroid.mode == mode &&
		        (mAndroid.devices & mAndroid.devices_mask) ==
				(devices & mAndroid.devices_mask) &&
		        (mAndroid.flags & mAndroid.flags_mask) ==
				(flags & mAndroid.flags_mask));
	}

	template<class T>
	int equalSet(const std::set<T> &a, const std::set<T> &b) const {
		if (a.size() != b.size())
			return 0;
		for (typename std::set<T>::iterator i = a.begin(); i != a.end(); i++)
			if (b.find(*i) == b.end())
				return 0;
		return 1;
	}

	int equalPcmSettings(const pcm_settings &a, const pcm_settings &b) const {
		return (equalSet<int>(a.rates, b.rates) &&
				equalSet<int>(a.channel_counts, b.channel_counts) &&
				a.period_size == b.period_size &&
				a.period_count == b.period_count &&
				a.min_threshold == b.min_threshold &&
				a.max_threshold == b.max_threshold);
	}

	int equal(UseCaseMapEntry &o) const {
		return (mUcm.verb == o.mUcm.verb && mUcm.modifier == o.mUcm.modifier &&
				equalSet<string>(mUcm.devices, o.mUcm.devices) &&
				equalPcmSettings(mInSettings, o.mInSettings) &&
				equalPcmSettings(mOutSettings, o.mOutSettings));
	}

	static void dumpPcmSettings(string &str, const pcm_settings &settings);
	const string &dump();
	Stream *active() { return mActive; }
	friend class UseCaseMgr;
	friend class Stream;
	friend class OutStream;
	friend class InStream;
};

typedef std::list<UseCaseMapEntry> uclist_t;
struct aucsetcmp {
	bool operator()(const uclist_t::iterator &a, const uclist_t::iterator &b) const {
		return &(*a) > &(*b);
	}
};
typedef std::set<uclist_t::iterator, aucsetcmp> aucset_t;

/* TODO for optimization fill to below map as the matches are found
   struct usecase_map_key {
   int mode;
   int devices;
   int flags;
   }
   struct usecase_map_key_cmp {
   bool operator()(const usecase_map_key &a,
   const usecase_map_key &b) const {
   return a.mode != b.mode ? a.mode < b.mode : a.devices < b.devices;
   }
   };
   typedef std::map<usecase_map_key, uclist_t::iterator> ucmap_t;
*/

class UseCaseMgr {
public:
	UseCaseMgr(MacroMap &mm);
	~UseCaseMgr();

	int loadConfiguration();
	int findEntry(uclist_t::iterator &entry, audio_mode_t mode,
	              audio_devices_t devices,
	              audio_output_flags_t flags = AUDIO_OUTPUT_FLAG_NONE);
	int activateEntry(const uclist_t::iterator &entry, Stream *activator);
	int deactivateEntry(const uclist_t::iterator &entry);

	int getPlaybackCard(const uclist_t::iterator &entry);
	int getPlaybackPort(const uclist_t::iterator &entry);

	int getCaptureCard(const uclist_t::iterator &entry);
	int getCapturePort(const uclist_t::iterator &entry);

	int changeStandby(const uclist_t::iterator &o,
					  const uclist_t::iterator &n) const;

	int getSupportedDeivices() const { return mAllDevices; }
	const uclist_t::const_iterator noEntry() const { return mUCList.end(); }

	const string &activeVerb() { return mActiveVerb; }
private:
	Mutex mLock;
	string mUcmConfName;
	snd_use_case_mgr_t *mucm;
	MacroMap &mMM;
	int mAllDevices;
	int mCard;
	int mActiveUseCaseCount;
	string mActiveVerb;

	UseCaseMapEntry mEntryDefault;

	uclist_t mUCList;
	aucset_t mActiveUCSet;

	int disableDevice(const string &device);
	int enableDevice(const string &device);

	int xmlGetSet(TiXmlElement *list, const char *name, std::set<string> &set);
	int xmlGetSet(TiXmlElement *list, const char *name, std::set<int> &set);
	int xmlGetFlags(TiXmlElement *list, int &flags,
					int (MacroMap::*method)(const char *));
	int xmlGetPcmSettings(TiXmlElement *pcm_settings,
						  UseCaseMapEntry::pcm_settings &settings);
	int xmlGetEntryInput(TiXmlElement *in, UseCaseMapEntry &entry);
	int xmlGetEntryOutput(TiXmlElement *out, UseCaseMapEntry &entry);
	int xmlGetEntry(TiXmlElement *row, UseCaseMapEntry &entry);
	int loadUseCaseMap(const char*);
};

}; // namespace UcmHal

#endif // UCMHALUSECASEMGR_H
