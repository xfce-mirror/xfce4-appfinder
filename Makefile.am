@SET_MAKE@

EXTRA_DIST = xfce4-appfinder.spec TODO ChangeLog.old

SUBDIRS = src

distclean-local:
	-rm -rf *.cache *~

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

ACLOCAL_AMFLAGS = -I m4
