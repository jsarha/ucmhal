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

#ifndef UCMHALMACROMAP_H
#define UCMHALMACROMAP_H

#include <stdint.h>
#include <sys/types.h>
#include <cutils/log.h>

#include <string.h>
#include <iostream>
#include <map>
#include <utility>

namespace UcmHal {

class MacroMap {
public:
	MacroMap();
	~MacroMap();

	struct ltstr {
		bool operator()(const char* s1, const char* s2) const {
			return strcmp(s1, s2) < 0;
		}
	};

	int device(const char *name);
	int mode(const char *name);

private:
	typedef std::map<const char *, int, ltstr> MacroMap_t;
	MacroMap_t mDeviceMacroMap;
	MacroMap_t mModeMacroMap;
};

}; // namespace UcmHal 

#endif /* UCMHALMACROMAP_H */
