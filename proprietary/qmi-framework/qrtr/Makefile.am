AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = \
        -I./lib \
        -I./src

h_sources = lib/libqrtr.h
h_sources += src/ns.h

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

bin_PROGRAMS = qrtr-cfg
qrtr_cfg_SOURCES = src/cfg.c $(h_sources)
qrtr_cfg_CC = @CC@

bin_PROGRAMS += qrtr-ns
qrtr_ns_SOURCES = src/ns.c src/map.c src/hash.c src/waiter.c src/util.c $(h_sources)
qrtr_ns_CC = @CC@

bin_PROGRAMS += qrtr-lookup
qrtr_lookup_SOURCES = src/util.c src/lookup.c $(h_sources)
qrtr_lookup_CC = @CC@

lib_LTLIBRARIES = libqrtr.la
libqrtr_la_CC = @CC@
libqrtr_la_SOURCES = lib/qrtr.c lib/qmi.c $(h_sources)
libqrtr_la_CFLAGS = $(AM_CFLAGS)
libqrtr_la_CPPFLAGS = $(AM_CPPFLAGS)
libqrtr_la_LDFLAGS = -shared -version-info 1:0:0
