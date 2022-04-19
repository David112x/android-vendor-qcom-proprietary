AM_CFLAGS = \
     $(GPSUTILS_CFLAGS) \
     $(DIAG_CFLAGS) \
     -I./ \
     -D__STDC_FORMAT_MACROS \
     -Dstrlcpy=g_strlcpy \
     -Dstrlcat=g_strlcat \
     -std=c++11

ACLOCAL_AMFLAGS = -I m4

liblocdiagiface_h_sources = \
    LocDiagIface.h

liblocdiagiface_c_sources = \
    LocDiagIface.cpp

liblocdiagiface_la_SOURCES = \
    $(liblocdiagiface_h_sources) \
    $(liblocdiagiface_c_sources)


if USE_GLIB
liblocdiagiface_la_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@ -include glib.h
liblocdiagiface_la_LDFLAGS = -lstdc++ -g -Wl,-z,defs -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
liblocdiagiface_la_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@ -include glib.h
else
liblocdiagiface_la_CFLAGS = $(AM_CFLAGS)
liblocdiagiface_la_LDFLAGS = -Wl,-z,defs -lpthread -shared -version-info 1:0:0
liblocdiagiface_la_CPPFLAGS = $(AM_CFLAGS) $(AM_CPPFLAGS)
endif

liblocdiagiface_la_LIBADD = -ldl -lstdc++ $(DIAG_LIBS) $(GPSUTILS_LIBS)
library_include_HEADERS = \
    LocDiagIface.h

#Create and Install libraries
lib_LTLIBRARIES = liblocdiagiface.la
library_includedir = $(pkgincludedir)
library_include_HEADERS = $(liblocdiagiface_h_sources)
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = loc-diag-iface.pc
EXTRA_DIST = $(pkgconfig_DATA)

