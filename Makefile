#
# Makefile for the autostart Video Disk Recorder plugin
#
# $Id$

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
# IMPORTANT: the presence of this macro is important for the Make.config
# file. So it must be defined, even if it is not used here!
#
PLUGIN = autostart

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep 'static const char \*VERSION *=' $(PLUGIN).h | awk '{ print $$6 }' | sed -e 's/[";]//g')

### Include defaults for the plugin

include Makefile.inc

### Make sure that necessary options are included:

include $(VDRDIR)/Make.global

### Allow user defined options to overwrite defaults:

-include $(VDRDIR)/Make.config

ifneq (exists, $(shell pkg-config dbus-1 && echo exists))
  $(warning ******************************************************************)
  $(warning 'dbus development' not detected! ')
  $(warning ******************************************************************)
endif

ifneq (exists, $(shell pkg-config libcdio_cdda && echo exists))
  $(warning ******************************************************************)
  $(warning 'libcdio_cdda' not detected! ')
  $(warning ******************************************************************)
endif

ifneq (exists, $(shell pkg-config dvdread && echo exists))
  $(warning ******************************************************************)
  $(warning 'libdvdread' not detected! ')
  $(warning ******************************************************************)
endif

### The version number of VDR's plugin API (taken from VDR's "config.h"):

APIVERSION = $(shell sed -ne '/define APIVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### Includes and Defines (add further entries here):

INCLUDES += -I$(VDRDIR)/include

DEFINES += -D_GNU_SOURCE -DPLUGIN_NAME_I18N='"$(PLUGIN)"'
# Uncomment to enable additional debug output
DEFINES += -DDEBUG

### The object files (add further files here):

OBJS = $(PLUGIN).o mediadetectorthread.o configmenu.o

# Options for dbus
LIBS = $(shell pkg-config --libs dbus-1)
CXXFLAGS += $(shell pkg-config --cflags dbus-1)
# Options for cdio
LIBS += $(shell pkg-config --libs libcdio_cdda)
CXXFLAGS += $(shell pkg-config --cflags libcdio_cdda)
# Options for DVDRead
LIBS += $(shell pkg-config --libs dvdread)
CXXFLAGS += $(shell pkg-config --cflags dvdread)

### The main target:

all: libvdr-$(PLUGIN).so i18n

### Implicit rules:

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

### Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cc) > $@

-include $(DEPFILE)

### Internationalization (I18N):

PODIR     = po
LOCALEDIR = $(VDRDIR)/locale
I18Npo    = $(wildcard $(PODIR)/*.po)
I18Nmsgs  = $(addprefix $(LOCALEDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot   = $(PODIR)/$(PLUGIN).pot

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): $(wildcard *.cc)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --msgid-bugs-address='<see README>' -o $@ $^

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q $@ $<
	@touch $@

$(I18Nmsgs): $(LOCALEDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	@mkdir -p $(dir $@)
	cp $< $@

.PHONY: i18n
i18n: $(I18Nmsgs) $(I18Npot)

### Targets:

detector.a: force_look
	@cd detector; $(MAKE)
	

libvdr-$(PLUGIN).so: $(OBJS) detector.a
	$(CXX) $(CXXFLAGS) -shared $(LIBS) $(OBJS) detector.a  -o $@
	@cp --remove-destination $@ $(LIBDIR)/$@.$(APIVERSION)

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

clean:
	@-rm -f $(OBJS) $(TESTOBJS) $(DEPFILE) *.so *.tgz core* *~ $(PODIR)/*.mo $(PODIR)/*.pot 
	@cd detector; $(MAKE) clean
	
force_look :
	true
	
