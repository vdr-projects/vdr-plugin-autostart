### Include defaults for the plugin

include ../Makefile.inc

PKGCFG = $(if $(VDRDIR),$(shell pkg-config --variable=$(1) $(VDRDIR)/vdr.pc),$(shell pkg-config --variable=$(1) vdr || pkg-config --variable=$(1) ../../../../vdr.pc))
# Options for dbus
LIBS = $(shell pkg-config --libs dbus-1)
CXXFLAGS += $(shell pkg-config --cflags dbus-1)
# Options for cdio
LIBS += $(shell pkg-config --libs libcdio)
CXXFLAGS += $(shell pkg-config --cflags libcdio)
# Options for DVDRead
LIBS += $(shell pkg-config --libs dvdread)
CXXFLAGS += $(shell pkg-config --cflags dvdread)

OBJS = cdiotester.o configfileparser.o dbusdevkit.o \
		filetester.o mediadetector.o mediatester.o \
		videodvdtester.o
HEADER = $(OBJS:%.o=%.h) logger.h stringtools.h dbusdevkit.h

OBJLIBS = ../detector.a 

TESTOBJS = detectortest.o
INCLUDES += -I$(VDRDIR)/../include

-include $(DEPFILE)



.PHONY: all 
all: $(OBJLIBS)

$(OBJLIBS): $(OBJS) $(HEADER)
	@echo $(AR) $(ARFLAGS) $@ $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

.PHONY: clean
clean:
	@-rm -f $(OBJS) $(TESTOBJS)
	@-rm -f $(OBJLIBS)
	@-rm -f detectortest
	@-rm -f core *~

### Implicit rules:

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<
	
### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cc) $(TESTOBJS:%.o=%.cc) $(HEADER)> $@

### Build test programm

.PHONY: test
test: CXXFLAGS += -D_NOVDR_ -DDEBUG
test: $(TESTOBJS) $(OBJLIBS) $(OBJS) $(HEADER)
	$(CXX) $(CXXFLAGS) $(TESTOBJS) ../detector.a $(LIBS) -o detectortest
	

