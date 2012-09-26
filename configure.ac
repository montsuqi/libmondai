#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.61)
AC_INIT([libmondai],[1.4.6])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

MAJOR_VERSION=`echo AC_PACKAGE_VERSION|sed 's/\(.*\)\.\(.*\)\.\(.*\)/\1/'|sed 's/\([0-9][0-9]*\).*/\1/'`
MINOR_VERSION=`echo AC_PACKAGE_VERSION|sed 's/\(.*\)\.\(.*\)\.\(.*\)/\2/'|sed 's/\([0-9][0-9]*\).*/\1/'`
MICRO_VERSION=`echo AC_PACKAGE_VERSION|sed 's/\(.*\)\.\(.*\)\.\(.*\)/\3/'|sed 's/\([0-9][0-9]*\).*/\1/'`

LTVERSION=`expr $MAJOR_VERSION + $MINOR_VERSION`:$MICRO_VERSION:$MINOR_VERSION

AC_DEFINE_UNQUOTED(PACKAGE,"$PACKAGE","")
AC_DEFINE_UNQUOTED(VERSION,"$VERSION","")

AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(MICRO_VERSION)
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)
AC_SUBST(LTVERSION)

# Checks for programs.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL

AM_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h float.h stdint.h stdlib.h string.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_BIGENDIAN
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_STRUCT_TM
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_MMAP
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([localtime_r memset mkdir munmap strchr strdup strrchr strtol])

dnl ***************************************************************************
dnl Find pkg-config
dnl ***************************************************************************
AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test x$PKG_CONFIG = xno ; then
  AC_MSG_ERROR([*** pkg-config not found. See http://pkgconfig.sourceforge.net])
fi

dnl ***************************************************************************
dnl GLib
dnl ***************************************************************************
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.0.0)

dnl ***************************************************************************
dnl jsonc
dnl ***************************************************************************
PKG_CHECK_MODULES(JSON, json >= 0.9)

dnl ***************************************************************************
dnl SOAP
dnl ***************************************************************************
USE_SOAP="yes"
AC_ARG_ENABLE(soap,
    [  --disable-soap          disable soap [[enable]]],
    [USE_SOAP=$enable_soap])
if test "x$USE_SOAP" != "xno"; then
    AC_DEFINE(USE_SOAP,1,"")
fi
AM_CONDITIONAL(SOAP, test x$USE_SOAP != "xno")

dnl ***************************************************************************
dnl libxml2
dnl ***************************************************************************
USE_LIBXML2="yes"
AC_ARG_ENABLE(xml2,
    [  --disable-xml2          disable xml with libxml2 [[enable]]],
    [USE_LIBXML2=$enable_xml2])
if test "x$USE_LIBXML2" != "xno"; then
	PKG_CHECK_MODULES(XML, libxml-2.0 >= 2.6.0)
	AC_SUBST(XML_CFLAGS)
	AC_SUBST(XML_LIBS)
	AC_DEFINE(USE_XML,2,"")
fi

dnl ***************************************************************************

AC_CONFIG_FILES([
Makefile
src/Makefile
tools/Makefile
docs/Makefile
libmondai.pc
])
AC_OUTPUT

dnl ***************************************************************************
dnl output result
dnl ***************************************************************************

echo "------------------------------------------------------"
echo "libmondai Configuration:"
echo ""
echo "  VERSION:               $VERSION"
echo "  LTVERSION:             $LTVERSION"
echo "  libxml2 Support:       $USE_LIBXML2"
echo "  SOAP Support:          $USE_SOAP"
echo ""
