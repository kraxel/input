# just some maintainer stuff for me ...
########################################################################

make-sync-dir = $(HOME)/src/gnu-make
pbuilder-dir  = /work/pbuilder/result
snapshot-dir  = $(HOME)/snapshot
snapshot-date = $(shell date +%Y%m%d)
snapshot-name = $(snapshot-dir)/$(PACKAGE)-$(snapshot-date).tar.gz

deb-version   = $(shell dpkg-parsechangelog | sed -n 's/^Version: \(.*:\|\)//p')
deb-arch     := $(shell uname -m | sed		\
			-e 's/i.86/i386/'	\
			-e 's/ppc/powerpc/')
deb-dsc      := ../$(PACKAGE)_$(VERSION).dsc
deb-changes  := $(pbuilder-dir)/$(PACKAGE)_$(VERSION)_$(deb-arch).changes


.PHONY: sync checkit release port tarball dist rpm
sync:: distclean
	test -d $(make-sync-dir)
	rm -f INSTALL mk/*.mk
	cp -v $(make-sync-dir)/INSTALL .
	cp -v $(make-sync-dir)/*.mk mk
	chmod 444 INSTALL mk/*.mk


dsc source $(deb-dsc): clean
	test "$(VERSION)" = "$(deb-version)"
	dpkg-buildpackage -S -us -uc -rfakeroot

debs pbuild $(deb-changes): $(deb-dsc)
	sudo /usr/sbin/pbuilder build $(deb-dsc)
	-lintian -i $(deb-changes)

release: $(deb-changes)
	debsign $(deb-changes)


tarball dist: realclean
	(cd ..; tar czf $(TARBALL) $(DIR))

snapshot snap: realclean
	(cd ..; tar czf $(snapshot-name) $(DIR))

rpm: tarball
	rpm -ta ../$(TARBALL)

