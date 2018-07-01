#
# Makefile for the autostart Video Disk Recorder plugin
#
# $Id$

### Include defaults for the plugin

include Makefile.inc

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'static const char \*VERSION *=' $(PLUGIN).h | awk '{ print $$6 }' | sed -e 's/[";]//g')
PKGCFG = $(if $(VDRDIR),$(shell pkg-config --variable=$(1) $(VDRDIR)/vdr.pc),$(shell pkg-config --variable=$(1) vdr || pkg-config --variable=$(1) ../../../vdr.pc))
CFGDIR = $(call PKGCFG,configdir)/plugins/$(PLUGIN)
PLGCFG = $(call PKGCFG,plgcfg)

ifneq (exists, $(shell pkg-config dbus-1 && echo exists))
  $(warning ******************************************************************)
  $(warning 'dbus development' not detected! ')
  $(warning ******************************************************************)
endif

ifneq (exists, $(shell pkg-config libcdio && echo exists))
  $(warning ******************************************************************)
  $(warning 'libcdio' not detected! ')
  $(warning ******************************************************************)
endif

ifneq (exists, $(shell pkg-config dvdread && echo exists))
  $(warning ******************************************************************)
  $(warning 'libdvdread' not detected! ')
  $(warning ******************************************************************)
endif

### The version number of VDR's plugin API (taken from VDR's "config.h"):

APIVERSION = $(call PKGCFG,apiversion)

### Allow user defined options to overwrite defaults:

-include $(PLGCFG)

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### The name of the shared object file:

SOFILE = libvdr-$(PLUGIN).so

### Includes and Defines (add further entries here):

INCLUDES += 

DEFINES += -DPLUGIN_NAME_I18N='"$(PLUGIN)"'
# Uncomment to enable additional debug output
DEFINES += -DDEBUG

### The object files (add further files here):

OBJS = $(PLUGIN).o mediadetectorthread.o configmenu.o

# Options for dbus
LIBS = $(shell pkg-config --libs dbus-1)
CXXFLAGS += $(shell pkg-config --cflags dbus-1)
# Options for cdio
LIBS += $(shell pkg-config --libs libcdio)
CXXFLAGS += $(shell pkg-config --cflags libcdio)

# Options for DVDRead
LIBS += $(shell pkg-config --libs dvdread)
CXXFLAGS += $(shell pkg-config --cflags dvdread)

### The main target:

all: $(SOFILE) i18n detector

### Implicit rules:

%.o: %.cc
	@echo "Pkg" $(PKGCFG) , $(VDRDIR)
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<
### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cc) > $@

-include $(DEPFILE)

### Internationalization (I18N):

PODIR     = po
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmo    = $(addsuffix .mo, $(foreach file, $(I18Npo), $(basename $(file))))
I18Nmsgs  = $(addprefix $(DESTDIR)$(LOCDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.cc *.h)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --msgid-bugs-address='<see README>' -o $@ `ls $^`

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q -N $@ $<
	@touch $@

$(I18Nmsgs): $(DESTDIR)$(LOCDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	install -D -m644 $< $@

.PHONY: i18n
i18n: $(I18Nmo) $(I18Npot)

install-i18n: $(I18Nmsgs)

### Targets:

detector.a: 
	@cd detector; $(MAKE)
	

$(SOFILE): $(OBJS) detector.a
	$(CXX) $(CXXFLAGS) -shared $(OBJS) detector.a $(LIBS) -o $@

install-lib: $(SOFILE)
	install -D $^ $(DESTDIR)$(LIBDIR)/$^.$(APIVERSION)

install: install-lib install-i18n

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

.PHONY: clean detector.a
clean:
	@-rm -f $(OBJS) $(TESTOBJS) $(DEPFILE) *.so *.tgz core* *~ $(PODIR)/*.mo $(PODIR)/*.pot 
	@cd detector; $(MAKE) clean
	

