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

files:
        @files=`ls $(DISTFILES) 2> /dev/null `; for p in $$files; do \
          echo $$p; \
        done
        @for subdir in $(SUBDIRS); do \
          files=`cd $$subdir; $(MAKE) files | grep -v "make\[[1-9]\]"`; \
          for file in $$files; do \
            echo $$subdir/$$file; \
          done; \
        done
release:
	rm -rf .deps */.deps
	$(MAKE) distcheck

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)`date +"%y%m%d"`

