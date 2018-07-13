# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
# IMPORTANT: the presence of this macro is important for the Make.config
# file. So it must be defined, even if it is not used here!
#
PLUGIN = autostart

### The directory environment:

LIBDIR = $(DESTDIR)$(call PKGCFG,libdir)
LOCDIR = $(DESTDIR)$(call PKGCFG,locdir)
PLGCFG = $(call PKGCFG,plgcfg)
TMPDIR ?= /tmp

export CFLAGS   = $(call PKGCFG,cflags)
export CXXFLAGS = $(call PKGCFG,cxxflags)
CXXFLAGS +=-std=c++11

### Allow user defined options to overwrite defaults:

-include $(PLGCFG)

### The version number of VDR's plugin API (taken from VDR's "config.h"):

APIVERSION = $(call PKGCFG,apiversion)

DEFINES += -DPLUGIN_NAME_I18N='"$(PLUGIN)"'
# Uncomment to enable additional debug output
DEFINES += -DDEBUG




