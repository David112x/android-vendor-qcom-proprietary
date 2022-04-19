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

bin_PROGRAMS = test_diag

test_diag_SOURCES = test_diag.c
test_diag_LDFLAGS = -lpthread -llog $(DIAG_LIBS)
