AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic \
	-DQMI_CCI_ANDROID

if USE_QMUX
AM_CFLAGS += -DQCCI_OVER_QMUX
endif

AM_CPPFLAGS = \
        -I../../inc \
        -I../inc \
        -I../../common/inc \
        -I../../qsocket/inc \
        $(QMI_CFLAGS) \
        $(QMIFV_CFLAGS)

h_sources = \
        ../inc/qmi_cci_target.h \
        ../inc/qmi_cci_common.h \
        ../../inc/common_v01.h \
        ../../inc/qmi_cci_target_ext.h \
        ../../inc/qmi_client.h \
        ../../inc/qmi_client_deprecated.h \
        ../../inc/qmi_csi.h \
        ../../inc/qmi_csi_target_ext.h \
        ../../inc/qmi_idl_lib.h \
        ../../inc/qmi_idl_lib_internal.h \
        ../../inc/qmi_idl_lib_target.h \
        ../../inc/qmi_client_instance_defs.h

requiredlibs = -lpthread \
        ../../encdec/libqmi_encdec.la

if USE_QMUX
requiredlibs += -lqmi_client_qmux
endif

library_includedir = $(pkgincludedir)
library_include_HEADERS = $(h_sources)

c_sources = qmi_cci_common.c \
        qmi_cci_target.c \
        qmi_cci_xport_ipc_router.c \
	qmi_cci_xport_qrtr.c \
        ../../common/src/common_v01.c

if USE_QMUX
c_sources += qmi_cci_xport_qmuxd.c
endif

lib_LTLIBRARIES = libqmi_cci.la
libqmi_cci_la_CC = @CC@
libqmi_cci_la_SOURCES = $(c_sources) $(h_sources)
libqmi_cci_la_CFLAGS = $(AM_CFLAGS)
libqmi_cci_la_CPPFLAGS = $(AM_CPPFLAGS)
libqmi_cci_la_LDFLAGS = -shared -version-info 1:0:0
libqmi_cci_la_LIBADD = $(requiredlibs)
