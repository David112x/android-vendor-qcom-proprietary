pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = pa_qti_soundtrigger.pc
EXTRA_DIST = $(pkgconfig_DATA)

AM_CFLAGS = -Wundef \
        -Werror \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-short-enums \
        -fpic \
        -I .

lib_includedir = $(includedir)/mm-audio/pulseaudio/qti_soundtrigger/
lib_include_HEADERS = pa_qti_soundtrigger.h

lib_LTLIBRARIES = libpaqtisoundtrigger.la
libpaqtisoundtrigger_la_CC = @CC@
libpaqtisoundtrigger_la_SOURCES = pa_qti_soundtrigger.c
libpaqtisoundtrigger_la_CFLAGS = $(AM_CFLAGS) -DNDEBUG
libpaqtisoundtrigger_la_CFLAGS +=  $(GIO_CFLAGS)
libpaqtisoundtrigger_la_LDFLAGS = -shared -module -avoid-version
libpaqtisoundtrigger_la_LDFLAGS += $(GIO_LIBS)

###Generate test app ####
bin_PROGRAMS = pa_qti_soundtrigger_test
pa_qti_soundtrigger_test_SOURCES = pa_qti_soundtrigger_test.c
pa_qti_soundtrigger_test_CFLAGS = $(AM_CFLAGS) -D__unused=__attribute__\(\(__unused__\)\)
pa_qti_soundtrigger_test_CFLAGS += -Dstrlcpy=g_strlcpy $(GLIB_CFLAGS) -include glib.h
pa_qti_soundtrigger_test_LDADD = libpaqtisoundtrigger.la $(GLIB_LIBS)
