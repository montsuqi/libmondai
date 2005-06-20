lib_LTLIBRARIES = libmondai.la
noinst_PROGRAMS = testconv testvalue testmem 

libmondai_la_LDFLAGS	=	\
	-version-info $(LTVERSION)	\
	-export-dynamic

CFLAGS	=	-O2 -Wall -Wunused -Wredundant-decls -Werror

EXTRA_DIST = libmondai.h.in

BUILT_SOURCES = libmondai.h
libmondai.h: libmondai.h.in

libmondaiincludedir = $(includedir)/libmondai

libmondaiinclude_HEADERS =	\
	misc_v.h				\
	LBSfunc.h				\
	OpenCOBOL_v.h			\
	dotCOBOL_v.h			\
	Native_v.h				\
	Text_v.h				\
	XML_v.h					\
	cobolvalue.h			\
	others.h				\
	numerici.h				\
	fixed_v.h				\
	hash_v.h				\
	getset.h				\
	value.h					\
	valueconv.h				\
	libmondai.h				\
	Lex.h					\
	RecParser.h				\
	types.h					\
	monstring.h				\
	memory_v.h				\
	numeric.h

INCLUDES 			= 		\
	$(GLIB_CFLAGS) 			\
	$(XML_CFLAGS)			\
	-I$(top_srcdir)

libmondai_la_SOURCES	=	\
	misc.c					\
	LBSfunc.c				\
	cobolvalue.c			\
	OpenCOBOL.c				\
	dotCOBOL.c				\
	Native.c				\
	Text.c					\
	XML.c					\
	hash.c					\
	getset.c				\
	value.c					\
	valueconv.c				\
	fixed.c					\
	numerici.c				\
	others.c				\
	RecParser.c				\
	Lex.c					\
	memory.c				\
	monstring.c				\
	numeric.c				\
	debug.h

LDADD 				=		\
	$(GLIB_LIBS)			\
	$(XML_LIBS)				\
	$(top_builddir)/src/libmondai.la

testconv_INCLUDES 	= 		\
	$(GLIB_CFLAGS) 			\
	$(XML_CFLAGS)			\
	-I$(top_srcdir)

testconv_SOURCES	=		\
	testconv.c

testvalue_INCLUDES 	= 		\
	$(GLIB_CFLAGS) 			\
	$(XML_CFLAGS)			\
	-I$(top_srcdir)

testvalue_SOURCES	=		\
	testvalue.c

testmem_INCLUDES 	= 		\
	$(GLIB_CFLAGS) 			\
	-I$(top_srcdir)

testmem_SOURCES		=		\
	testmem.c
