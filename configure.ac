dnl configure.ac
dnl

m4_define([appfinder_version],[4.3.2])

AC_INIT([xfce4-appfinder], [appfinder_version()], 
	[xfce4-dev@xfce.org])

APPFINDER_VERSION=appfinder_version()
AM_INIT_AUTOMAKE([xfce4-appfinder], [$APPFINDER_VERSION])
AM_CONFIG_HEADER([config.h])

AM_MAINTAINER_MODE

dnl check for UNIX variants
AC_AIX
AC_ISC_POSIX
AC_MINIX

dnl check for basic programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_INTLTOOL

dnl check for standard header files
AC_HEADER_STDC

dnl Check for i18n support
XDT_I18N([be bn_IN ca cs de el en_GB es es_MX et eu fa fi fr gu he hy it
	  ja ko lt mr nl pa pl pt_BR pt_PT ro ru sv tr uk zh_CN zh_TW])

dnl Check for required packages
XDT_CHECK_PACKAGE([LIBXFCEGUI4], [libxfcegui4-1.0], [4.3.6])

dnl Check for debugging support
XDT_FEATURE_DEBUG()

AC_OUTPUT([
xfce4-appfinder.spec
Makefile
po/Makefile.in
src/Makefile
doc/Makefile
doc/C/Makefile
doc/C/images/Makefile
doc/fr/Makefile
doc/fr/images/Makefile
])

