## librdfa/c/Makefile.am

lib_LTLIBRARIES = librdfa.la

include_HEADERS = \
	rdfa.h \
	rdfa_utils.h

librdfa_la_SOURCES = \
	context.c \
	curie.c \
	iri.c \
	language.c \
	lists.c \
	namespace.c \
	rdfa.c \
	rdfa_utils.c \
	subject.c \
	triple.c

if PARSER_LIBXML2
librdfa_la_SOURCES += parser_libxml2.c
endif

if PARSER_RAPTOR
librdfa_la_SOURCES += parser_raptor.c
endif

if MINGW32
librdfa_la_SOURCES += \
	mingw32_utils.c \
	mingw32_utils.h
endif

if NEED_STRTOK_R
librdfa_la_SOURCES += \
	strtok_r.c \
	strtok_r.h
endif

AM_CPPFLAGS       = $(LIBXML2_CFLAGS)
librdfa_la_LIBADD = $(LIBXML2_LIBS)

AM_LDFLAGS = -version-info $(LIBRDFA_SO_VERSION)

## end librdfa/c/Makefile.am
