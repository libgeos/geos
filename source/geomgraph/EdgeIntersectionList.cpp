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

#include <geos/geomgraph.h>

#ifndef DEBUG
#define DEBUG 0
#endif

namespace geos {

EdgeIntersectionList::EdgeIntersectionList(Edge *newEdge)
{
	list=new vector<EdgeIntersection*>();
	edge=newEdge;
}

EdgeIntersectionList::~EdgeIntersectionList()
{
//	delete edge;
	for(int i=0;i<(int)list->size();i++) {
		delete (*list)[i];
	}
	delete list;
}

EdgeIntersection*
EdgeIntersectionList::add(const Coordinate& coord, int segmentIndex, double dist)
{
#if DEBUG
	cerr<<"["<<this<<"] EdgeIntersectionList::add("<<coord.toString()<<","<<segmentIndex<<","<<dist<<")"<<endl;
#endif // DEBUG
	vector<EdgeIntersection *>::iterator insertIt=list->begin();
	bool isInList=findInsertionPoint(segmentIndex,dist,&insertIt);
	EdgeIntersection *ei;
	if (!isInList)
	{
#if DEBUG
		cerr<<"  intersection not in list"<<endl;
#endif // DEBUG
		ei=new EdgeIntersection(coord,segmentIndex,dist);
		list->insert(insertIt,ei);
	}
	else
	{
		ei=*insertIt;
#if DEBUG
		cerr<<"  intersection already in list (should merge z)"<<endl;
		cerr<<"  current: "<<ei->coord.toString()<<endl;
		cerr<<"  intersection: "<<coord.toString()<<endl;
#endif // DEBUG
	}
	return ei;
}

vector<EdgeIntersection*>::iterator
EdgeIntersectionList::iterator()
{
	return list->begin();
}

bool
EdgeIntersectionList::isEmpty()
{
	return list->empty();
}

bool
EdgeIntersectionList::findInsertionPoint(int segmentIndex, double dist,vector<EdgeIntersection*>::iterator *insertIt)
{
	vector<EdgeIntersection *>::iterator findIt=list->begin();
	//bool found=false;
	while(findIt<list->end()) {
		EdgeIntersection *ei=*findIt;
		findIt++;
		int compare=ei->compare(segmentIndex, dist);
		// intersection found - insertIt.next() will retrieve it
		if (compare==0) return true;
		// this ei is past the intersection location, so intersection was not found
		if (compare>0) return false;
		// this ei was before the intersection point, so move to next
		(*insertIt)++;
	}
	return false;
}

bool
EdgeIntersectionList::isIntersection(const Coordinate& pt)
{
	vector<EdgeIntersection *>::iterator it;
	for (it=list->begin();it<list->end();it++) {
		EdgeIntersection *ei=*it;
		if (ei->coord==pt)
			return true;
	}
	return false;
}

void
EdgeIntersectionList::addEndpoints()
{
	int maxSegIndex=edge->pts->getSize()-1;
	add(edge->pts->getAt(0), 0, 0.0);
	add(edge->pts->getAt(maxSegIndex), maxSegIndex, 0.0);
}

void
EdgeIntersectionList::addSplitEdges(vector<Edge*> *edgeList)
{
	// ensure that the list has entries for the first and last point
	// of the edge
	addEndpoints();
	vector<EdgeIntersection *>::iterator it=list->begin();
	// there should always be at least two entries in the list
	EdgeIntersection *eiPrev=*it;
	it++;
	while (it<list->end()) {
		EdgeIntersection *ei=*it;
		Edge *newEdge=createSplitEdge(eiPrev,ei);
		edgeList->push_back(newEdge);
		eiPrev=ei;
		it++;
	}
}

Edge*
EdgeIntersectionList::createSplitEdge(EdgeIntersection *ei0, EdgeIntersection *ei1)
{
#if DEBUG
	cerr<<"["<<this<<"] EdgeIntersectionList::createSplitEdge()"<<endl;
#endif // DEBUG
	int npts=ei1->segmentIndex-ei0->segmentIndex+2;
#if DEBUG
	cerr<<"    npts:"<<npts<<endl;
#endif // DEBUG

	const Coordinate& lastSegStartPt=edge->pts->getAt(ei1->segmentIndex);

	// if the last intersection point is not equal to the its segment
	// start pt, add it to the points list as well.
	// (This check is needed because the distance metric is not totally
	// reliable!). The check for point equality is 2D only - Z values
	// are ignored
	bool useIntPt1=ei1->dist>0.0 || !ei1->coord.equals2D(lastSegStartPt);
	if (!useIntPt1) {
		npts--;
#if DEBUG
		cerr<<"    !useIntPt1 (npts:"<<npts<<")"<<endl;
#endif // DEBUG
	}
	CoordinateSequence* pts=new DefaultCoordinateSequence(npts);
	int ipt=0;
	//Coordinate *c=new Coordinate(ei0->coord);
	//pts->setAt(*c,ipt++);
	//delete c;
	pts->setAt(ei0->coord,ipt++);
#if DEBUG
	cerr<<"    pt"<<(ipt-1)<<": "<<pts->getAt(ipt-1).toString()<<endl;
#endif // DEBUG
	for(int i=ei0->segmentIndex+1; i<=ei1->segmentIndex;i++)
	{
		if ( ! useIntPt1 && ei1->segmentIndex == i )
		{
			pts->setAt(ei1->coord, ipt++);
		}
		else
		{
			pts->setAt(edge->pts->getAt(i),ipt++);
		}
#if DEBUG
		cerr<<"    pt"<<(ipt-1)<<": "<<pts->getAt(ipt-1).toString()<<endl;
#endif // DEBUG
	}
	if (useIntPt1)
	{
		pts->setAt(ei1->coord,ipt);
#if DEBUG
		cerr<<"    ustIntPt1: pt"<<(ipt-1)<<": "<<pts->getAt(ipt-1).toString()<<endl;
#endif // DEBUG
	}
	return new Edge(pts, new Label(edge->getLabel()));
}

string
EdgeIntersectionList::print()
{
	string out="Intersections: ";
	vector<EdgeIntersection *>::iterator it;
	for (it=list->begin();it<list->end();it++) {
		EdgeIntersection *ei=*it;
		out+=ei->print();
	}
	return out;
}

} // namespace

/**********************************************************************
 * $Log$
 * Revision 1.10  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
 *
 * Revision 1.9  2004/11/23 19:53:06  strk
 * Had LineIntersector compute Z by interpolation.
 *
 * Revision 1.8  2004/11/22 13:02:12  strk
 * Forced use if computed intersection point in ::createSplitEdge (for Z computation)
 *
 * Revision 1.7  2004/11/17 08:13:16  strk
 * Indentation changes.
 * Some Z_COMPUTATION activated by default.
 *
 * Revision 1.6  2004/11/04 19:08:06  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.5  2004/10/21 22:29:54  strk
 * Indentation changes and some more COMPUTE_Z rules
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
 * Revision 1.1  2004/03/19 09:48:45  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.18  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.17  2003/11/06 19:04:28  strk
 * removed useless Coordinate copy in ::createSplitEdge()
 *
 **********************************************************************/

