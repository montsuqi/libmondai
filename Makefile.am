SUBDIRS = 					\
	src					\
	tools 					\
	docs

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmondai.pc

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-$(VERSION)~pre`date +"%y%m%d"`

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = BSDL GPL2
