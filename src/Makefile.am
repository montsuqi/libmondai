lib_LTLIBRARIES = libmondai.la

libmondai_la_LDFLAGS	=	\
	-version-info $(LTVERSION)	\
	-export-dynamic

libmondaiincludedir = $(includedir)/libmondai
libmondaiinclude_HEADERS =	\
	misc.h			\
	LBSfunc.h		\
	OpenCOBOL_v.h	\
	dotCOBOL_v.h	\
	Native_v.h		\
	cobolvalue.h	\
	value.h			\
	valueconv.h		\
	libmondai.h		\
	SQLparser.h		\
	DDparser.h		\
	types.h

INCLUDES 			= 		\
	-I$(top_srcdir)			\
	$(GLIB_CFLAGS) \
	$(FIXED_CFLAGS)

libmondai_la_SOURCES	=		\
	misc.c			misc.h			\
	LBSfunc.c		LBSfunc.h		\
	cobolvalue.c	cobolvalue.h	\
	OpenCOBOL.c		OpenCOBOL_v.h	\
	dotCOBOL.c		dotCOBOL_v.h	\
	Native.c		Native_v.h		\
	value.c			value.h			\
	valueconv.c		valueconv.h		\
	SQLlex.c		SQLlex.h		\
	SQLparser.c		SQLparser.h		\
	DDparser.c		DDparser.h		\
	DDlex.c			DDlex.h			\
	types.h							\
	debug.h
