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
#include <map>
#include <utility>

#include "UcmHalTypes.h"

struct str_parms;

namespace UcmHal {

class Parameters {
public:
	Parameters(const char *supported[]);
	~Parameters();

	int update(const char *kvpairs, std::list<const char *> *changed = NULL);
	string *get(const char *key, string &value) const;
	char *toStr() const;
private:
	const char **mSupported;
	str_parms *mparms;
};

template<class T>
class Hook {
public:
	Hook(T *obj, void (T::*hook)(const char *), const char *key) :
		mObj(obj), mHook(hook), mKey(key) {}
	void fire() {
		(mObj->*mHook)();
	}
private:
	const char *mKey;
	T *mObj;
	void (T::*mHook)();
};

template<class T>
class HookedParameters : public Parameters {
public:
	HookedParameters(const char *supported[]) : Parameters(supported) {}

	void setHook(T *obj, void (T::*hook)(const char *), const char *key) {
		mHooks.insert(Hook<T>(obj, hook, key));
	}

	int updateTrigger(const char *kvpairs) {
		std::list<const char *> changed;
		int ret = update(kvpairs, &changed);
		for (std::list<const char *>::iterator i = changed.begin();
		     i != changed.end(); i++) {
			if (mHooks.count(*i) == 1)
				mHooks.find(*i)->fire();
		}
		return ret;
	}
private:
	std::map<const char *, Hook<T>, strcomp> mHooks;
};


}; // namespace UcmHal
#endif /* UCMHALPARAMETERS_H */
