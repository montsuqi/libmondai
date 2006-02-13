# libmondai -- MONTSUQI data access library
# Copyright (C) 2005 Ogochan.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

lib_LTLIBRARIES = libmondai.la

if SOAP
TESTSOAP  =  testsoap
else
TESTSOAP  =
endif
noinst_PROGRAMS = testconv testvalue testmem $(TESTSOAP)

libmondai_la_LDFLAGS	=	\
	-version-info $(LTVERSION)	\
	-export-dynamic

AM_CFLAGS	=	-O2 -Wall -Wunused -Wredundant-decls -Werror -funsigned-char

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
	SOAP_v.h				\
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
	SOAP.c					\
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

if SOAP
testsoap_INCLUDES 	= 		\
	$(GLIB_CFLAGS) 			\
	$(XML_CFLAGS)			\
	-I$(top_srcdir)

testsoap_SOURCES	=		\
	testsoap.c
endif

testmem_INCLUDES 	= 		\
	$(GLIB_CFLAGS) 			\
	-I$(top_srcdir)

testmem_SOURCES		=		\
	testmem.c
