AC_INIT([Mokojeweled], [0.1], [daniele.athome@gmail.com], [mokojeweled])
AC_CONFIG_SRCDIR(src)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE

AC_ISC_POSIX
AC_GNU_SOURCE
AC_DISABLE_STATIC
AC_PROG_CC
AC_PROG_LIBTOOL

AM_GNU_GETTEXT(external)
AM_GNU_GETTEXT_VERSION([0.12.1])

PKG_CHECK_MODULES(
	ELEMENTARY, elementary, [], [AC_MSG_FAILURE([elementary not found])]
)

AC_ARG_ENABLE(debug,
    AC_HELP_STRING([--enable-debug], [Enable debug build (default: disabled)]),
    enable_debug=$enableval,
    enable_debug=no)

if test "x$enable_debug" != "xno"; then
    AC_DEFINE(DEBUG, 1, [Define to enable debug build])
fi


AC_SUBST(VERSION)

EFL_WITH_BIN([edje], [edje-cc], [edje_cc])

AC_OUTPUT([
Makefile
data/Makefile
data/themes/Makefile
data/themes/gweled/Makefile
po/Makefile.in
src/Makefile
mokojeweled.spec
])
