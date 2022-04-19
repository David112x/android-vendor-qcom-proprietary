# Makefile.am - Automake script for diag
#
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = test klog mdlog PktRspTest uart_log dci_sample callback_sample socket_log
dist_doc_DATA =
docdir = doc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = diag-vendor.pc
EXTRA_DIST = $(pkgconfig_DATA)
