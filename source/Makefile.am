SUBDIRS = \
	algorithm \
	geom \
	headers \
	index \
	io \
	linearref \
	noding \
	operation \
	planargraph \
	geomgraph \
	precision \
	simplify \
	util

EXTRA_DIST = Makefile.vc dirlist.mk

INCLUDES = -I$(top_srcdir)/source/headers

lib_LTLIBRARIES = libgeos.la

libgeos_la_LDFLAGS = \
	-release @VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_PATCH@ \
	-no-undefined

libgeos_la_SOURCES = \
	inlines.cpp

libgeos_la_LIBADD = \
	algorithm/libalgorithm.la \
	geom/libgeom.la \
	geomgraph/libgeomgraph.la \
	index/libindex.la \
	io/libio.la \
	linearref/liblinearref.la \
	noding/libnoding.la \
	operation/liboperation.la \
	planargraph/libplanargraph.la \
	precision/libprecision.la \
	simplify/libsimplify.la \
	util/libutil.la  
