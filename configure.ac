dnl -*- shell-script -*-
dnl Process this file with autoconf to produce a configure script.

AC_INIT([libgmbus], [0.3.0], [crunchy@bitkipper.net])
AC_CONFIG_SRCDIR([gmbus/mbus.h])
AM_INIT_AUTOMAKE([dist-bzip2])
AC_PREREQ(2.5)

AC_PROG_CC
AC_LANG(C)

AC_DISABLE_STATIC
AC_PROG_LIBTOOL

# by Marcelo Magallon <mmagallo@efis.ucr.ac.cr>
# Turn around -rpath problem with libtool 1.0c
# This define should be improbable enough to not conflict with anything
case ${host} in
  mipsel-unknown-linux-gnu)
  AC_MSG_RESULT([Fixing libtool for -rpath problems.])
  sed < libtool > libtool-2 \
  's/^hardcode_libdir_flag_spec.*$/hardcode_libdir_flag_spec=" -D__LIBTOOL_IS_A_FOOL__ "/'
  mv libtool-2 libtool
  chmod 755 libtool
  ;;
esac

AC_HEADER_STDC

AC_CACHE_CHECK([for ip_mreqn], [ac_cv_have_ip_mreqn],
    [AC_EGREP_HEADER([ip_mreqn], [netinet/in.h],
        [ac_cv_have_ip_mreqn=yes], [ac_cv_have_ip_mreqn=no])])
if test $ac_cv_have_ip_mreqn = yes; then
  AC_DEFINE([HAVE_IP_MREQN],1,[Define this if you have
      ip_mreqn in netinet/in.h])
fi

AC_PATH_PROG(DOXYGEN, doxygen, no)
AM_CONDITIONAL(WITH_DOXYGEN, test "x$DOXYGEN" != "xno")

dnl check for glib-2.0
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.6)
AC_SUBST(GLIB_LIBS)
AC_SUBST(GLIB_CFLAGS)

AC_ARG_ENABLE(guidelines,
    AS_HELP_STRING( [--enable-guidelines[=ARG]],
	[includes the Mbus guidelines implementation [default=yes]] ),
    guides=$enableval, guides=yes )

AM_CONDITIONAL(WITH_GUIDELINES, test "$guides" = "yes" )
if test "$guides" = "yes"; then
  GUIDELINE_INCLUDE="#include \"gmbus/rpcentity.h\""
else
  GUIDELINE_INCLUDE=""
fi

AC_SUBST(GUIDELINE_INCLUDE)

AC_ARG_ENABLE(tests,
    AS_HELP_STRING( [--enable-tests[=ARG]],
	[includes the Mbus library tests for development support [default=yes]] ),
    tests=$enableval, tests=yes )

echo $tests
AM_CONDITIONAL(WITH_TESTS, test "$tests" = "yes" )

AC_OUTPUT( gmbus-1.0.pc \
    Makefile gmbus/Makefile tests/Makefile \
    doc/Makefile doc/Doxyfile \
    gmbus/gmbus.h
)
