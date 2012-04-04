## librdfa/python/Makefile.am

if BUILD_PYTHON

pythonsitedir = $(PYTHON_SITE_PKG)

dist_pythonsite_DATA = rdfa.py

pythonsite_LTLIBRARIES = _rdfa.la

_rdfa_la_SOURCES = \
	RdfaParser.cpp \
	RdfaParser.h \
	SwigRdfaParser.cpp

BUILT_SOURCES = SwigRdfaParser.cpp
CLEANFILES = $(BUILT_SOURCES)

_rdfa_la_LIBADD = $(top_builddir)/c/librdfa.la

AM_CPPFLAGS = \
	-I$(top_srcdir)/c \
	$(AX_SWIG_PYTHON_CPPFLAGS) \
	$(LIBXML2_CFLAGS)

AM_LDFLAGS = -avoid-version -export-dynamic -module

SwigRdfaParser.cpp: $(srcdir)/rdfa.i
	$(SWIG) $(AX_SWIG_PYTHON_OPT) -o $@ $(srcdir)/rdfa.i

rdfa.py: SwigRdfaParser.cpp
	@true

else # ! BUILD_PYTHON

SwigRdfaParser.cpp rdfa.py:
	@echo "cannot generate $@ without --enable-python"
	@exit 1

endif # BUILD_PYTHON

MAINTAINERCLEANFILES = rdfa.py

## end librdfa/python/Makefile.am
