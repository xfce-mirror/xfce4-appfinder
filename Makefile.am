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
