#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
prefix=@prefix@
top_srcdir=@top_srcdir@
top_builddir=@top_builddir@

noinst_PROGRAMS = example 

LIBS = $(top_builddir)/src/libgeos.la

example_SOURCES = example.cpp 
example_LDADD = $(LIBS)

AM_CPPFLAGS = -I$(top_srcdir)/include

DOXYGEN = doxygen

doxygen-html apidoc doxygen: Doxyfile
	$(DOXYGEN) $<

clean-local:
	rm -Rf doxygen_docs

EXTRA_DIST = Doxyfile
