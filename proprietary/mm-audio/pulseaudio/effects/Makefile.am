AM_CFLAGS = -Wundef \
        -Wunused-variable \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-short-enums \
        -fpic \
        -I .

h_sources = inc/pa_qti_effect.h \
            inc/pa_qti_effect_audiosphere.h \
            inc/pa_qti_effect_bassboost.h \
            inc/pa_qti_effect_equalizer.h \
            inc/pa_qti_effect_presetreverb.h \
            inc/pa_qti_effect_virtualizer.h \
            test/pa_qti_effect_test.h

lib_includedir = $(includedir)/mm-audio/pa_qti_effect
lib_include_HEADERS = $(h_sources)

header_includes = -I $(top_srcdir)/inc

lib_LTLIBRARIES = libpaqtieffect.la
libpaqtieffect_la_CC = @CC@
libpaqtieffect_la_SOURCES = src/pa_qti_effect.c
libpaqtieffect_la_CFLAGS = $(AM_CFLAGS)
libpaqtieffect_la_CFLAGS += $(GIO_CFLAGS)
libpaqtieffect_la_CFLAGS += $(header_includes)
libpaqtieffect_la_LDFLAGS = -shared -module -avoid-version $(GIO_LIBS)

lib_LTLIBRARIES += libpaqtieffectstestutils.la
libpaqtieffectstestutils_la_CC = @CC@
libpaqtieffectstestutils_la_SOURCES = test/pa_qti_effect_test_utils.c
libpaqtieffectstestutils_la_CFLAGS = $(AM_CFLAGS)
libpaqtieffectstestutils_la_CFLAGS += $(header_includes)
libpaqtieffectstestutils_la_LDFLAGS = -shared -module -avoid-version

bin_PROGRAMS = pa_qti_effect_test
pa_qti_effect_test_INCLUDES = $(header_includes)
pa_qti_effect_test_SOURCES = test/pa_qti_effect_test.c
pa_qti_effect_test_CFLAGS = $(AM_CFLAGS)
pa_qti_effect_test_CFLAGS += $(pa_qti_effect_test_INCLUDES)
pa_qti_effect_test_LDADD = libpaqtieffect.la libpaqtieffectstestutils.la
