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

#include <system/audio.h>

#include "UcmHalParameters.h"
#include "UcmHalMacro.h"

// cutils/str_parms.h is broken...
extern "C" {
#include <cutils/str_parms.h>
}

namespace UcmHal {

Parameters::Parameters(const char **supported) : mSupported(supported) {
	uh_assert_se(mparms = str_parms_create());
}

Parameters::~Parameters() {
	str_parms_destroy(mparms);
}

int Parameters::update(const char *kvpairs, std::list<const char *> *changed) {
	struct str_parms *parms = str_parms_create_str(kvpairs);
    uh_assert(parms);
    int parms_changed = 0;

    for (int i=0; mSupported[i]; i++) {
	    int ret;
	    char nvalue[32];
	    ret = str_parms_get_str(parms, mSupported[i], nvalue, sizeof(nvalue));
	    if (ret >= 0) {
		    char ovalue[32];
		    ret = str_parms_get_str(mparms, mSupported[i], ovalue, sizeof(ovalue));
		    if (ret < 0 || 0 != strcmp(nvalue, ovalue)) {
			    LOGV("Parameter \"%s\" changed from \"%s\" to \"%s\"",
			         mSupported[i], ovalue, nvalue);
			    str_parms_add_str(mparms, mSupported[i], nvalue);
			    if (changed)
				    changed->push_back(mSupported[i]);
			    parms_changed++;
		    }
	    }
    }
    str_parms_destroy(parms);
    return 0;
}

string *Parameters::get(const char *key, string &value) const {
	int ret;
	char val[32];
	value.clear();
	ret = str_parms_get_str(mparms, key, val, sizeof(value));
	if (ret >= 0)
		value = val;
	return &value;
}

char *Parameters::toStr() const {
	return str_parms_to_str(mparms);
}

}; // namespace UcmHal
