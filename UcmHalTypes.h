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
#include <string.h>

#ifndef NO_ANDROID_STL
#define _STLP_HAS_INCLUDE_NEXT  1
#define _STLP_USE_MALLOC   1
#define _STLP_USE_NO_IOSTREAMS  1
#include <stl/config/_android.h>
#endif

#include <set>
#include <utility>
#include <string>

namespace UcmHal {

class Dev;
class OutStream;
class InStream;
class UseCaseMgr;

class Mutex {
public:
	inline Mutex() {
		// Using recursive mutexes make code much simpler
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&mMutex, &attr);
	}
	inline ~Mutex() { pthread_mutex_destroy(&mMutex); }
	inline int lock() { return pthread_mutex_lock(&mMutex); }
	inline int unlock() { return pthread_mutex_unlock(&mMutex); }
private:
	pthread_mutex_t mMutex;
};

class AutoMutex {
public:
	inline AutoMutex(Mutex &mutex) : mMutex(mutex) {
		mMutex.lock();
	}
	inline AutoMutex(Mutex *mutex) : mMutex(*mutex) {
		mMutex.lock();
	}
	inline ~AutoMutex() { mMutex.unlock(); }
private:
	Mutex &mMutex;
};

typedef std::string string;

struct strcomp {
	bool operator()(const char *a, const char *b) const {
		return 0 > ::strcmp(a, b);
	}
	bool operator()(const char *&a, const char *&b) const {
		return 0 > ::strcmp(a, b);
	}
};

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
