SUBDIRS = snapround

noinst_LTLIBRARIES = libnoding.la

INCLUDES = -I$(top_srcdir)/source/headers 

libnoding_la_SOURCES = \
	FastNodingValidator.cpp \
	IntersectionAdder.cpp \
	IntersectionFinderAdder.cpp \
	IteratedNoder.cpp \
	MCIndexNoder.cpp \
	NodingValidator.cpp \
	SegmentNode.cpp \
	SegmentNodeList.cpp \
	SegmentString.cpp \
	ScaledNoder.cpp \
	SimpleNoder.cpp \
	SingleInteriorIntersectionFinder.cpp \
	Octant.cpp

libnoding_la_LIBADD = snapround/libsnapround.la



