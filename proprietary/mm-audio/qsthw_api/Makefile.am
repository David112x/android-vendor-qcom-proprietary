if QSTHW_API
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = qsthw_api.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CPPFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-short-enums \
        -fpic \
        -I .

AM_CPPFLAGS += -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/mm-audio/qsthw
AM_CPPFLAGS += -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/mm-audio/qti-audio-server
AM_CPPFLAGS += -I $(PKG_CONFIG_SYSROOT_DIR)/usr/include/sound_trigger
AM_CPPFLAGS += -D__unused=__attribute__\(\(__unused__\)\)

lib_includedir = $(includedir)/mm-audio/qsthw_api
lib_include_HEADERS = qsthw_api.h qsthw_defs.h

lib_LTLIBRARIES = libqtisoundtriggerhw.la
libqtisoundtriggerhw_la_CC = @CC@
libqtisoundtriggerhw_la_SOURCES = qsthw_api.cpp
libqtisoundtriggerhw_la_LIBADD = -ltinyalsa -lhardware -lexpat -lcutils -llog -ldl -lqtisoundtriggerwrapper
libqtisoundtriggerhw_la_CPPFLAGS = $(AM_CPPFLAGS)
libqtisoundtriggerhw_la_CPPFLAGS += -DNDEBUG
libqtisoundtriggerhw_la_CPPFLAGS += -std=c++11 -DHAVE_PTHREADS -DHAVE_ANDROID_OS
libqtisoundtriggerhw_la_CPPFLAGS += -DDEBUG_REFS_CALLSTACK_ENABLED=0
libqtisoundtriggerhw_la_LDFLAGS = -shared -module -avoid-version -lpthread -lutils

if QTI_AUDIO_SERVER_ENABLED
AM_CPPFLAGS += -DQTI_AUDIO_SERVER_ENABLED
libqtisoundtriggerhw_la_LDFLAGS +=  -lqtiaudioserver -lbinder
endif

###Generate test app ####
bin_PROGRAMS = qti_sound_trigger_test
qti_sound_trigger_test_SOURCES = qsthw_test.c
qti_sound_trigger_test_CFLAGS = $(AM_CPPFLAGS) -D__unused=__attribute__\(\(__unused__\)\) -DLINUX_ENABLED
qti_sound_trigger_test_LDADD = -lcutils libqtisoundtriggerhw.la -lpthread
endif
