/**********************************************************************
 * $Id: SweepLineSegment.cpp 1820 2006-09-06 16:54:23Z mloskot $
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geomgraph/index/SweepLineSegment.h>
#include <geos/geomgraph/index/SegmentIntersector.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/geomgraph/Edge.h>

using namespace geos::geom;

namespace geos {
namespace geomgraph { // geos.geomgraph
namespace index { // geos.geomgraph.index

SweepLineSegment::SweepLineSegment(Edge *newEdge, int newPtIndex):
	edge(newEdge),
	pts(newEdge->getCoordinates()),
	ptIndex(newPtIndex)
{
	//pts=newEdge->getCoordinates();
	//edge=newEdge;
	//ptIndex=newPtIndex;
}

SweepLineSegment::~SweepLineSegment()
{
}

double
SweepLineSegment::getMinX()
{
	double x1=pts->getAt(ptIndex).x;
	double x2=pts->getAt(ptIndex+1).x;
	return x1<x2?x1:x2;
}

double
SweepLineSegment::getMaxX()
{
	double x1=pts->getAt(ptIndex).x;
	double x2=pts->getAt(ptIndex+1).x;
	return x1>x2?x1:x2;
}

void
SweepLineSegment::computeIntersections(SweepLineSegment *ss,
	SegmentIntersector *si)
{
	si->addIntersections(edge, ptIndex, ss->edge, ss->ptIndex);
}

} // namespace geos.geomgraph.index
} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.6  2006/03/15 17:16:31  strk
 * streamlined headers inclusion
 *
 * Revision 1.5  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.4  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.3  2005/11/15 10:04:37  strk
 *
 * Reduced heap allocations (vectors, mostly).
 * Enforced const-correctness, changed some interfaces
 * to use references rather then pointers when appropriate.
 *
 * Revision 1.2  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/14 06:04:26  ybychkov
 * "geomgraph/index" committ problem fixed.
 *
 * Revision 1.10  2004/03/19 09:49:29  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.8  2003/10/15 15:30:32  strk
 * Declared a SweepLineEventOBJ from which MonotoneChain and 
 * SweepLineSegment derive to abstract SweepLineEvent object 
 * previously done on void * pointers.
 * No more compiler warnings...
 *
 **********************************************************************/

