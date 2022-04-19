pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = esp.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g \
        -fpic \
        -I ./lib/inc/ \
        -I ./lib/src/

cpp_sources = lib/src/esp_api.cpp

c_sources = lib/src/mems_utils.c \
            lib/src/mutex_cond_posix.c \
            lib/src/parallel_task_queue.c

lib_includedir = $(includedir)/esp
lib_include_HEADERS = lib/inc/esp_api.h

lib_LTLIBRARIES = libesp.la
libesp_la_CC = @CC@
libesp_la_SOURCES = $(c_sources) $(cpp_sources)
libesp_la_CFLAGS = $(AM_CFLAGS) -O3 -fasm -mfpu=neon -ftree-vectorize -funroll-loops -D_ENABLE_ARM_OPTS_ -D_ENABLE_ARM_BASIC_OPTS_ -D_SUPPRESS_ARM_OPTS_WARNING -D__MULTI_THREAD__
libesp_la_CPPFLAGS = $(AM_CFLAGS) -O3 -fasm -mfpu=neon -ftree-vectorize -funroll-loops -D_ENABLE_ARM_OPTS_ -D_ENABLE_ARM_BASIC_OPTS_ -D_SUPPRESS_ARM_OPTS_WARNING -D__MULTI_THREAD__
libesp_la_LDFLAGS = -shared -module -avoid-version
