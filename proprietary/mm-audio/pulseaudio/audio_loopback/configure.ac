#                                               -*- Autoconf -*-
# configure.ac -- Autoconf script for pulseaudio client
#

# Process this file with autoconf to produce a configure script.

# Requires autoconf tool later than 2.61
AC_PREREQ([2.69])
# Initialize the hal-interface package version 1.0.0
AC_INIT([pa_qti_loopback],1.0.0)
# Does not strictly follow GNU Coding standards
AM_INIT_AUTOMAKE([foreign])
# Disables auto rebuilding of configure, Makefile.ins
#AM_MAINTAINER_MODE
# defines some macros variable to be included by source
AC_CONFIG_HEADERS([config.h])
# defines some macros variable to be included by source
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AM_PROG_AS
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([GIO], [gio-2.0])
AC_SUBST([GIO_CFLAGS])
AC_SUBST([GIO_LIBS])

AC_CONFIG_FILES([ \
        Makefile
        ])

AC_ARG_WITH([pa-qti-effect],
      AC_HELP_STRING([--with-pa-qti-effect],
         [Include effect, Include effect header files when compiling loopback module]))

PA_QTI_EFFECT_CFLAGS=
AS_IF([test "x$with_pa_qti_effect" != "xno"], [PA_QTI_EFFECT_CFLAGS="-I$with_pa_qti_effect"])

AC_SUBST(PA_QTI_EFFECT_CFLAGS)

AC_OUTPUT
