# boilerplate Makefile.am
if WITH_DOXYGEN
DOC=doc
else
DOC=
endif

if WITH_TESTS
TESTS=tests
else
TESTS=
endif

SUBDIRS = gmbus $(TESTS) $(DOC)
DIST_SUBDIRS= gmbus tests doc

EXTRA_DIST = gmbus-1.0.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gmbus-1.0.pc

$(pkgconfig_DATA): config.status

