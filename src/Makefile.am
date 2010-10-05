MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = -DDATADIR=\""$(datadir)"\" \
		@ELEMENTARY_CFLAGS@	\
		-DLOCALEDIR=\"$(localedir)\" \
		-Wall

AM_LDFLAGS = @ELEMENTARY_LIBS@

bin_PROGRAMS = mokojeweled

mokojeweled_SOURCES =	\
	main.c		\
	test_mainwin.c
