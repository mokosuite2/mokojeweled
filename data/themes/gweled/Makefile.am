MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS = -v
#-id $(top_srcdir)/data/themes -fd $(top_srcdir)/data/themes

filesdir = $(datadir)/mokojeweled/themes
files_DATA = gweled.edj

EXTRA_DIST = \
	$(files_DATA) \
	theme.edc \
	border.png \
	gem01.png \
	gem02.png \
	gem03.png \
	gem04.png \
	gem05.png \
	gem06.png \
	gem07.png

gweled.edj: theme.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@
                        
clean-local:
	rm -f *.edj
