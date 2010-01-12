SUBDIRS = \
	buffer 		\
	distance 	\
	linemerge 	\
	overlay 	\
	polygonize	\
	predicate	\
	relate		\
        union           \
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
	predicate/liboppredicate.la	\
	relate/liboprelate.la		\
        union/libopunion.la             \
	valid/libopvalid.la


