SUBDIRS = 								\
  	 images

noinst_DATA = 								\
	xfce4-appfinder.xml 

docdir=$(datadir)/xfce4/doc/fr

doc_DATA= 								\
	xfce4-appfinder.html 

EXTRA_DIST = $(noinst_DATA) $(doc_DATA)

html: $(srcdir)/xfce4-appfinder.xml
	xsltproc $(srcdir)/../xfce-nochunk.xsl $(srcdir)/xfce4-appfinder.xml

