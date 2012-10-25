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

#ifndef UCMHALTYPES_H
#define UCMHALTYPES_H

#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>

#include <set>
#include <utility>
#include <string>

namespace UcmHal {

class Dev;
class OutStream;
class InStream;
class UseCaseMgr;

class AutoMutex {
public:
	inline AutoMutex(pthread_mutex_t mutex) : mMutex(&mutex) {
		pthread_mutex_lock(mMutex);
	}
	inline AutoMutex(pthread_mutex_t *mutex) : mMutex(mutex) {
		pthread_mutex_lock(mMutex);
	}
	inline ~AutoMutex() { pthread_mutex_unlock(mMutex); }
private:
	pthread_mutex_t *mMutex;
};

typedef std::basic_string<char> string;

template<typename T>
struct ptrcmp {
	bool operator()(const T *a, const T *b) const {
		return a < b;
	}
};

typedef std::set<OutStream *, ptrcmp<OutStream> > OutStreamSet_t;
typedef std::set<InStream *, ptrcmp<InStream> > InStreamSet_t;

}; // namespace UcmHal
#endif /* UCMHALSTLTYPES_H */
