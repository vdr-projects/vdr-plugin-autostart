include ../Makefile.inc

OBJS = cdiotester.o configfileparser.o dbusdevkit.o \
		filedetector.o mediadetector.o mediatester.o \
		videodvdtester.o
HEADER = $(OBJS:%.o=%.h) logger.h stringtools.h

OBJLIBS = ../detector.a 

TESTOBJS = detectortest.o

-include $(DEPFILE)

all: $(OBJLIBS)

$(OBJLIBS): $(OBJS) $(HEADER)
	@echo $(AR) $(ARFLAGS) $@ $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)
	
clean:
	@-rm -f $(OBJS) $(TESTOBJS)
	@-rm -f $(OBJLIBS)
	@-rm -f detectortest
	@-rm -f core

### Implicit rules:

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<
	
### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cc) $(TESTOBJS:%.o=%.cc) $(HEADER)> $@

### Build test programm
test: $(TESTOBJS) $(OBJLIBS) $(OBJS) $(HEADER)
	$(CXX) $(CXXFLAGS) $(LIBS) $(TESTOBJS) $(OBJLIBS) -o detectortest
	
force_look :
	true

	
