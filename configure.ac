dnl configure.ac
dnl

AC_INIT([src/main.c])

AM_INIT_AUTOMAKE([xfce4-appfinder], [4.2.2])

AM_CONFIG_HEADER([config.h])

AM_MAINTAINER_MODE

dnl check for UNIX variants
AC_AIX
AC_ISC_POSIX
AC_MINIX

dnl check for basic programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

dnl check for standard header files
AC_HEADER_STDC

dnl Check for i18n support
BM_I18N([xfce4-appfinder], [af be bn_IN ca cs da de el en_GB es es_MX et eu
			    fa fi gu he hu hy it ja ko lt mr nl pa pt_BR pt_PT
			    ro ru sk tr uk zh_CN])

dnl Check for required packages
BM_DEPEND([LIBXFCEGUI4], [libxfcegui4-1.0], [4.2.0])

dnl Check for debugging support
BM_DEBUG_SUPPORT

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

