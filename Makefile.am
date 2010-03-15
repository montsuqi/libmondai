SUBDIRS = 					\
	src					\
	tools 					\
	docs

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmondai.pc

EXTRA_DIST =					\
	libmondai.m4

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-`date +"%Y%m%d"`

install-data-local:
	-mkdir -p $(DESTDIR)$(datadir)/aclocal
	$(INSTALL_DATA) $(top_srcdir)/libmondai.m4 $(DESTDIR)$(datadir)/aclocal/libmondai.m4
