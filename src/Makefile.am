bin_PROGRAMS = xfce4-appfinder

xfce4_appfinder_SOURCES = 						\
	main.c								\
	main.h

xfce4_appfinder_CFLAGS =						\
	-I$(top_srcdir)							\
	-DLOCALEDIR=\"$(localedir)\"					\
	@LIBXFCEGUI4_CFLAGS@

xfce4_appfinder_LDADD =							\
	@LIBXFCEGUI4_LIBS@

EXTRA_DIST =								\
	inline-icon.h							\
	xfce4-appfinder.png

noinst_DATA =								\
	inline-icon.h							\
	xfce4-appfinder.png

inline-icon.h: $(srcdir)/xfce4-appfinder.png
	gdk-pixbuf-csource --raw --build-list				\
	default_icon_data_48_48 xfce4-appfinder.png > $(srcdir)/inline-icon.h
