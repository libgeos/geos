SUBDIRS = \
	buffer 		\
	distance 	\
	linemerge 	\
	overlay 	\
	polygonize	\
	predicate	\
	relate		\
	valid		

noinst_LTLIBRARIES = liboperation.la

INCLUDES = -I$(top_srcdir)/source/headers 

liboperation_la_SOURCES = \
	GeometryGraphOperation.cpp \
	IsSimpleOp.cpp 

liboperation_la_LIBADD = \
	buffer/libopbuffer.la		\
	distance/libopdistance.la	\
	linemerge/liboplinemerge.la	\
	overlay/libopoverlay.la		\
	polygonize/liboppolygonize.la	\
	relate/liboprelate.la		\
	predicate/liboppredicate.la	\
	valid/libopvalid.la


