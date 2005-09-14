prefix=@prefix@
top_srcdir=@top_srcdir@

noinst_PROGRAMS = geostest 

geostest_SOURCES = geostest.c
geostest_LDADD = libgeos_c.la
geostest_INCLUDES = -I.

include_HEADERS = \
	geos_c.h

lib_LTLIBRARIES = libgeos_c.la

LIBS = $(top_builddir)/source/geom/libgeos.la

DIR_SOURCES = \
	geos_c.cpp 

libgeos_c_la_SOURCES = $(DIR_SOURCES)

libgeos_c_la_LDFLAGS = -version-info 1:0:0

dist-local:
	cp -p $(DIST_SOURCES) Makefile.am Makefile.in $(distdir)
