# This make file builds a statically linked test binary.
# For compilation a linux build environment with compatible architecture is
# needed. For ARM based system I am using an omap4-ubuntu chroot with
# qemu.
MYDROID = /home/oku/compile/android
TINYALSA = /home/oku/compile/tinyalsa
CSOURCES = use-case.c alsa-shim.c alsa-control.c audio_hw.c test/testmain.c
CPPSOURCES = \
	UcmHalDev.cpp \
	UcmHalUseCaseMgr.cpp \
	UcmHalMacroMap.cpp \
	UcmHalOutStream.cpp \
	UcmHalInStream.cpp \
	UcmHalParameters.cpp

HEADERS = \
	use-case.h \
	alsa-shim.h \
	alsa-control.h \
	UcmHalDev.h \
	UcmHalUseCaseMgr.h \
	UcmHalMacroMap.h \
	UcmHalOutStream.h \
	UcmHalInStream.h \
	UcmHalParameters.h \
	UcmHalTypes.h \
	UcmHalMacro.h \
	test/cutils/log.h

OBJECTS = $(CPPSOURCES:.cpp=.o) $(CSOURCES:.c=.o)
ANDROIDOBJECTS = \
	test/cutils/str_parms.o \
	test/cutils/hashmap.o \
	test/cutils/memory.o
TINYALSAOBJS = $(TINYALSA)/pcm.o $(TINYALSA)/mixer.o
CFLAGS = -O0 -g -Wall \
		 -DOMAP_ENHANCEMENT \
	  	 -DALSA_USE_CASE_DIR=\"/system/usr/share/alsa/ucm\" \
		 -DHAVE_PTHREADS=1 \
		 -I test \
		 -I $(TINYALSA)/include \
		 -I $(MYDROID)/system/core/include \
		 -I $(MYDROID)/hardware/libhardware/include

LDFLAGS = --static -ltinyxml -lpthread -ldl

test/cutils/%.o: 
	gcc $(CFLAGS) -c -o $@ $(patsubst test/cutils/%.o, $(MYDROID)/system/core/libcutils/%.c, $@)

ucmhaltest: $(OBJECTS) $(ANDROIDOBJECTS)
	g++ -o $@ $(CFLAGS) $(OBJECTS) $(TINYALSAOBJS) $(ANDROIDOBJECTS) $(LDFLAGS)

%.o : %.cpp
	g++ $(CFLAGS) -c -o $@ $<

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(ANDROIDOBJECTS)

$(OBJECTS): $(HEADERS)

install:
	adb push ucmhaltest system/xbin
	adb push ucm/OMAP5EVM system/usr/share/alsa/ucm/OMAP5EVM
