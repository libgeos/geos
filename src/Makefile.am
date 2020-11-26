#
# This file is part of project GEOS (http://trac.osgeo.org/geos/)
#
SUBDIRS = \
    algorithm \
    edgegraph \
    geom \
    index \
    io \
    linearref \
    math \
    noding \
    operation \
    planargraph \
    geomgraph \
    precision \
    simplify \
    triangulate \
    shape \
    util

EXTRA_DIST = dirlist.mk CMakeLists.txt info.plist.in

AM_CPPFLAGS = -I$(top_srcdir)/include

lib_LTLIBRARIES = libgeos.la

# libgeos uses -release because it is not feasible to know if the ABI
# has changed between releases; the project chooses not to expend the
# effort to determine this because GEOS does not promise ABI stability.
libgeos_la_LDFLAGS = \
    -release @VERSION_RELEASE@ \
    -no-undefined

libgeos_la_SOURCES = \
    inlines.cpp

libgeos_la_LIBADD = \
    algorithm/libalgorithm.la \
    edgegraph/libedgegraph.la \
    geom/libgeom.la \
    geomgraph/libgeomgraph.la \
    index/libindex.la \
    io/libio.la \
    linearref/liblinearref.la \
    math/libmath.la \
    noding/libnoding.la \
    operation/liboperation.la \
    planargraph/libplanargraph.la \
    precision/libprecision.la \
    simplify/libsimplify.la \
    triangulate/libtriangulate.la \
    shape/libshape.la \
    util/libutil.la
