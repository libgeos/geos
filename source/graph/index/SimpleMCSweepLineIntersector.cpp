#include "../../headers/graphindex.h"
#include <algorithm>

SimpleMCSweepLineIntersector::SimpleMCSweepLineIntersector(){
	events=new vector<SweepLineEvent*>();
}

SimpleMCSweepLineIntersector::~SimpleMCSweepLineIntersector(){
	delete events;
}

void SimpleMCSweepLineIntersector::computeIntersections(vector<Edge*> *edges,SegmentIntersector *si,bool testAllSegments){
	if (testAllSegments)
		add(edges,NULL);
	else
		add(edges);
	computeIntersections(si);
}

void SimpleMCSweepLineIntersector::computeIntersections(vector<Edge*> *edges0,vector<Edge*> *edges1,SegmentIntersector *si){
	add(edges0,edges0);
	add(edges1,edges1);
	computeIntersections(si);
cout << "# overlapping MCs: " << nOverlaps << endl;
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
void SimpleMCSweepLineIntersector::prepareEvents(){
	sort(events->begin(),events->end(),sleLessThan);
	for(int i=0;i<(int)events->size();i++ ){
		SweepLineEvent *ev=(*events)[i];
		if (ev->isDelete()){
			ev->getInsertEvent()->setDeleteEventIndex(i);
		}
	}
}

void SimpleMCSweepLineIntersector::computeIntersections(SegmentIntersector *si){
	nOverlaps=0;
	prepareEvents();
	for(int i=0;i<(int)events->size();i++) {
		SweepLineEvent *ev=(*events)[i];
		if (ev->isInsert()) {
			processOverlaps(i,ev->getDeleteEventIndex(),ev,si);
		}
	}
}

void SimpleMCSweepLineIntersector::processOverlaps(int start,int end,SweepLineEvent *ev0,
													SegmentIntersector *si){
	MonotoneChain *mc0=(MonotoneChain*) ev0->getObject();
	/**
	* Since we might need to test for self-intersections,
	* include current insert event object in list of event objects to test.
	* Last index can be skipped, because it must be a Delete event.
	*/
	for(int i=start;i<end;i++) {
		SweepLineEvent *ev1=(*events)[i];
		if (ev1->isInsert()) {
			MonotoneChain *mc1=(MonotoneChain*) ev1->getObject();
			// don't compare edges in same group
			// null group indicates that edges should be compared
			if (ev0->edgeSet==NULL || (ev0->edgeSet!=ev1->edgeSet)) {
				mc0->computeIntersections(mc1,si);
				nOverlaps++;
			}
		}
	}
}

