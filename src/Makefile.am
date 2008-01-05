bin_PROGRAMS = xfce4-appfinder

xfce4_appfinder_SOURCES = \
	main.c \
	appfinder.c \
	appfinder.h

xfce4_appfinder_CFLAGS = \
	-I$(top_srcdir) \
	-DBINDIR=\"$(bindir)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	-DDATADIR=\"$(datadir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	$(LIBX11_CFLAGS)	\
	$(GTHREAD_CFLAGS) \
	$(LIBXFCE4MENU_CFLAGS)

xfce4_appfinder_LDFLAGS = \
	-export-dynamic

xfce4_appfinder_LDADD = \
	$(LIBX11_LDFLAGS) \
	$(LIBX11_LIBS) \
	$(GTHREAD_LIBS) \
	$(LIBXFCE4MENU_LIBS)

