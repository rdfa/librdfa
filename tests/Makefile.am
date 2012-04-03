## librdfa/tests/Makefile.am

noinst_PROGRAMS = \
	rdfa2n3 \
	rdfastring2n3 \
	curies \
	speed \
	speed2

AM_CPPFLAGS = \
	-I$(top_srcdir)/c \
	$(LIBXML2_CFLAGS)

LDADD = \
	$(top_builddir)/c/librdfa.la \
	$(LIBXML2_LIBS)

EXTRA_DIST = test-cases/fetch-tests

clean-local:
	rm $(noinst_PROGRAMS)

## end librdfa/tests/Makefile.am
