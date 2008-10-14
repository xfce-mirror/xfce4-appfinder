# $Id$
# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

bin_PROGRAMS =								\
	xfce4-appfinder

xfce4_appfinder_SOURCES =						\
	main.c								\
	xfce-appfinder-window.h						\
	xfce-appfinder-window.c						\
	frap-icon-entry.h						\
	frap-icon-entry.c

xfce4_appfinder_CFLAGS = 						\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DDATADIR=\"$(datadir)\"					\
	-DG_LOG_DOMAIN=\"xfce4-appfinder\"				\
	$(PLATFORM_CFLAGS)						\
	$(GLIB_CFLAGS)							\
	$(GTK_CFLAGS)							\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(LIBXFCEGUI4_CFLAGS)						\
	$(LIBXFCE4MENU_CFLAGS)						\
	$(LIBXFCONF_CFLAGS)						\
	$(THUNAR_VFS_CFLAGS)

xfce4_appfinder_LDFLAGS = 						\
	$(PLATFORM_LIBS)						\
	$(GLIB_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GTK_LIBS)							\
	$(LIBXFCE4UTIL_LIBS)						\
	$(LIBXFCEGUI4_LIBS)						\
	$(LIBXFCE4MENU_LIBS)						\
	$(LIBXFCONF_LIBS)							\
	$(THUNAR_VFS_LIBS)
