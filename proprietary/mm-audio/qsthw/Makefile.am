if QSTHW
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = qsthw.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-short-enums \
        -fpic \
        -I .

AM_CFLAGS += -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/sound_trigger
if USE_KEEP_ALIVE
AM_CFLAGS += -DUSE_KEEP_ALIVE
endif
if SVA_AUDIO_CONCURRENCY
AM_CFLAGS += -DSVA_AUDIO_CONC
endif
lib_includedir = $(includedir)/mm-audio/qsthw
lib_include_HEADERS = qsthw.h qsthw_defs.h

lib_LTLIBRARIES = libqtisoundtriggerwrapper.la
libqtisoundtriggerwrapper_la_CC = @CC@
libqtisoundtriggerwrapper_la_SOURCES = qsthw.c
libqtisoundtriggerwrapper_la_LIBADD = -ltinyalsa -lhardware -lexpat -lcutils -llog -ldl
libqtisoundtriggerwrapper_la_CFLAGS = $(AM_CFLAGS)
libqtisoundtriggerwrapper_la_CFLAGS += -DNDEBUG
libqtisoundtriggerwrapper_la_LDFLAGS = -shared -module -avoid-version

endif
