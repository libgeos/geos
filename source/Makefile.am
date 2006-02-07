SUBDIRS = \
	algorithm \
	geom \
	headers \
	index \
	io \
	noding \
	operation \
	planargraph \
	geomgraph \
	precision \
	util

EXTRA_DIST = Makefile.vc

lib_LTLIBRARIES = libgeos.la

libgeos_la_LDFLAGS = \
	-release @VERSION_MAJOR@.@VERSION_MINOR@.@VERSION_PATCH@ \
	-no-undefined

libgeos_la_SOURCES = 

libgeos_la_LIBADD = \
	algorithm/libalgorithm.la \
	geom/libgeom.la \
	geomgraph/libgeomgraph.la \
	index/libindex.la \
	io/libio.la \
	noding/libnoding.la \
	operation/liboperation.la \
	planargraph/libplanargraph.la \
	precision/libprecision.la \
	util/libutil.la  
