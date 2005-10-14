bin_PROGRAMS = xfce4-appfinder

xfce4_appfinder_SOURCES = 						\
	xfce4-appfinder.h                       			\
	xfce4-appfinder.c                       			\
	appfinder.h							\
	inline-icon.h							\
	main.c								\
	main.h

xfce4_appfinder_CFLAGS =						\
	-I$(top_srcdir)							\
	-DLOCALEDIR=\"$(localedir)\"					\
	@LIBXFCEGUI4_CFLAGS@

xfce4_appfinder_LDADD =							\
	@LIBXFCEGUI4_LIBS@

appsdir = $(datadir)/applications
apps_in_files = xfce4-appfinder.desktop.in
apps_DATA = $(apps_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

icondir = $(datadir)/pixmaps
icon_DATA =								\
	xfce4-appfinder.png

inline-icon.h: $(srcdir)/xfce4-appfinder.png
	gdk-pixbuf-csource --raw --build-list				\
	default_icon_data_48_48 xfce4-appfinder.png > $(srcdir)/inline-icon.h

EXTRA_DIST = $(icon_DATA) $(apps_DATA)

DISTCLEANFILES = $(apps_DATA)



