/**********************************************************************
 * $Id$
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
 **********************************************************************
 *
 **********************************************************************/

#include <geos/opRelate.h>
#include <geos/geom/Coordinate.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geomgraph/EdgeEnd.h>
#include <geos/geomgraph/EdgeIntersectionList.h>
#include <geos/geomgraph/Label.h>

#include <vector>

using namespace std;
using namespace geos::geomgraph;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace relate { // geos.operation.relate

vector<EdgeEnd*> *
EdgeEndBuilder::computeEdgeEnds(vector<Edge*> *edges)
{
	vector<EdgeEnd*> *l=new vector<EdgeEnd*>();
	for(vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *e=*i;
		computeEdgeEnds(e,l);
	}
	return l;
}

/**
 * Creates stub edges for all the intersections in this
 * Edge (if any) and inserts them into the graph.
 */
void
EdgeEndBuilder::computeEdgeEnds(Edge *edge, vector<EdgeEnd*> *l)
{
	EdgeIntersectionList &eiList=edge->getEdgeIntersectionList();
	//Debug.print(eiList);
	// ensure that the list has entries for the first and last point of the edge
	eiList.addEndpoints();

	EdgeIntersectionList::iterator it=eiList.begin();
	// no intersections, so there is nothing to do
	if (it==eiList.end()) return;

	EdgeIntersection *eiPrev=NULL;
	EdgeIntersection *eiCurr=NULL;

	EdgeIntersection *eiNext=*it;
	it++;
	do {
		eiPrev=eiCurr;
		eiCurr=eiNext;
		eiNext=NULL;
		if (it!=eiList.end()) {
			eiNext=*it;
			it++;
		}
		if (eiCurr!=NULL) {
			createEdgeEndForPrev(edge,l,eiCurr,eiPrev);
			createEdgeEndForNext(edge,l,eiCurr,eiNext);
		}
	} while (eiCurr!=NULL);
}

/**
 * Create a EdgeStub for the edge before the intersection eiCurr.
 * The previous intersection is provided
 * in case it is the endpoint for the stub edge.
 * Otherwise, the previous point from the parent edge will be the endpoint.
 * 
 * eiCurr will always be an EdgeIntersection, but eiPrev may be null.
 */
void
EdgeEndBuilder::createEdgeEndForPrev(Edge *edge, vector<EdgeEnd*> *l,
		EdgeIntersection *eiCurr, EdgeIntersection *eiPrev)
{
	int iPrev=eiCurr->segmentIndex;
	if (eiCurr->dist==0.0) {
		// if at the start of the edge there is no previous edge
		if (iPrev==0) return;
		iPrev--;
	}
	Coordinate pPrev(edge->getCoordinate(iPrev));
	// if prev intersection is past the previous vertex, use it instead
	if (eiPrev!=NULL && eiPrev->segmentIndex>=iPrev)
		pPrev=eiPrev->coord; 
	Label *label=new Label(*(edge->getLabel()));
	// since edgeStub is oriented opposite to it's parent edge, have to flip sides for edge label
	label->flip();
	EdgeEnd *e=new EdgeEnd(edge,eiCurr->coord,pPrev,label);
	//e.print(System.out);  System.out.println();
	l->push_back(e);
}

/**
 * Create a StubEdge for the edge after the intersection eiCurr.
 * The next intersection is provided
 * in case it is the endpoint for the stub edge.
 * Otherwise, the next point from the parent edge will be the endpoint.
 * 
 * eiCurr will always be an EdgeIntersection, but eiNext may be null.
 */
void
EdgeEndBuilder::createEdgeEndForNext(Edge *edge, vector<EdgeEnd*> *l,
		EdgeIntersection *eiCurr, EdgeIntersection *eiNext)
{
	int iNext=eiCurr->segmentIndex + 1;
	// if there is no next edge there is nothing to do
	if (iNext>=edge->getNumPoints() && eiNext==NULL) return;
	Coordinate pNext(edge->getCoordinate(iNext));
	// if the next intersection is in the same segment as the current, use it as the endpoint
	if (eiNext!=NULL && eiNext->segmentIndex==eiCurr->segmentIndex)
		pNext=eiNext->coord; 
	EdgeEnd *e=new EdgeEnd(edge,eiCurr->coord,pNext,new Label(*(edge->getLabel())));
	//Debug.println(e);
	l->push_back(e);
}

} // namespace geos.operation.relate
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.16  2006/03/20 16:57:44  strk
 * spatialindex.h and opValid.h headers split
 *
 * Revision 1.15  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.14  2006/01/08 15:24:40  strk
 * Changed container-related typedef to class-scoped STL-like typedefs.
 * Fixed const correctness of EdgeIntersectionList::begin() and ::end() consts;
 * defined M_PI when undef as suggested by Charlie Savage.
 * Removed <stdio.h> include from GeometricShapeFactory.cpp.
 *
 * Revision 1.13  2005/11/21 16:03:20  strk
 *
 * Coordinate interface change:
 *         Removed setCoordinate call, use assignment operator
 *         instead. Provided a compile-time switch to
 *         make copy ctor and assignment operators non-inline
 *         to allow for more accurate profiling.
 *
 * Coordinate copies removal:
 *         NodeFactory::createNode() takes now a Coordinate reference
 *         rather then real value. This brings coordinate copies
 *         in the testLeaksBig.xml test from 654818 to 645991
 *         (tested in 2.1 branch). In the head branch Coordinate
 *         copies are 222198.
 *         Removed useless coordinate copies in ConvexHull
 *         operations
 *
 * STL containers heap allocations reduction:
 *         Converted many containers element from
 *         pointers to real objects.
 *         Made some use of .reserve() or size
 *         initialization when final container size is known
 *         in advance.
 *
 * Stateless classes allocations reduction:
 *         Provided ::instance() function for
 *         NodeFactories, to avoid allocating
 *         more then one (they are all
 *         stateless).
 *
 * HCoordinate improvements:
 *         Changed HCoordinate constructor by HCoordinates
 *         take reference rather then real objects.
 *         Changed HCoordinate::intersection to avoid
 *         a new allocation but rather return into a provided
 *         storage. LineIntersector changed to reflect
 *         the above change.
 *
 * Revision 1.12  2005/11/16 15:49:54  strk
 * Reduced gratuitous heap allocations.
 *
 * Revision 1.11  2005/11/14 18:14:04  strk
 * Reduced heap allocations made by TopologyLocation and Label objects.
 * Enforced const-correctness on GraphComponent.
 * Cleanups.
 *
 * Revision 1.10  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.9  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.8  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

