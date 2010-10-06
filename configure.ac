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

IT_PROG_INTLTOOL([0.35.0])

#AM_GNU_GETTEXT(external)
GETTEXT_PACKAGE="mokojeweled"
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, ["$GETTEXT_PACKAGE"], [Name of the gettext message domain])

PKG_CHECK_MODULES(
	ELEMENTARY, elementary, [], [AC_MSG_FAILURE([elementary not found])]
)

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
