prefix=@prefix@
top_srcdir=@top_srcdir@
GEOS_CAPI_VERSION="@VERSION@-CAPI-@CAPI_VERSION@"

noinst_PROGRAMS = geostest 

geostest_SOURCES = geostest.c
geostest_LDADD = libgeos_c.la
geostest_INCLUDES = -I.

capidir = $(includedir)

nodist_capi_HEADERS = \
	geos_c.h

lib_LTLIBRARIES = libgeos_c.la

LIBS = $(top_builddir)/source/geom/libgeos.la

DIR_SOURCES = \
	geos_c.cpp 

libgeos_c_la_SOURCES = $(DIR_SOURCES)
libgeos_c_la_CPPFLAGS = -DGEOS_CAPI_VERSION='$(GEOS_CAPI_VERSION)'


libgeos_c_la_LDFLAGS = -version-info @CAPI_INTERFACE_CURRENT@:@CAPI_INTERFACE_REVISION@:@CAPI_INTERFACE_AGE@

dist-local:
	cp -p $(DIST_SOURCES) Makefile.am Makefile.in $(distdir)

