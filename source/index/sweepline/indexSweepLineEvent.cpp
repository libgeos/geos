#include "../../headers/indexSweepline.h"
#include "stdio.h"

indexSweepLineEvent::indexSweepLineEvent(double x,indexSweepLineEvent *newInsertEvent,SweepLineInterval *newSweepInt) {
	xValue = x;
	insertEvent=newInsertEvent;
	eventType=indexSweepLineEvent::INSERT;
	if (insertEvent!=NULL)
		eventType=indexSweepLineEvent::DELETE;
	sweepInt=newSweepInt;
}

bool indexSweepLineEvent::isInsert() {
	return insertEvent==NULL;
}

bool indexSweepLineEvent::isDelete() {
	return insertEvent!=NULL;
}

indexSweepLineEvent* indexSweepLineEvent::getInsertEvent() {
	return insertEvent;
}

int indexSweepLineEvent::getDeleteEventIndex() {
	return deleteEventIndex;
}

void indexSweepLineEvent::setDeleteEventIndex(int newDeleteEventIndex) {
	deleteEventIndex=newDeleteEventIndex;
}

SweepLineInterval* indexSweepLineEvent::getInterval() {
	return sweepInt;
}

int indexSweepLineEvent::compareTo(indexSweepLineEvent *pe) {
	if (xValue<pe->xValue) return -1;
	if (xValue>pe->xValue) return 1;
	if (eventType<pe->eventType) return -1;
	if (eventType>pe->eventType) return 1;
	return 0;
}

int indexSweepLineEvent::compareTo(void *o) {
	indexSweepLineEvent *pe=(indexSweepLineEvent*) o;
	if (xValue<pe->xValue) return -1;
	if (xValue>pe->xValue) return 1;
	if (eventType<pe->eventType) return -1;
	if (eventType>pe->eventType) return 1;
	return 0;
}
