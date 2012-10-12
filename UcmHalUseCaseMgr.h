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

#include <string.h>
#include <iostream>
#include <set>
#include <utility>
#include <string>

#include <use-case.h>
#include <tinyxml.h>

#include "UcmHalMacroMap.h"

namespace UcmHal {

typedef std::basic_string<char> string;

class UseCaseMgr {
public:
	UseCaseMgr(MacroMap &mm);
	~UseCaseMgr();

	int loadConfiguration();

	int setUseCase(int mode, int devices);
	int getSupportedDeivices() {
		return mAllDevices;
	}
private:
	const char *mUcmConfName;
	snd_use_case_mgr_t *mucm;
	MacroMap &mMM;
	int mAllDevices;

	struct mapping {
		int mode;
		int devices;
		int devices_mask; 
		string ucm_verb;
		string ucm_device;
		string ucm_modifier; 
	};

	struct cmp {
		bool operator()(const mapping &a, const mapping &b) const {
			// For now, lets just sort according to mode
			return a.mode < b.mode;
			//return a.mode < b.mode ? true : a.devices < b.devices;
		}
	};

	// TODO store pointer (or ref) instead of object to avoid heap trashing
	// For current implementation just a list would do...
	typedef std::multiset<mapping, cmp> ucmset_t;
	ucmset_t mUCMap;

	int loadUseCaseMap(const char*);
};

}; // namespace UcmHal 

#endif // UCMHALUSECASEMGR_H
