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
 * Revision 1.7  2004/03/19 09:49:29  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.6  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.5  2003/10/15 16:39:03  strk
 * Made Edge::getCoordinates() return a 'const' value. Adapted code set.
 *
 **********************************************************************/


#include "../../headers/geomgraphindex.h"

namespace geos {

SimpleSweepLineIntersector::SimpleSweepLineIntersector(){
	events=new vector<SweepLineEvent*>();
	nOverlaps=0;
}

SimpleSweepLineIntersector::~SimpleSweepLineIntersector(){
	for(int i=0;i<(int)events->size();i++) {
		delete (*events)[i];
	}
	delete events;
}

void SimpleSweepLineIntersector::computeIntersections(vector<Edge*> *edges,SegmentIntersector *si,bool testAllSegments){
	if (testAllSegments)
		add(edges,NULL);
	else
		add(edges);
	computeIntersections(si);
}

void SimpleSweepLineIntersector::computeIntersections(vector<Edge*> *edges0,vector<Edge*> *edges1,SegmentIntersector *si){
	add(edges0,edges0);
	add(edges1,edges1);
	computeIntersections(si);
}

void SimpleSweepLineIntersector::add(vector<Edge*> *edges) {
	for(vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *edge=*i;
		// edge is its own group
		add(edge,edge);
	}
}

void SimpleSweepLineIntersector::add(vector<Edge*> *edges,void* edgeSet){
	for(vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *edge=*i;
		add(edge,edgeSet);
	}
}

void SimpleSweepLineIntersector::add(Edge *edge,void* edgeSet){
	const CoordinateList *pts=edge->getCoordinates();
	for(int i=0;i<pts->getSize()-1; i++) {
		SweepLineSegment *ss=new SweepLineSegment(edge,i);
		SweepLineEvent *insertEvent=new SweepLineEvent(edgeSet,ss->getMinX(),NULL,ss);
		events->push_back(insertEvent);
		events->push_back(new SweepLineEvent(edgeSet,ss->getMaxX(),insertEvent,ss));
	}
}

bool sleLessThan(SweepLineEvent *first,SweepLineEvent *second) {
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
void SimpleSweepLineIntersector::prepareEvents(){
	sort(events->begin(),events->end(),sleLessThan);
	for(int i=0;i<(int)events->size();i++ ){
		SweepLineEvent *ev=(*events)[i];
		if (ev->isDelete()){
			ev->getInsertEvent()->setDeleteEventIndex(i);
		}
	}
}

void SimpleSweepLineIntersector::computeIntersections(SegmentIntersector *si){
	nOverlaps=0;
	prepareEvents();
	for(int i=0;i<(int)events->size();i++) {
		SweepLineEvent *ev=(*events)[i];
		if (ev->isInsert()) {
			processOverlaps(i,ev->getDeleteEventIndex(),ev,si);
		}
	}
}

void SimpleSweepLineIntersector::processOverlaps(int start,int end,SweepLineEvent *ev0,
													SegmentIntersector *si){
	SweepLineSegment *ss0=(SweepLineSegment*) ev0->getObject();
	/**
	* Since we might need to test for self-intersections,
	* include current insert event object in list of event objects to test.
	* Last index can be skipped, because it must be a Delete event.
	*/
	for(int i=start;i<end;i++) {
		SweepLineEvent *ev1=(*events)[i];
		if (ev1->isInsert()) {
			SweepLineSegment *ss1=(SweepLineSegment*) ev1->getObject();
			if (ev0->edgeSet==NULL || (ev0->edgeSet!=ev1->edgeSet)) {
				ss0->computeIntersections(ss1,si);
				nOverlaps++;
			}
		}
	}
}

}

