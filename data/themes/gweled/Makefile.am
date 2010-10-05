MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = @edje_cc@
EDJE_FLAGS = -v
#-id $(top_srcdir)/data/themes -fd $(top_srcdir)/data/themes

filesdir = $(datadir)/mokojeweled/themes

files_DATA = gweled.edj

EXTRA_DIST = \
	gweled.edc
# TODO add png or svg?

gweled.edj: gweled.edc
	$(EDJE_CC) $(EDJE_FLAGS) $< $@
                        
clean-local:
	rm -f *.edj
