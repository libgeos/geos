#include "../../headers/indexSweepline.h"
#include "stdio.h"
#include <algorithm>

SweepLineIndex::SweepLineIndex() {
	events=new vector<indexSweepLineEvent*>();
	int nOverlaps=0;
}

SweepLineIndex::~SweepLineIndex() {
	delete events;
}

void SweepLineIndex::add(SweepLineInterval *sweepInt) {
	indexSweepLineEvent *insertEvent=new indexSweepLineEvent(sweepInt->getMin(),NULL,sweepInt);
	events->push_back(insertEvent);
	events->push_back(new indexSweepLineEvent(sweepInt->getMax(),insertEvent,sweepInt));
}

bool isleLessThan(indexSweepLineEvent *first,indexSweepLineEvent *second) {
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
void SweepLineIndex::buildIndex() {
	if (indexBuilt) return;
	sort(events->begin(),events->end(),isleLessThan);
	for(int i=0;i<(int)events->size();i++) {
		indexSweepLineEvent *ev=(indexSweepLineEvent*)(*events)[i];
		if (ev->isDelete()) {
			ev->getInsertEvent()->setDeleteEventIndex(i);
		}
	}
	indexBuilt = true;
}

void SweepLineIndex::computeOverlaps(SweepLineOverlapAction *action) {
	nOverlaps = 0;
	buildIndex();
	for(int i=0;i<(int)events->size();i++) {
		indexSweepLineEvent *ev=(indexSweepLineEvent*)(*events)[i];
		if (ev->isInsert()) {
			processOverlaps(i,ev->getDeleteEventIndex(),ev->getInterval(),action);
		}
	}
}

void SweepLineIndex::processOverlaps(int start,int end,SweepLineInterval *s0,SweepLineOverlapAction *action) {
	/**
	* Since we might need to test for self-intersections,
	* include current insert event object in list of event objects to test.
	* Last index can be skipped, because it must be a Delete event.
	*/
	for(int i=start;i<end;i++) {
		indexSweepLineEvent *ev=(indexSweepLineEvent*)(*events)[i];
		if (ev->isInsert()) {
			SweepLineInterval *s1=ev->getInterval();
			action->overlap(s0,s1);
			nOverlaps++;
		}
	}
}