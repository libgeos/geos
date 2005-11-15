/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 * $Log$
 * Revision 1.4  2005/11/15 10:04:37  strk
 * Reduced heap allocations (vectors, mostly).
 * Enforced const-correctness, changed some interfaces
 * to use references rather then pointers when appropriate.
 *
 * Revision 1.3  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/14 06:04:26  ybychkov
 * "geomgraph/index" committ problem fixed.
 *
 * Revision 1.9  2004/03/19 09:49:29  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.8  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.7  2003/10/15 15:30:32  strk
 * Declared a SweepLineEventOBJ from which MonotoneChain and SweepLineSegment
 * derive to abstract SweepLineEvent object previously done on void * pointers.
 * No more compiler warnings...
 *
 **********************************************************************/


#include <geos/geomgraphindex.h>

namespace geos {

MonotoneChain::MonotoneChain(MonotoneChainEdge *newMce,int newChainIndex) {
	mce=newMce;
	chainIndex=newChainIndex;
}

MonotoneChain::~MonotoneChain() {
}

void
MonotoneChain::computeIntersections(MonotoneChain *mc, SegmentIntersector *si)
{
	mce->computeIntersectsForChain(chainIndex,*(mc->mce),mc->chainIndex,*si);
}

}

