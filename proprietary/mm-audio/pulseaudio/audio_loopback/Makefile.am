AM_CFLAGS = -Wundef \
        -Werror \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-short-enums \
        -fpic \
        -I .

lib_includedir = $(includedir)/mm-audio/pa_qti_loopback
lib_include_HEADERS = $(top_srcdir)/pa_qti_loopback.h

header_includes = -I $(top_srcdir)/inc

lib_LTLIBRARIES = libpaqtiloopback.la
libpaqtiloopback_la_CC = @CC@
libpaqtiloopback_la_SOURCES = pa_qti_loopback.c
libpaqtiloopback_la_CFLAGS = $(AM_CFLAGS) $(header_includes) $(GIO_CFLAGS)
libpaqtiloopback_la_LDFLAGS = -shared -module -avoid-version $(GIO_LIBS)

bin_PROGRAMS = pa_qti_loopback_test
pa_qti_loopback_test_INCLUDES = $(header_includes) $(PA_QTI_EFFECT_CFLAGS)
pa_qti_loopback_test_SOURCES = pa_qti_loopback_test.c
pa_qti_loopback_test_CFLAGS = $(AM_CFLAGS) $(pa_qti_loopback_test_INCLUDES)
pa_qti_loopback_test_LDADD = libpaqtiloopback.la -lpaqtieffectstestutils -lpaqtieffect
