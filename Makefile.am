@SET_MAKE@

SUBDIRS = src data po

EXTRA_DIST = \
	TODO \
	xfce4-appfinder.spec \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

DISTCLEANFILES = \
	xfce4-appfinder.spec \
	intltool-extract \
	intltool-merge \
	intltool-update

AUTOMAKE_OPTIONS = \
	1.8 \
	dist-bzip2

distclean-local:
	-rm -rf *.cache *~

rpm: distcheck
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz
