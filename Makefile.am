## Makefile.am for libfixed

SRC_SUBDIRS = src docs
SUBDIRS = $(SRC_SUBDIRS)

bin_SCRIPTS = libmondai-config

# require automake 1.4
AUTOMAKE_OPTIONS = 1.4

EXTRA_DIST = \
	TODO \
	acconfig.h \
	makecopyright

release:
	rm -rf .deps */.deps
	$(MAKE) distcheck

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)`date +"%y%m%d"`

