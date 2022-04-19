AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = -I../inc \
              -I. \
              $(QMIF_CFLAGS) \
              $(ACDBLOADER_CFLAGS)

AM_INCLUDES = -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/mm-audio/qahw_api/inc

c_sources = core_sound_driver_v01.c csd_server.c csd_hal.c

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

bin_PROGRAMS = csd_server
csd_server_CC = @CC@

csd_server_SOURCES = $(c_sources) $(h_sources)
csd_server_CFLAGS = $(AM_CFLAGS) $(AM_INCLUDES) -DUSE_GLIB @GLIB_CFLAGS@
csd_server_CPPFLAGS = $(AM_CPPFLAGS) $(AM_INCLUDES) -DUSE_GLIB @GLIB_CFLAGS@
csd_server_LDFLAGS = -lpthread  $(ACDBLOADER_LIBS) $(QMIF_LIBS) @GLIB_LIBS@ -shared -version-info 1:0:0
csd_server_LDADD = -lqahw
