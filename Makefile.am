## librdfa/Makefile.am

ACLOCAL_AMFLAGS = -I setup

SUBDIRS = c python tests docs

pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = librdfa.pc

EXTRA_DIST = \
	LICENSE \
	LICENSE-apache-2.0 \
	LICENSE.gpl-2.0 \
	LICENSE.lgpl-2.1 \
	installers/debian/changelog.in \
	installers/debian/compat \
	installers/debian/control \
	installers/debian/copyright \
	installers/debian/librdfa1-dev.install \
	installers/debian/librdfa1.install \
	installers/debian/python-rdfa.install \
	installers/debian/rules

packages:
	@echo "Generating Debian packages..."
	$(LN_S) installers/debian debian
	dpkg-buildpackage -us -uc -rfakeroot
	mkdir -p installers/packages
	mv ../*rdfa$(VERSION).deb installers/packages
	@echo "Generating RPM packages..."
	cd installers/packages && fakeroot alien --to-rpm -k --scripts *.deb
	@echo "Generating TGZ packages..."
	cd installers/packages && fakeroot alien --to-tgz -k --scripts *.deb

zip-windows:
	@echo "Generating Windows ZIP release..."
	mkdir -p installers/librdfa-windows-$(VERSION)
	cp -p c/rdfa.h installers/librdfa-windows-$(VERSION)
	cp -p c/rdfa_utils.h installers/librdfa-windows-$(VERSION)
	cp -p libs/rdfa.dll installers/librdfa-windows-$(VERSION)
	cp -p libs/rdfa.lib installers/librdfa-windows-$(VERSION)
	cd installers && zip -r9 librdfa-$(VERSION)-windows-$(VERSION).zip \
		librdfa-windows-$(VERSION)
	rm -rf installers/librdfa-windows-$(VERSION)

zip-macos:
	@echo "Generating Mac OS X ZIP release..."
	mkdir -p installers/librdfa-macos-$(VERSION)
	cp -p c/rdfa.h installers/librdfa-macos-$(VERSION)
	cp -p c/rdfa_utils.h installers/librdfa-macos-$(VERSION)
	cp -p libs/librdfa.dylib installers/librdfa-macos-$(VERSION)
	cp -p libs/librdfa.a installers/librdfa-macos-$(VERSION)
	cd installers && zip -r9 librdfa-$(VERSION)-macos-$(VERSION).zip \
		librdfa-macos-$(VERSION)
	rm -rf installers/librdfa-macos-$(VERSION)

.PHONY: packages zip-windows zip-macos

clean-local:
	rm -rf installers/librdfa-*
	rm -rf installers/packages
	rm -f debian

## end librdfa/Makefile.am
