AM_CFLAGS = \
     -I./ \
     -I../api \
     -D__STDC_FORMAT_MACROS \
     -Dstrlcpy=g_strlcpy \
     -Dstrlcat=g_strlcat \
     -std=c++14

if MULTIBAND_SUPPORTED
AM_CFLAGS += -DMULTIBAND_SUPPORTED
endif

ACLOCAL_AMFLAGS = -I m4

libep_ref_simulator_la_SOURCES = \
    EnginePluginAPISimulator.cpp


if USE_GLIB
libep_ref_simulator_la_CFLAGS = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@ -include glib.h
libep_ref_simulator_la_LDFLAGS = -lstdc++ -Wl,-z,defs -lpthread @GLIB_LIBS@ -shared -version-info 1:0:0
libep_ref_simulator_la_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@ -include glib.h
endif

libep_ref_simulator_la_LIBADD = -ldl -lstdc++

library_includedir = $(pkgincludedir)
#Create and Install libraries
lib_LTLIBRARIES = libep_ref_simulator.la

