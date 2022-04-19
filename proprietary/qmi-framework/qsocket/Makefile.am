AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic

AM_CPPFLAGS = \
        -I./inc

h_sources = \
         inc/msm_ipc.h \
         inc/qsocket.h \
         inc/qsocket_ipcr.h

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

lib_LTLIBRARIES = libqsocket.la
libqsocket_la_CC = @CC@
libqsocket_la_SOURCES = src/qsocket_ipcr.c $(h_sources)
libqsocket_la_CFLAGS = $(AM_CFLAGS)
libqsocket_la_CPPFLAGS = $(AM_CPPFLAGS)
libqsocket_la_LDFLAGS = -shared -version-info 1:0:0