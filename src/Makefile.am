# libmondai -- MONTSUQI data access library
# Copyright (C) 2005-2010 Ogochan.
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

noinst_PROGRAMS =	\
	testfileutils \
	testfixed \
	testconv \
	testvalue \
	testmem \
	testNativePack \
	testjson \
	testnumeric \
	testOpenCOBOLPack

libmondai_la_LDFLAGS	=	\
	-version-info $(LTVERSION)	\
	-export-dynamic

AM_CFLAGS	=	-Wall -Wunused -Wredundant-decls -funsigned-char \
	-Wno-pointer-sign

libmondaiincludedir = $(includedir)/libmondai

libmondaiinclude_HEADERS =	\
	lock.h					\
	misc_v.h				\
	LBSfunc.h				\
	OpenCOBOL_v.h			\
	dotCOBOL_v.h			\
	Native_v.h				\
	Text_v.h				\
	XML_v.h					\
	php_v.h					\
	json_v.h				\
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
	numeric.h				\
	fileutils.h

AM_CPPFLAGS 			= 		\
	$(GLIB_CFLAGS) 			\
	$(XML_CFLAGS)			\
	$(JSON_CFLAGS) 			\
	-I$(top_srcdir)

libmondai_la_SOURCES	=	\
	libmondai.h						\
	misc.c							\
	lock.c			lock.h			\
	LBSfunc.c		LBSfunc.h		\
	cobolvalue.c	cobolvalue.h	\
	OpenCOBOL.c		OpenCOBOL_v.h	\
	dotCOBOL.c		dotCOBOL_v.h	\
	Native.c		Native_v.h		\
	Text.c			Text_v.h		\
	php.c			php_v.h			\
	json.c			json_v.h		\
	XML.c			XML_v.h			\
	hash.c			hash_v.h		\
	getset.c		getset.h		\
	value.c			value.h			\
	valueconv.c		valueconv.h		\
	fixed.c			fixed_v.h		\
	numerici.c		numerici.h		\
	others.c		others.h		\
	RecParser.c		RecParser.h		\
	Lex.c			Lex.h			\
	memory.c		memory_v.h		\
	monstring.c		monstring.h		\
	numeric.c		numeric.h		\
	fileutils.c		fileutils.h		\
	debug.h

libmondai_la_LIBADD	=		\
	$(GLIB_LIBS)			\
	$(XML_LIBS)				\
	$(JSON_LIBS)

LDADD 				=		\
	$(GLIB_LIBS)			\
	$(XML_LIBS)				\
	$(JSON_LIBS)			\
	libmondai.la

testconv_SOURCES	=		\
	testconv.c

testvalue_SOURCES	=		\
	testvalue.c

testNativePack_SOURCES	=		\
	testNativePack.c

testmem_SOURCES		=		\
	testmem.c

testfixed_SOURCES		=		\
	testfixed.c

testfileutils_SOURCES		=		\
	testfileutils.c

testjson_SOURCES		=		\
	testjson.c

testnumeric_SOURCES		=		\
	testnumeric.c

testOpenCOBOLPack_SOURCES		=		\
	testOpenCOBOLPack.c

EXTRA_DIST = lgpl testrec.rec testinc.rec
