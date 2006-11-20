# action is snap or all
PKGDIR=/usr/local/pkg

all: maui326

maui326:
	ACTION=snap ; \
	PKG=maui ; \
	VERSION=3.2.6 ; \
	PATCH=p18 ; \
	BASEDIR=$$PKG-$$VERSION$$PATCH ; \
	cd $$BASEDIR ; \
	for i in src/[ms]* include; do \
	(cd $$i;mkheader.pl ./header.tmpl ./header.cfg $(PKGDIR)/$$BASEDIR/$$i *.[ch]); \
	done ; \
	for i in contrib docs; do \
	(cd $$i;cp -r * $(PKGDIR)/$$BASEDIR/$$i); \
	(cd $(PKGDIR)/$$BASEDIR/$$i;find . -name CVS -exec rm -rf {} \; 2>/dev/null) ; \
	done ; \
	cd .. ; \
	TMPFILE=$(PKGDIR)/tmp/$$BASEDIR.tar.gz; \
	rm -f $$TMPFILE; \
	(cd $(PKGDIR);tar -czf $$TMPFILE $$BASEDIR) ;\
	./copyfiles maui $$VERSION $$PATCH $$TMPFILE $$ACTION

