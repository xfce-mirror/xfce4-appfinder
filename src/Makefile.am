# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

INCLUDES = \
	-I$(top_srcdir) \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	-DG_LOG_DOMAIN=\"xfce4-appfinder\" \
	$(PLATFORM_CPPFLAGS)

bin_PROGRAMS = xfce4-appfinder

xfce4_appfinder_SOURCES = \
	appfinder-category-model.c \
	appfinder-category-model.h \
	appfinder-model.c \
	appfinder-model.h \
	appfinder-window.c \
	appfinder-window.h \
	main.c

xfce4_appfinder_CFLAGS = \
	$(GTHREAD_CFLAGS) \
	$(GTK_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(GARCON_CFLAGS) \
	$(PLATFORM_CFLAGS)

xfce4_appfinder_LDADD = \
	$(GLIB_LIBS) \
	$(GTHREAD_LIBS) \
	$(GTK_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(GARCON_LIBS) \
	$(LIBXFCONF_LIBS)

xfce4_appfinder_LDFLAGS = \
	-no-undefined \
	$(PLATFORM_LDFLAGS)
