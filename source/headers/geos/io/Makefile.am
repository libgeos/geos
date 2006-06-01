SUBDIRS = 

geosdir = $(includedir)/geos/io

EXTRA_DIST = 

# NOTES:
# ByteOrderDataInStream.{h,inl} needed by WKBReader.h for composition
# ByteOrderValues.h needed by ByteOrderDataInStream.inl for inlines
# ParseException.h needed by ByteOrderDataInStream.inl for inlines
# WKBConstants.h is worth exposing

geos_HEADERS = \
	ByteOrderDataInStream.h		\
	ByteOrderDataInStream.inl	\
	ByteOrderValues.h		\
	ParseException.h		\
	WKBConstants.h			\
	WKBReader.h			\
	WKBWriter.h			\
	WKTReader.h			\
	WKTReader.inl			\
	WKTWriter.h			

nodist_geos_HEADERS = 		\
	StringTokenizer.h	\
	Writer.h
