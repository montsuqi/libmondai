lib_LTLIBRARIES = libmondai.la
noinst_PROGRAMS = testconv

libmondai_la_LDFLAGS	=	\
	-version-info $(LTVERSION)	\
	-export-dynamic

CONFIG_CLEAN_FILES =  libmondai.h

libmondaiincludedir = $(includedir)/libmondai

libmondaiinclude_HEADERS =	\
	misc.h			\
	LBSfunc.h		\
	OpenCOBOL_v.h	\
	dotCOBOL_v.h	\
	Native_v.h		\
	Text_v.h		\
	cobolvalue.h	\
	others.h		\
	numerici.h		\
	hash.h			\
	getset.h		\
	value.h			\
	valueconv.h		\
	libmondai.h		\
	SQLparser.h		\
	DDparser.h		\
	monstring.h		\
	types.h

INCLUDES 			= 		\
	$(GLIB_CFLAGS) 			\
	$(FIXED_CFLAGS)			\
	-I$(top_srcdir)

libmondai_la_SOURCES	=			\
	monstring.c		monstring.h		\
	misc.c			misc.h			\
	LBSfunc.c		LBSfunc.h		\
	cobolvalue.c	cobolvalue.h	\
	OpenCOBOL.c		OpenCOBOL_v.h	\
	dotCOBOL.c		dotCOBOL_v.h	\
	Native.c		Native_v.h		\
	Text.c			Text_v.h		\
	hash.c			hash.h			\
	getset.c		getset.h		\
	value.c			value.h			\
	valueconv.c		valueconv.h		\
	numerici.c		numerici.h		\
	others.c		others.h		\
	SQLlex.c		SQLlex.h		\
	SQLparser.c		SQLparser.h		\
	DDparser.c		DDparser.h		\
	DDlex.c			DDlex.h			\
	types.h							\
	debug.h

testconv_LDADD 		=	\
	$(GLIB_LIBS)		\
	$(FIXED_LIBS)		\
	./libmondai.la

testconv_SOURCES	=				\
	testconv.c
