#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
prefix=@prefix@
top_srcdir=@top_srcdir@
GEOS_CAPI_VERSION="@VERSION@-CAPI-@CAPI_VERSION@"
GEOS_JTS_PORT="@JTS_PORT@"

AM_CPPFLAGS = -I$(top_srcdir)/include

capidir = $(includedir)

nodist_capi_HEADERS = \
    geos_c.h

lib_LTLIBRARIES = libgeos_c.la

DIR_SOURCES = \
    geos_c.cpp \
    geos_ts_c.cpp \
    $(NULL)

libgeos_c_la_SOURCES = $(DIR_SOURCES)
libgeos_c_la_CPPFLAGS = $(AM_CPPFLAGS) -DGEOS_CAPI_VERSION='$(GEOS_CAPI_VERSION)' -DGEOS_JTS_PORT='$(GEOS_JTS_PORT)'
libgeos_c_la_LIBADD = $(top_builddir)/src/libgeos.la
libgeos_c_la_LDFLAGS = \
    -version-info @CAPI_INTERFACE_CURRENT@:@CAPI_INTERFACE_REVISION@:@CAPI_INTERFACE_AGE@ \
    -no-undefined

dist-local:
	cp -p $(DIST_SOURCES) Makefile.am Makefile.in $(distdir)

EXTRA_DIST = geos_c.h CMakeLists.txt
