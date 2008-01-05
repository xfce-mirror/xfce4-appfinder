appsdir = $(datadir)/applications
apps_in_files = xfce4-appfinder.desktop.in
apps_DATA = $(apps_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = xfce4-appfinder.png

EXTRA_DIST = $(icon_DATA) $(apps_in_files)
DISTCLEANFILES = $(apps_DATA)

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(datadir)/icons/hicolor

install-data-hook:
	@-if test -z "$(DESTDIR)"; then                                 \
                echo "Updating Gtk icon cache.";                        \
                $(gtk_update_icon_cache);                               \
        else                                                            \
                echo "*** Icon cache not updated. Remember to run:";    \
                echo "***";                                             \
                echo "***   $(gtk_update_icon_cache)";                  \
                echo "***";                                             \
        fi

# vi:set ts=8 sw=8 noet ai nocindent:
