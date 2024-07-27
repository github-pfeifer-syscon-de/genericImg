dnl Process this file with autoconf to produce a configure script.

AC_CONFIG_MACRO_DIR([m4])

m4_define([v_maj], [0])
m4_define([v_min], [3])
m4_define([v_mic], [2])
m4_define([project_version], [v_maj.v_min.v_mic])
m4_define([lt_cur], m4_eval(v_maj + v_min))
m4_define([lt_rev], [v_mic])
m4_define([lt_age], [v_min])

AC_INIT([genericimg],[project_version])

version_info="lt_cur:lt_rev:lt_age"
AC_SUBST([version_info])

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11])

AM_SILENT_RULES([yes])

AX_CHECK_ENABLE_DEBUG([info], [DEBUG], [RELEASE], [IS-RELEASE])
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX([20], [noext], [optional])

AC_DISABLE_STATIC

LT_INIT

AC_CANONICAL_HOST
AC_MSG_NOTICE([host_os $host_os])

depend_gtkmm="gtkmm-3.0"
AC_SUBST([depend_gtkmm])
PKG_CHECK_MODULES([GTKMM], [$depend_gtkmm])

depend_libpng="libpng"
AC_SUBST([depend_libpng])
PKG_CHECK_MODULES([LIBPNG], [$depend_libpng])

depend_glibmm="glibmm-2.4 giomm-2.4"
AC_SUBST([depend_glibmm])
PKG_CHECK_MODULES([GLIBMM], [$depend_glibmm])

depend_libexif="libexif"
AC_SUBST([depend_libexif])
PKG_CHECK_MODULES([LIBEXIF], [$depend_libexif])

depend_jsonglib1="json-glib-1.0"
AC_SUBST([depend_jsonglib1])
PKG_CHECK_MODULES(JSONGLIB1, [$depend_jsonglib1])

AC_ARG_WITH([sysdlog], AS_HELP_STRING([--with-sysdlog], [Build with systemdlog otherwise user home log will be used]))

AS_IF([test "x$with_sysdlog" = "xyes"], [
    PKG_CHECK_MODULES([SYSTEMDLOG], [libsystemd], [AC_DEFINE([SYSDLOG], [1], [Using systemd journal])])
])

AM_CONDITIONAL([SYSDLOG], test "x$with_sysdlog" = "xyes"])

if test "$host_os" = "mingw32" ; then
    AC_SUBST(EXTRA_LDFLAGS, "-no-undefined")
else
    AC_SUBST(EXTRA_LDFLAGS, "")
fi

AC_CONFIG_FILES([
Makefile
res/Makefile
src/genericimg.pc
src/Makefile
])
AC_OUTPUT
