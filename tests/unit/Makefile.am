
INCLUDES = -I$(top_srcdir)/source/headers -I$(top_srcdir)/tests/tut

TESTS = geos_unit

#noinst_PROGRAMS = geos_unit
check_PROGRAMS = geos_unit
#bin_PROGRAMS = geos_tut

geos_unit_LDADD = $(top_builddir)/source/libgeos.la 

geos_unit_SOURCES = \
	geos_unit.cpp \
	geom/Envelope.cpp \
	geom/LineSegment.cpp \
	io/WKBReader.cpp \
	index/quadtree/DoubleBitsTest.cpp \
	algorithm/PointLocator.cpp \
	geom/Geometry/isRectangle.cpp 

