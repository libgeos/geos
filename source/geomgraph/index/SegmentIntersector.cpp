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
 **********************************************************************/


#include <geos/geomgraphindex.h>
#include <math.h>

#define DEBUG_INTERSECT 0

namespace geos {

bool
SegmentIntersector::isAdjacentSegments(int i1,int i2)
{
	return abs(i1-i2)==1;
}

SegmentIntersector::SegmentIntersector()
{
	hasIntersectionVar=false;
	hasProper=false;
	hasProperInterior=false;
	numIntersections=0;
	numTests=0;
	bdyNodes=NULL;
}

SegmentIntersector::~SegmentIntersector()
{
	if (bdyNodes!=NULL) {
		for(int i=0;i<(int)bdyNodes->size();i++) {
			delete (*bdyNodes)[i];
		}
		delete bdyNodes;
	}
}

SegmentIntersector::SegmentIntersector(LineIntersector *newLi,bool newIncludeProper,bool newRecordIsolated)
{
	hasIntersectionVar=false;
	hasProper=false;
	hasProperInterior=false;
	numIntersections=0;
	numTests=0;

	li=newLi;
	includeProper=newIncludeProper;
	recordIsolated=newRecordIsolated;
	bdyNodes=NULL;
}

void
SegmentIntersector::setBoundaryNodes(vector<Node*> *bdyNodes0,vector<Node*> *bdyNodes1)
{
	if (bdyNodes==NULL)
		bdyNodes=new vector<vector<Node*>*>();
	bdyNodes->resize(2);
	*(bdyNodes->begin())=bdyNodes0;
	*(bdyNodes->begin()+1)=bdyNodes1;
}

/*
 * @return the proper intersection point, or <code>null</code>
 * if none was found
 */
Coordinate&
SegmentIntersector::getProperIntersectionPoint()
{
	return properIntersectionPoint;
}

bool
SegmentIntersector::hasIntersection()
{
	return hasIntersectionVar;
}

/*
 * A proper intersection is an intersection which is interior to at least two
 * line segments.  Note that a proper intersection is not necessarily
 * in the interior of the entire Geometry, since another edge may have
 * an endpoint equal to the intersection, which according to SFS semantics
 * can result in the point being on the Boundary of the Geometry.
 */
bool
SegmentIntersector::hasProperIntersection()
{
	return hasProper;
}

/*
 * A proper interior intersection is a proper intersection which is <b>not</b>
 * contained in the set of boundary nodes set for this SegmentIntersector.
 */
bool
SegmentIntersector::hasProperInteriorIntersection()
{
	return hasProperInterior;
}

/*
 * A trivial intersection is an apparent self-intersection which in fact
 * is simply the point shared by adjacent line segments.
 * Note that closed edges require a special check for the point
 * shared by the beginning and end segments.
 */
bool
SegmentIntersector::isTrivialIntersection(Edge *e0,int segIndex0,Edge *e1,int segIndex1)
{
//	if (e0->equals(e1)) {
	if (e0==e1) {
		if (li->getIntersectionNum()==1) {
			if (isAdjacentSegments(segIndex0,segIndex1))
				return true;
			if (e0->isClosed()) {
				int maxSegIndex=e0->getNumPoints()-1;
				if ((segIndex0==0 && segIndex1==maxSegIndex)
					|| (segIndex1==0 && segIndex0==maxSegIndex)) {
					return true;
				}
			}
		}
	}
	return false;
}

/**
* This method is called by clients of the EdgeIntersector class to test for and add
* intersections for two segments of the edges being intersected.
* Note that clients (such as MonotoneChainEdges) may choose not to intersect
* certain pairs of segments for efficiency reasons.
*/
void
SegmentIntersector::addIntersections(Edge *e0,int segIndex0,Edge *e1,int segIndex1)
{

//	if (e0->equals(e1) && segIndex0==segIndex1) return;
	if (e0==e1 && segIndex0==segIndex1) return;
	numTests++;
	const CoordinateSequence* cl0=e0->getCoordinates();
	const Coordinate& p00=cl0->getAt(segIndex0);
	const Coordinate& p01=cl0->getAt(segIndex0+1);
	const CoordinateSequence* cl1=e1->getCoordinates();
	const Coordinate& p10=cl1->getAt(segIndex1);
	const Coordinate& p11=cl1->getAt(segIndex1+1);
	li->computeIntersection(p00,p01,p10,p11);

	/*
	 * Always record any non-proper intersections.
	 * If includeProper is true, record any proper intersections as well.
	 */
	if (li->hasIntersection()) {
		if (recordIsolated) {
			e0->setIsolated(false);
			e1->setIsolated(false);
		}
		//intersectionFound = true;
		numIntersections++;

		// If the segments are adjacent they have at least one trivial
		// intersection, the shared endpoint.
		// Don't bother adding it if it is the
		// only intersection.
		if (!isTrivialIntersection(e0,segIndex0,e1,segIndex1))
		{
#if DEBUG_INTERSECT
			cerr<<"SegmentIntersector::addIntersections(): has !TrivialIntersection"<<endl;
#endif // DEBUG_INTERSECT
			hasIntersectionVar=true;
			if (includeProper || !li->isProper()) {
				//Debug.println(li);
				e0->addIntersections(li,segIndex0,0);
				e1->addIntersections(li,segIndex1,1);
#if DEBUG_INTERSECT
			cerr<<"SegmentIntersector::addIntersections(): includeProper || !li->isProper()"<<endl;
#endif // DEBUG_INTERSECT
			}
			if (li->isProper())
			{
				properIntersectionPoint.setCoordinate(li->getIntersection(0));
#if DEBUG_INTERSECT
				cerr<<"SegmentIntersector::addIntersections(): properIntersectionPoint: "<<properIntersectionPoint.toString()<<endl;
#endif // DEBUG_INTERSECT
				hasProper=true;
				if (!isBoundaryPoint(li,bdyNodes))
					hasProperInterior=true;
			}
			//if (li.isCollinear())
			//hasCollinear = true;
		}
	}
}

bool
SegmentIntersector::isBoundaryPoint(LineIntersector *li,vector<vector<Node*>*> *tstBdyNodes)
{
	if (tstBdyNodes==NULL) return false;
	if (isBoundaryPoint(li,(*tstBdyNodes)[0])) return true;
	if (isBoundaryPoint(li,(*tstBdyNodes)[1])) return true;
	return false;
}

bool
SegmentIntersector::isBoundaryPoint(LineIntersector *li,vector<Node*> *tstBdyNodes)
{
	for(vector<Node*>::iterator i=tstBdyNodes->begin();i<tstBdyNodes->end();i++) {
		Node *node=*i;
		const Coordinate& pt=node->getCoordinate();
		if (li->isIntersection(pt)) return true;
	}
	return false;
}
}

/**********************************************************************
 * $Log$
 * Revision 1.6  2004/11/17 08:41:42  strk
 * Fixed a bug in Z computation and removed debugging output by default.
 *
 * Revision 1.5  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.4  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.3  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added DefaultCoordinateSequenceFactory::instance() function.
 *
 * Revision 1.2  2004/07/02 13:28:26  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/14 06:04:26  ybychkov
 * "geomgraph/index" committ problem fixed.
 *
 * Revision 1.18  2004/03/19 09:49:29  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.17  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.16  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 **********************************************************************/
