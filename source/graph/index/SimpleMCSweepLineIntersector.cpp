#include "graphindex.h"
#include <algorithm>

SimpleMCSweepLineIntersector::SimpleMCSweepLineIntersector(){}

void SimpleMCSweepLineIntersector::computeIntersections(vector<Edge*> *edges,SegmentIntersector *si){
	add(edges,0);
	computeIntersections(si,false);
}

void SimpleMCSweepLineIntersector::computeIntersections(vector<Edge*> *edges0,vector<Edge*> *edges1,SegmentIntersector *si){
	add(edges0,0);
	add(edges1,1);
	computeIntersections(si,true);
cout << "# overlapping MCs: " << nOverlaps << endl;
}

void SimpleMCSweepLineIntersector::add(vector<Edge*> *edges,int geomIndex){
	for(vector<Edge*>::iterator i=edges->begin();i<edges->end();i++) {
		Edge *edge=*i;
		add(edge,geomIndex);
	}
}

void SimpleMCSweepLineIntersector::add(Edge *edge,int geomIndex){
	MonotoneChainEdge *mce=edge->getMonotoneChainEdge();
	vector<int> startIndex(mce->getStartIndexes());
	for(int i=0;i<(int)startIndex.size()-1;i++) {
		MonotoneChain *mc=new MonotoneChain(mce,i,geomIndex);
		SweepLineEvent *insertEvent=new SweepLineEvent(geomIndex,mce->getMinX(i),NULL,mc);
		events.push_back(insertEvent);
		SweepLineEvent *deleteEvent=new SweepLineEvent(geomIndex,mce->getMaxX(i),insertEvent,mc);
		events.push_back(deleteEvent);
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
	sort(events.begin(),events.end(),sleLessThan);
int numdel=0;
	for(int i=0;i<(int)events.size();i++ ){
		SweepLineEvent *ev=events[i];
		if (ev->isDelete()){
			numdel++;
			SweepLineEvent *iev=ev->getInsertEvent();
			iev->setDeleteEventIndex(i);
		}
	}
	numdel++;
}

void SimpleMCSweepLineIntersector::computeIntersections(SegmentIntersector *si,bool doMutualOnly){
	nOverlaps=0;
	prepareEvents();
	int numov=0;
	for(int i=0;i<(int)events.size();i++) {
		SweepLineEvent *ev=events[i];
		MonotoneChain *mc=(MonotoneChain*)ev->getObject();
		if (ev->isInsert()) {
			numov++;
			processOverlaps(i,ev->getDeleteEventIndex(),mc,si,doMutualOnly);
		}
	}
	numov++;
}

void SimpleMCSweepLineIntersector::processOverlaps(int start,int end,MonotoneChain *mc0,
													SegmentIntersector *si,bool doMutualOnly){
	/**
	* Since we might need to test for self-intersections,
	* include current insert event object in list of event objects to test.
	* Last index can be skipped, because it must be a Delete event.
	*/
	for(int i=start;i<end;i++) {
		SweepLineEvent *ev=events[i];
		if (ev->isInsert()) {
			MonotoneChain *mc1=(MonotoneChain*) ev->getObject();
			if (!doMutualOnly || (mc0->getGeomIndex()!=mc1->getGeomIndex())) {
				mc0->computeIntersections(mc1,si);
				nOverlaps++;
			}
		}
	}
}

