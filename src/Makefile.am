MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @ELEMENTARY_CFLAGS@ 		\
		-DDATADIR=\""$(datadir)"\" 	\
		-DLOCALEDIR=\"$(localedir)\" 	\
		-Wall -DDEBUG

AM_LDFLAGS = @ELEMENTARY_LIBS@

bin_PROGRAMS = mokojeweled

mokojeweled_SOURCES =	\
	globals.h	\
	main.c		\
	menu.c		\
	menu.h		\
	mokowin.c	\
	mokoinwin.c	\
	gui.c		\
	gui.h		\
	board.c		\
	board.h		\
	theme.c		\
	theme.h

