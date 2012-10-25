MYDROID = /home/oku/compile/android
TINYALSA = /home/oku/compile/tinyalsa
CSOURCES = use-case.c alsa-shim.c alsa-control.c audio_hw.c testmain.c 
CPPSOURCES = UcmHalDev.cpp UcmHalUseCaseMgr.cpp UcmHalMacroMap.cpp UcmHalOutStream.cpp UcmHalInStream.cpp 
HEADERS = use-case.h UcmHalDev.h UcmHalUseCaseMgr.h UcmHalMacroMap.h UcmHalOutStream.h UcmHalInStream.h
OBJECTS = $(CPPSOURCES:.cpp=.o) $(CSOURCES:.c=.o)
TINYALSAOBJS = $(TINYALSA)/pcm.o $(TINYALSA)/mixer.o 
CFLAGS = -O0 -g -Wall \
		 -DOMAP_ENHANCEMENT \
	  	 -DALSA_USE_CASE_DIR=\"/system/usr/share/alsa/ucm\" \
		 -I . \
		 -I $(TINYALSA)/include \
		 -I $(MYDROID)/system/core/include \
		 -I $(MYDROID)/hardware/libhardware/include

ucmhaltest: $(OBJECTS)
	g++ -o $@ $(CFLAGS) $(OBJECTS) $(TINYALSAOBJS) --static -ltinyxml -lpthread -ldl 

%.o : %.cpp
	g++ $(CFLAGS) -c -o $@ $<

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $<

clean :
	rm -f $(OBJECTS)

$(OBJECTS): $(HEADERS) 

install:
	adb push ucmhaltest system/xbin
	adb push ucm/Tablet44xx/Tablet44xx.conf system/usr/share/alsa/ucm/OMAP5EVM/OMAP5EVM.conf
	adb push ucm/Tablet44xx/hifi system/usr/share/alsa/ucm/OMAP5EVM
	adb push ucm/Tablet44xx/voice_call system/usr/share/alsa/ucm/OMAP5EVM
