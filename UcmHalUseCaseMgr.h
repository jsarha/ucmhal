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

#include "UcmHalMacroMap.h"

namespace UcmHal {

class Stream;

class UseCaseMapEntry {
public:
	UseCaseMapEntry() : mMode(0), mDevices(0), mDevicesMask(0),
	                    mFlags(0), mFlagsMask(0), mActive(NULL) {}
	int match(const int mode, const int devices, const int flags) {
		return (mode == mMode &&
		        (mDevices & mDevicesMask) == (devices & mDevicesMask) &&
		        (mFlags & mFlagsMask) == (flags & mFlagsMask));
	}
	const string &dump();
	Stream *active() { return mActive; }
	int equal(UseCaseMapEntry &o) const {
		return (mUcmVerb == o.mUcmVerb && mUcmDevice == o.mUcmDevice &&
		        mUcmModifier == o.mUcmModifier);
	}
	friend class UseCaseMgr;
private:
	int mMode;
	int mDevices;
	int mDevicesMask;
	int mFlags;
	int mFlagsMask;
	string mUcmVerb;
	string mUcmDevice;
	string mUcmModifier;
	Stream *mActive;
	string mDump;
};

typedef std::list<UseCaseMapEntry> uclist_t;

/* TODO for optimization fill to below map as the matches are found
   struct usecase_map_key {
   int mode;
   int devices;
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

	// TODO need input/output parameter
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

	uclist_t mUCList;

	int loadUseCaseMap(const char*);
};

}; // namespace UcmHal

#endif // UCMHALUSECASEMGR_H
