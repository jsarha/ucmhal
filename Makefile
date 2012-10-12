CSOURCES = use-case.c audio_hw.c
CPPSOURCES = UcmHalDev.cpp UcmHalUseCaseMgr.cpp UcmHalMacroMap.cpp UcmHalOutStream.cpp UcmHalInStream.cpp testmain.cpp 
HEADERS = use-case.h UcmHalDev.h UcmHalUseCaseMgr.h UcmHalMacroMap.h UcmHalOutStream.h UcmHalInStream.h
OBJECTS = $(CPPSOURCES:.cpp=.o) $(CSOURCES:.c=.o)
CFLAGS = -O0 -g -Wall \
		 -DOMAP_ENHANCEMENT \
	  	 -DALSA_USE_CASE_DIR=\".\" \
		 -I . \
		 -I /home/oku/android/jellybean/system/core/include \
		 -I /home/oku/android/jellybean/hardware/libhardware/include

ucmhaltest: $(OBJECTS)
	g++ -o $@ $(CFLAGS) $(OBJECTS) -ltinyxml -lasound

%.o : %.cpp
	g++ $(CFLAGS) -c -o $@ $<

%.o : %.c
	gcc $(CFLAGS) -c -o $@ $<

clean :
	rm -f $(OBJECTS)

$(OBJECTS): $(HEADERS) $(CSOURCES) $(CPPSOURCES)

