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


#include <geos/geomgraphindex.h>

namespace geos {

SweepLineSegment::SweepLineSegment(Edge *newEdge,int newPtIndex){
	pts=newEdge->getCoordinates();
	edge=newEdge;
	ptIndex=newPtIndex;
}

SweepLineSegment::~SweepLineSegment() {
}

double SweepLineSegment::getMinX(){
	double x1=pts->getAt(ptIndex).x;
	double x2=pts->getAt(ptIndex+1).x;
	return x1<x2?x1:x2;
}

double SweepLineSegment::getMaxX() {
	double x1=pts->getAt(ptIndex).x;
	double x2=pts->getAt(ptIndex+1).x;
	return x1>x2?x1:x2;
}

void SweepLineSegment::computeIntersections(SweepLineSegment *ss,SegmentIntersector *si) {
	si->addIntersections(edge,ptIndex,ss->edge,ss->ptIndex);
}
}

