# $Id: Makefile.am 2841 2010-01-13 00:48:20Z mloskot $
#
# This file is part of project GEOS (http://trac.osgeo.org/geos/) 
#
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/tests/unit/tut -I$(top_builddir)/capi

EXTRA_DIST = \
	brokengrammar \
	test.wkt \
	test.expected \
	testrunner.sh

# TODO: Enable if sample input WKT file is provided
#TESTS = geostest

check_PROGRAMS = geostest

# The -lstdc++ is needed for --disable-shared to work
geostest_SOURCES = geostest.c
geostest_LDADD = $(top_builddir)/capi/libgeos_c.la -lstdc++
