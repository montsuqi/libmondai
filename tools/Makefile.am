## Process this file with automake to produce Makefile.in

bin_PROGRAMS = rec2copy monconv

LDADD 		=	\
	$(GLIB_LIBS)						\
	$(XML_LIBS)						\
	$(top_builddir)/src/libmondai.la

INCLUDES 			= 					\
	-I$(top_srcdir)/src					\
	$(GLIB_CFLAGS)

DEPENDENCIES	=						\
	$(top_srcdir)/src/libmondai.la

rec2copy_SOURCES	=					\
	option.c	option.h				\
	rec2copy.c
monconv_SOURCES	=						\
	option.c	option.h				\
	monconv.c

EXTRA_DIST = option.doc
