MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -DPKGDATADIR=\"$(pkgdatadir)\" \
		-I ../			\
		@GLIB_CFLAGS@		\
		@ELEMENTARY_CFLAGS@	\
		-DLOCALEDIR=\"$(localedir)\" \
		-Wall


AM_LDFLAGS =	@GLIB_LIBS@		\
		@ELEMENTARY_LIBS@


bin_PROGRAMS = mokojeweled

mokojeweled_SOURCES =	\
	main.c		\
	test_mainwin.c
