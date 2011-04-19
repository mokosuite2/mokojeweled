MAINTAINERCLEANFILES = Makefile.in

AM_CPPFLAGS = @ELEMENTARY_CFLAGS@ 		\
		@ECORE_CFLAGS@			\
		@EDJE_CFLAGS@			\
		@EINA_CFLAGS@			\
		@EVAS_CFLAGS@			\
		-DDATADIR=\""$(datadir)"\" 	\
		-DLOCALEDIR=\"$(localedir)\" 	\
		-Wall

AM_LDFLAGS = \
	@ELEMENTARY_LIBS@ \
	@ECORE_LIBS@ \
	@EDJE_LIBS@ \
	@EINA_LIBS@ \
	@EVAS_LIBS@

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

