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
 * Revision 1.3  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.2  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/14 06:04:26  ybychkov
 * "geomgraph/index" committ problem fixed.
 *
 * Revision 1.17  2004/03/19 09:49:29  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.16  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/geomgraphindex.h>
#include <algorithm>

namespace geos {

SimpleMCSweepLineIntersector::SimpleMCSweepLineIntersector(){
	events=new vector<SweepLineEvent*>();
}

SimpleMCSweepLineIntersector::~SimpleMCSweepLineIntersector(){
	for(int i=0;i<(int)events->size();i++) {
		SweepLineEvent *sle=(*events)[i];
		if (sle->isDelete()) delete sle;
	}
	delete events;
}

void
SimpleMCSweepLineIntersector::computeIntersections(vector<Edge*> *edges, SegmentIntersector *si, bool testAllSegments)
{
	if (testAllSegments)
		add(edges,NULL);
	else
		add(edges);
	computeIntersections(si);
}

void
SimpleMCSweepLineIntersector::computeIntersections(vector<Edge*> *edges0, vector<Edge*> *edges1, SegmentIntersector *si)
{
	add(edges0,edges0);
	add(edges1,edges1);
	computeIntersections(si);
}

void SimpleMCSweepLineIntersector::add(vector<Edge*> *edges) {
	for(vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *edge=*i;
		// edge is its own group
		add(edge,edge);
	}
}

void SimpleMCSweepLineIntersector::add(vector<Edge*> *edges,void* edgeSet){
	for(vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *edge=*i;
		add(edge,edgeSet);
	}
}

void SimpleMCSweepLineIntersector::add(Edge *edge,void* edgeSet){
	MonotoneChainEdge *mce=edge->getMonotoneChainEdge();
	vector<int>* startIndex=mce->getStartIndexes();
	for(int i=0;i<(int)startIndex->size()-1;i++) {
		MonotoneChain *mc=new MonotoneChain(mce,i);
		SweepLineEvent *insertEvent=new SweepLineEvent(edgeSet,mce->getMinX(i),NULL,mc);
		events->push_back(insertEvent);
		events->push_back(new SweepLineEvent(edgeSet,mce->getMaxX(i),insertEvent,mc));
	}
}

bool MCsleLessThan(SweepLineEvent *first,SweepLineEvent *second) {
	if (first->compareTo(second)<0)
		return true;
	else
		return false;
}

/**
* Because Delete Events have a link to their corresponding Insert event,
* it is possible to compute exactly the range of events which must be
* compared to a given Insert event object.
*/
void SimpleMCSweepLineIntersector::prepareEvents(){
	sort(events->begin(),events->end(),MCsleLessThan);
	for(int i=0;i<(int)events->size();i++ ){
		SweepLineEvent *ev=(*events)[i];
		if (ev->isDelete()){
			ev->getInsertEvent()->setDeleteEventIndex(i);
		}
	}
}

void
SimpleMCSweepLineIntersector::computeIntersections(SegmentIntersector *si)
{
	nOverlaps=0;
	prepareEvents();
	for(int i=0;i<(int)events->size();i++) {
		SweepLineEvent *ev=(*events)[i];
		if (ev->isInsert()) {
			processOverlaps(i,ev->getDeleteEventIndex(),ev,si);
		}
	}
}

void
SimpleMCSweepLineIntersector::processOverlaps(int start, int end,
	SweepLineEvent *ev0, SegmentIntersector *si)
{
	MonotoneChain *mc0=(MonotoneChain*) ev0->getObject();
	/*
	 * Since we might need to test for self-intersections,
	 * include current insert event object in list of event objects to test.
	 * Last index can be skipped, because it must be a Delete event.
	 */
	for(int i=start; i<end; i++) {
		SweepLineEvent *ev1=(*events)[i];
		if (ev1->isInsert()) {
			MonotoneChain *mc1=(MonotoneChain*) ev1->getObject();
			// don't compare edges in same group
			// null group indicates that edges should be compared
			if (ev0->edgeSet==NULL || (ev0->edgeSet!=ev1->edgeSet))
			{
				mc0->computeIntersections(mc1,si);
				nOverlaps++;
			}
		}
	}
}

}

