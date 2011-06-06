
appsdir = $(datadir)/applications
apps_in_files = \
	xfce4-appfinder.desktop.in \
	xfce4-run.desktop.in
apps_DATA = $(apps_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(apps_in_files)

DISTCLEANFILES = \
	$(apps_DATA)

# vi:set ts=8 sw=8 noet ai nocindent:
