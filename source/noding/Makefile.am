SUBDIRS = snapround

noinst_LTLIBRARIES = libnoding.la

INCLUDES = -I$(top_srcdir)/source/headers 

libnoding_la_SOURCES = \
	BasicSegmentString.cpp \
	FastNodingValidator.cpp \
	FastSegmentSetIntersectionFinder.cpp \
	IntersectionAdder.cpp \
	IntersectionFinderAdder.cpp \
	IteratedNoder.cpp \
	MCIndexNoder.cpp \
	MCIndexSegmentSetMutualIntersector.cpp \
	NodedSegmentString.cpp \
	NodingValidator.cpp \
	Octant.cpp \
	OrientedCoordinateArray.cpp \
	ScaledNoder.cpp \
	SegmentIntersectionDetector.cpp \
	SegmentNode.cpp \
	SegmentNodeList.cpp \
	SegmentString.cpp \
	SegmentStringUtil.cpp \
	SimpleNoder.cpp \
	SingleInteriorIntersectionFinder.cpp

libnoding_la_LIBADD = snapround/libsnapround.la

