#include "../../headers/graphindex.h"

namespace geos {

SimpleSweepLineIntersector::SimpleSweepLineIntersector(){
	events=new vector<SweepLineEvent*>();
	nOverlaps=0;
}

SimpleSweepLineIntersector::~SimpleSweepLineIntersector(){
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
cout << "# overlapping MCs: " << nOverlaps << endl;
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
	CoordinateList *pts=edge->getCoordinates();
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