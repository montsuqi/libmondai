SUBDIRS = 					\
	src					\
	tools 					\
	docs

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libmondai.pc

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-`date +"%Y%m%d"`

ACLOCAL_AMFLAGS = -I m4
