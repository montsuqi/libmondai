## Makefile.am for libfixed

SRC_SUBDIRS = src tools docs
SUBDIRS = $(SRC_SUBDIRS)

bin_SCRIPTS = libmondai-config

# require automake 1.4
AUTOMAKE_OPTIONS = 1.4

EXTRA_DIST = \
	TODO \
	libmondai.m4

release:
	rm -rf .deps */.deps
	$(MAKE) distcheck

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)`date +"%y%m%d"`

install-data-local:
	-mkdir -p $(DESTDIR)$(datadir)/aclocal
	$(INSTALL_DATA) $(srcdir)/libmondai.m4 $(DESTDIR)$(datadir)/aclocal/libmondai.m4
