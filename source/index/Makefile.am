#SUBDIRS = bintree chain quadtree strtree sweepline

noinst_LTLIBRARIES = libindex.la

INCLUDES = -I$(top_srcdir)/source/headers 

libindex_la_SOURCES = \
	bintree/Bintree.cpp \
	bintree/BinTreeInterval.cpp \
	bintree/BinTreeNode.cpp \
	bintree/Key.cpp \
	bintree/NodeBase.cpp \
	bintree/Root.cpp \
	chain/indexMonotoneChain.cpp \
	chain/MonotoneChainBuilder.cpp \
	chain/MonotoneChainOverlapAction.cpp \
	chain/MonotoneChainSelectAction.cpp \
	quadtree/DoubleBits.cpp \
	quadtree/IntervalSize.cpp \
	quadtree/Quadtree.cpp \
	quadtree/QuadTreeKey.cpp \
	quadtree/QuadTreeNode.cpp \
	quadtree/QuadTreeNodeBase.cpp \
	quadtree/QuadTreeRoot.cpp \
	strtree/AbstractNode.cpp \
	strtree/AbstractSTRtree.cpp \
	strtree/Interval.cpp \
	strtree/ItemBoundable.cpp \
	strtree/SIRtree.cpp \
	strtree/STRtree.cpp \
	sweepline/indexSweepLineEvent.cpp \
	sweepline/SweepLineIndex.cpp \
	sweepline/SweepLineInterval.cpp 

libindex_la_LIBADD = 

