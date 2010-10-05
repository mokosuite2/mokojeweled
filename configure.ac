AC_PREREQ(2.53)
AC_INIT(mokojeweled, 0.0.1, http://gitorious.org/mokosuite2/)
AM_INIT_AUTOMAKE(mokojeweled, 0.0.1)
AC_CONFIG_SRCDIR(src/)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_GNU_SOURCE
AM_GNU_GETTEXT([external])
AC_ISC_POSIX
AC_PROG_CC
AC_STDC_HEADERS
AC_PROG_LIBTOOL
AC_CONFIG_MACRO_DIR([m4])

PKG_CHECK_MODULES(GLIB, glib-2.0, [],
		[AC_MSG_FAILURE([glib not found])]
		)

PKG_CHECK_MODULES(ELEMENTARY, elementary, [],
		[AC_MSG_FAILURE([elementary not found])]
		)

BINDIR=`eval echo $bindir`
AC_SUBST([BINDIR])
DATADIR=`eval echo $datadir`
AC_SUBST([DATADIR])

AC_SUBST([VERSION])

EFL_WITH_BIN([edje], [edje-cc], [edje_cc])

AC_OUTPUT([
Makefile
data/Makefile
data/themes/Makefile
po/Makefile.in
src/Makefile
])
