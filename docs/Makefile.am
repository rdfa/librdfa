## librdfa/docs/Makefile.am

if BUILD_DOCS

all-local: build.stamp

build.stamp:
	test -f Makefile.am	# need builddir == srcdir
	cd .. && $(DOXYGEN) $(abs_top_srcdir)/setup/docs.doxygen
	touch $@

endif # BUILD_DOCS

clean-local:
	rm -rf c-api
	rm -f build.stamp
	rm -f api.doxygen.errors

## end librdfa/docs/Makefile.am
