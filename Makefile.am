@SET_MAKE@

EXTRA_DIST =								\
	AUTHORS								\
	BUGS								\
	COPYING								\
	ChangeLog 							\
	INSTALL								\
	NEWS								\
	README								\
	TODO								\
	xfce4-appfinder.spec

SUBDIRS =								\
	po								\
	doc								\
	src

distclean-local:
	-rm -rf *.cache *~

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

ACLOCAL_AMFLAGS = -I m4

dist-bz2: dist
	zcat $(PACKAGE)-$(VERSION).tar.gz | bzip2 --best -c > $(PACKAGE)-$(VERSION).tar.bz2

distcheck-bz2: distcheck
	zcat $(PACKAGE)-$(VERSION).tar.gz | bzip2 --best -c > $(PACKAGE)-$(VERSION).tar.bz2
