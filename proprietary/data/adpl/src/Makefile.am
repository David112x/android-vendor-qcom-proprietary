AM_CFLAGS = -Wundef \
        -Wstrict-prototypes \
        -Wno-trigraphs \
        -g -O0 \
        -fno-inline \
        -fno-short-enums \
        -fpic


AM_CPPFLAGS = -D__packed__= \
        -DIMAGE_APPS_PROC \
        -DFEATURE_Q_NO_SELF_QPTR \
        -DAEEINTERFACE_CPLUSPLUS \
        -DFEATURE_SMD \
        -DFEATURE_DIAG_MP_MASTER_APPS \
        -DFEATURE_DS_LINUX_ANDROID \
        -DFEATURE_DSS_LINUX \
        -DFEATURE_LINUX \
        -DFEATURE_MULTIPROCESSOR \
        -DFEATURE_DSUTILS_OFFTARGET \
        -DFEATURE_NO_REX \
        -DFEATURE_XMLLIB \
        -DFEATURE_MDM_LE \
        -I$(top_srcdir)/mobileap/ipc \
        -I$(WORKSPACE)/qmi/services \
        -I$(top_srcdir)/dsutils/inc \
        -I$(top_srcdir)/netmgr/inc \
        -I$(WORKSPACE)/qmi-framework/inc \
        -I$(WORKSPACE)/qmi-framework/qcci/inc \
        $(QMI_CFLAGS) \
        $(DIAG_CFLAGS) \
        $(DSUTILS_CFLAGS) \
        $(XMLLIB_CFLAGS) \
        $(QMIFRAMEWORK_CFLAGS) \
        $(DATAOSS_CFLAGS) \
        -I$(WORKSPACE)/common/inc

requiredlibs = -lpthread -lrt \
         ../../netmgr/src/libnetmgr.la \
         ../../mobileap/ipc/libqcmapipc.la \
         ../../mobileap/ipc/libqcmaputils.la

library_includedir = $(pkgincludedir)

adpl_sources = adpl_main.c \
              adpl_cmdq.c \
              adpl_peripheral.c \
              adpl_modem.c \
              adpl_dpm.c \
              adpl_data.c \
              adpl_qmi.c

adpl_h_sources = adpl.h \
                adpl_cmdq.h


init_ddir = ${sysconfdir}/init.d
init_d_SCRIPTS = start_adpl_le

bin_PROGRAMS = adpl
adpl_CC = @CC@
adpl_SOURCES = $(adpl_sources) $(adpl_h_sources)
if USE_GLIB
adpl_CFLAGS  = -DUSE_GLIB $(AM_CFLAGS) @GLIB_CFLAGS@
adpl_LDFLAGS  = $(DIAG_LIBS) $(XMLLIB_LIBS) $(DATAOSS_LIBS) -lpthread -lqmi_cci -lqmiservices @GLIB_LIBS@ -shared -version-info 1:0:0
adpl_CPPFLAGS = -DUSE_GLIB $(AM_CFLAGS) $(AM_CPPFLAGS) @GLIB_CFLAGS@
else
adpl_CFLAGS = $(AM_CFLAGS) $(DIAG_CFLAGS) -fPIC
adpl_LDFLAGS = -lpthread $(DIAG_LIBS) $(XMLLIB_LIBS) $(DATAOSS_LIBS) -lqmi_cci -lqmiservices -shared -version-info 1:0:0
adpl_CPPFLAGS = $(AM_CPPFLAGS)
endif
adpl_CPPFLAGS = $(AM_CPPFLAGS)
adpl_LDADD = $(requiredlibs)
