MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS = -v
#-id $(top_srcdir)/data/themes -fd $(top_srcdir)/data/themes

filesdir = $(datadir)/mokojeweled/themes
files_DATA = gweled.edj

EXTRA_DIST = \
	theme.edc \
	border.png \
	gem01.png \
	gem01.svg \
	gem02.png \
	gem02.svg \
	gem03.png \
	gem03.svg \
	gem04.png \
	gem04.svg \
	gem05.png \
	gem05.svg \
	gem06.png \
	gem06.svg \
	gem07.png \
	gem07.svg

gweled.edj: theme.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@
                        
clean-local:
	rm -f *.edj
