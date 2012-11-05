# Copyright (C) 2012 Texas Instruments
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

###
### UCM ABE AUDIO HAL
###

ifeq  ($(strip $(BOARD_USES_UCM_AUDIO_HW)),true)

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Should change this so the enable variable gets used as the name?
#LOCAL_MODULE := audio.primary.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE := audio.primary.ucmhal
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_SRC_FILES := \
	audio_hw.c \
	use-case.c \
	alsa-shim.c \
	alsa-control.c \
	UcmHalDev.cpp \
	UcmHalUseCaseMgr.cpp \
	UcmHalMacroMap.cpp \
	UcmHalOutStream.cpp \
	UcmHalInStream.cpp \
	UcmHalParameters.cpp

LOCAL_C_INCLUDES += \
	external/tinyalsa/include \
	external/tinyxml \
	system/media/audio_utils/include \
	system/media/audio_effects/include \

LOCAL_SHARED_LIBRARIES := liblog libcutils libtinyalsa libaudioutils \
	libdl libexpat libtinyxml
LOCAL_STATIC_LIBRARIES := libastl
LOCAL_MODULE_TAGS := optional
LOCAL_CFLAGS += -DALSA_USE_CASE_DIR=\"/system/usr/share/alsa/ucm\"

# Use STLport
LOCAL_C_INCLUDES += external/stlport/stlport
#LOCAL_STATIC_LIBRARIES += libstlport_static
LOCAL_SHARED_LIBRARIES += libstlport

include $(BUILD_SHARED_LIBRARY)

endif
