bin_PROGRAMS = xfce4-appfinder

xfce4_appfinder_SOURCES = 					\
	af-constants.h							\
	appfinder.h							\
	inline-icon.h							\
	callbacks.c							\
	callbacks.h							\
	main.c								\
	main.h

xfce4_appfinder_CFLAGS =						\
	-I$(top_srcdir)							\
	-DLOCALEDIR=\"$(localedir)\"					\
	@LIBXFCEGUI4_CFLAGS@

xfce4_appfinder_LDADD =							\
	@LIBXFCEGUI4_LIBS@

appsdir = $(datadir)/applications
apps_DATA = xfce4-appfinder.desktop

icondir = $(datadir)/pixmaps
icon_DATA =								\
	xfce4-appfinder.png

inline-icon.h: $(srcdir)/xfce4-appfinder.png
	gdk-pixbuf-csource --raw --build-list				\
	default_icon_data_48_48 xfce4-appfinder.png > $(srcdir)/inline-icon.h

EXTRA_DIST = $(icon_DATA) $(apps_DATA)


