# Configure paths for libmondai
# Koji SHIMIZU 03-03-10

dnl AM_PATH_LIBMONDAI([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND ]]])
dnl Test for libmondai, and define LIBMONDAI_CFLAGS and LIBMONDAI_LIBS
dnl

AC_DEFUN(AM_PATH_LIBMONDAI,
[dnl 
dnl Get the cflags and libraries from the libmondai-config script
dnl
AC_ARG_WITH(libmondai-prefix,
            [  --with-libmondai-prefix=PFX      Prefix where libmondai is installed (optional)],
            libmondai_config_prefix="$withval",
            libmondai_config_prefix="")

AC_ARG_WITH(libmondai-exec-prefix,
            [  --with-libmondai-exec-prefix=PFX Exec prefix where libmondai is installed (optional)],
            libmondai_config_exec_prefix="$withval",
            libmondai_config_exec_prefix="")

AC_ARG_ENABLE(libmondaitest,
              [  --disable-libmondaitest Do not try to compile and run a test libmondai program],
              ,
              enable_libmondaitest=yes)

  if test x$libmondai_config_exec_prefix != x ; then
     libmondai_config_args="$libmondai_config_args --exec-prefix=$libmondai_config_exec_prefix"
     if test x${LIBMONDAI_CONFIG+set} != xset ; then
        LIBMONDAI_CONFIG=$libmondai_config_exec_prefix/bin/libmondai-config
     fi
  fi
  if test x$libmondai_config_prefix != x ; then
     libmondai_config_args="$libmondai_config_args --prefix=$libmondai_config_prefix"
     if test x${LIBMONDAI_CONFIG+set} != xset ; then
        LIBMONDAI_CONFIG=$libmondai_config_prefix/bin/libmondai-config
     fi
  fi

  AC_PATH_PROG(LIBMONDAI_CONFIG, libmondai-config, no)
  min_libmondai_version=ifelse([$1], ,0.0.1,$1)
  AC_MSG_CHECKING(for libmondai - version >= $min_libmondai_version)
  no_libmondai=""
  if test "$LIBMONDAI_CONFIG" = "no" ; then
    no_libmondai=yes
  else
    LIBMONDAI_CFLAGS=`$LIBMONDAI_CONFIG $libmondai_config_args --cflags`
    LIBMONDAI_LIBS=`$LIBMONDAI_CONFIG $libmondai_config_args --libs`
    libmondai_config_major_version=`$LIBMONDAI_CONFIG $libmondai_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    libmondai_config_minor_version=`$LIBMONDAI_CONFIG $libmondai_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    libmondai_config_micro_version=`$LIBMONDAI_CONFIG $libmondai_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    if test "x$enable_libmondaitest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $LIBMONDAI_CFLAGS"
      LIBS="$LIBMONDAI_LIBS $LIBS"
dnl
dnl Now check if the installed libmondai is sufficiently new. (Also sanity
dnl checks the results of libmondai-config to some extent
dnl
      rm -f conf.libmondaitest
      AC_TRY_RUN([
#include <libmondai.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int 
main ()
{
  int major, minor, micro;
  char *tmp_version;

  system ("touch conf.libmondaitest");

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = strdup("$min_libmondai_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
    printf("%s, bad version string\n", "$min_libmondai_version");
    exit(1);
  }
  
  if (!LIBMONDAI_CHECK_VERSION($libmondai_config_major_version,
                               $libmondai_config_minor_version,
                               $libmondai_config_micro_version)) {
    printf("\n*** An old version of libmondai (%d.%d.%d) was found.\n",
           libmondai_major_version, libmondai_minor_version,
	   libmondai_micro_version);
    printf("*** You need a version of libmondai newer than %d.%d.%d. The latest version of\n",
           major, minor, micro);
    printf("*** libmondai is always available from http://www.montsuqi.org/.\n");
    printf("***\n");
    printf("*** If you have already installed a sufficiently new version, this error\n");
    printf("*** probably means that the wrong copy of the libmondai-config shell script is\n");
    printf("*** being found. The easiest way to fix this is to remove the old version\n");
    printf("*** of libmondai, but you can also set the LIBMONDAI_CONFIG environment to point to the\n");
    printf("*** correct copy of libmondai-config. (In this case, you will have to\n");
    printf("*** modify your LD_LIBRARY_PATH enviroment variable, or edit /etc/ld.so.conf\n");
    printf("*** so that the correct libraries are found at run-time))\n");

    return 1;
  }

  return 0;
}
],, no_libmondai=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_libmondai" = x ; then
     AC_MSG_RESULT(yes (version $libmondai_config_major_version.$libmondai_config_minor_version.$libmondai_config_micro_version))
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$LIBMONDAI_CONFIG" = "no" ; then
       echo "*** The libmondai-config script installed by libmondai could not be found"
       echo "*** If libmondai was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the LIBMONDAI_CONFIG environment variable to the"
       echo "*** full path to libmondai-config."
     else
       if test -f conf.libmondaitest ; then
        :
       else
          echo "*** Could not run libmondai test program, checking why..."
          CFLAGS="$CFLAGS $LIBMONDAI_CFLAGS"
          LIBS="$LIBS $LIBMONDAI_LIBS"
          AC_TRY_LINK([
#include <libmondai.h>
#include <stdio.h>
],      [ return ((libmondai_major_version) || (libmondai_minor_version) || (libmondai_micro_version)); ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding libmondai or finding the wrong"
          echo "*** version of LIBMONDAI. If it is not finding LIBMONDAI, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
          echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means libmondai was incorrectly installed"
          echo "*** or that you have moved libmondai since it was installed. In the latter case, you"
          echo "*** may want to edit the libmondai-config script: $LIBMONDAI_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     LIBMONDAI_CFLAGS=""
     LIBMONDAI_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(LIBMONDAI_CFLAGS)
  AC_SUBST(LIBMONDAI_LIBS)
  rm -f conf.libmondaitest
])
