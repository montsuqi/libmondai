## Process this file with automake to produce Makefile.in

bin_PROGRAMS = rec2copy

LDADD 		=	\
	$(GLIB_LIBS)		\
	$(FIXED_LIBS)		\
	-L$(top_builddir)/src -lmondai

INCLUDES 			= 		\
	-I$(top_srcdir)/src		\
	$(GLIB_CFLAGS)			\
	$(FIXED_CFLAGS)

rec2copy_SOURCES	=	\
	rec2copy.c

