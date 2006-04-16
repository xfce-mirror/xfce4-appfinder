# $Id$

EXTRA_DIST =								\
	intltool-extract.in						\
	intltool-merge.in						\
	intltool-update.in						\
	AUTHORS								\
	BUGS								\
	COPYING								\
	ChangeLog 							\
	INSTALL								\
	NEWS								\
	README								\
	TODO								\
	xfce4-appfinder.spec

DISTCLEANFILES =							\
	intltool-extract						\
	intltool-merge							\
	intltool-update							\
	xfce4-appfinder.spec
	
SUBDIRS =								\
	src								\
	doc								\
	po

AUTOMAKE_OPTIONS =							\
	1.8								\
	dist-bzip2

distclean-local:
	-rm -rf *.cache *~

rpm: dist
	rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz
	@rm -f $(PACKAGE)-$(VERSION).tar.gz

# vi:set ts=8 sw=8 noet ai nocindent:
