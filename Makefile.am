SUBDIRS = 					\
	src					\
	tools 					\
	docs

bin_SCRIPTS = 					\
	libmondai-config

man_MANS = 					\
	libmondai-config.1

EXTRA_DIST =					\
	libmondai.m4				\
	libmondai-config.1.in			\
	libmondai-config.in

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-`date +"%Y%m%d"`

install-data-local:
	-mkdir -p $(DESTDIR)$(datadir)/aclocal
	$(INSTALL_DATA) $(top_srcdir)/libmondai.m4 $(DESTDIR)$(datadir)/aclocal/libmondai.m4
