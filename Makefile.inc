
### The C++ compiler and options:

CXX      ?= g++
CXXFLAGS ?= -g -O2 -Wall -Woverloaded-virtual -Wno-parentheses

### The directory environment:

VDRDIR = ../../..
LIBDIR = ../../lib
TMPDIR = /tmp

INCLUDES += -I$(VDRDIR)/include

DEFINES += -D_GNU_SOURCE -DPLUGIN_NAME_I18N='"$(PLUGIN)"'
# Uncomment to enable additional debug output
DEFINES += -DDEBUG

### The object files (add further files here):

# Options for dbus
LIBS = $(shell pkg-config --libs dbus-1)
CXXFLAGS += $(shell pkg-config --cflags dbus-1)
# Options for cdio
LIBS += $(shell pkg-config --libs libcdio_cdda)
CXXFLAGS += $(shell pkg-config --cflags libcdio_cdda)
# Options for DVDRead
LIBS += $(shell pkg-config --libs dvdread)
CXXFLAGS += $(shell pkg-config --cflags dvdread)

