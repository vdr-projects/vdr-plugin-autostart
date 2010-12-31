include ../Makefile.inc

OBJS = cdiotester.o configfileparser.o dbusdevkit.o \
		filedetector.o mediadetector.o mediatester.o \
		videodvdtester.o

OBJLIBS = ../detector.a 

TESTOBJS = detectortest.o

-include $(DEPFILE)

all: $(OBJLIBS)

$(OBJLIBS): $(OBJS) 
	@echo $(AR) $(ARFLAGS) $@ $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)
	
clean:
	@-rm -f $(OBJS)
	@-rm -f $(OBJLIBS)
	@-rm -f detectortest

### Implicit rules:

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<
	
### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cc) $(TESTOBJS:%.o=%.cc) > $@

### Build test programm
test: $(TESTOBJS) $(OBJLIBS)
	$(CXX) $(CXXFLAGS) $(LIBS) $(TESTOBJS) $(OBJLIBS) -o detectortest
	
force_look :
	true

	
