SUBDIRS = 

geosdir = $(includedir)/geos/index/strtree

EXTRA_DIST = 

# NOTES:
#
# AbstractSTRtree.h needed by STRtree.h for inheritance
# AbstractSTRtree.h needed by SIRtree.h for inheritance
# Interval.h needed by  SIRtree.h for inlines
# Boundable.h needed by ItemBoundable.h and AbstractNode.h for inheritance
# AbstractNode.h needed by AbstractSTRtree.h for inlines

geos_HEADERS = \
	AbstractNode.h	\
	AbstractSTRtree.h	\
	Boundable.h	\
	Interval.h	\
	SIRtree.h	\
	STRtree.h

nodist_geos_HEADERS = \
	ItemBoundable.h	
