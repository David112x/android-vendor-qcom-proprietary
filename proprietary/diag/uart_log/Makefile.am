
AM_CFLAGS = -Wall \
		-Wundef \
		-Wstrict-prototypes \
		-Wno-trigraphs \
		-DUSE_LOGCAT_LOGGING

AM_CPPFLAGS = -D__packed__= \
		-DIMAGE_APPS_PROC \
		-DFEATURE_Q_SINGLE_LINK \
		-DFEATURE_Q_NO_SELF_QPTR \
		-DFEATURE_LINUX \
		-DFEATURE_NATIVELINUX \
		-DFEATURE_DSM_DUP_ITEMS \
		-DFEATURE_LE_DIAG \
		$(DIAG_CFLAGS)

bin_PROGRAMS = diag_uart_log

diag_uart_log_SOURCES = diag_uart_log.c
if USE_GLIB
diag_uart_log_CFLAGS =  -DUSE_GLIB @GLIB_CFLAGS@ $(DIAG_CFLAGS)
diag_uart_log_LDFLAGS = -lpthread @GLIB_LIBS@ -llog $(DIAG_LIBS)
else
diag_uart_log_LDFLAGS = -lpthread -lcutils -llog $(DIAG_LIBS)
endif
