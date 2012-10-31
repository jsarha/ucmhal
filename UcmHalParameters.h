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

#ifndef UCMHALPARAMETERS_H
#define UCMHALPARAMETERS_H

#include <stdint.h>
#include <sys/types.h>
#include <cutils/log.h>

#include <string.h>
#include <list>
#include <utility>

#include "UcmHalTypes.h"

struct str_parms;

namespace UcmHal {

//template<T>
class Parameters {
public:
	Parameters(const char *supported[]);
	~Parameters();
	
	//void setHook(T *obj, void (T::*hook), const char *key);
		             
	int update(const char *kvpairs, std::list<const char *> *changed = NULL);
	string *get(const char *key, string &value) const;
	char *toStr() const;
private:
	const char **mSupported;
	str_parms *mparms;
//	std::map<const char *, Hook<T>, keycmp> mHooks;
};

/* Hook implementation postponed, not sure if they are needed
template<T>
class Hook {
public:
	Hook(T *obj, void (T::*hook)(const char *), const char *key) :
		obj(mObj, mHook(hook), mKey(key) {}
	void fire(const char *val) {
		(mObj->*mHook)(val);
	}
private:
	const char *mKey;
	T *mObj;
	void (T::*mHook)(const char *val);
}
*/

}; // namespace UcmHal
#endif /* UCMHALPARAMETERS_H */
