#include <stdio.h>
#include "../../headers/graphindex.h"

namespace geos {

SweepLineEvent::SweepLineEvent(void* newEdgeSet,double x,SweepLineEvent *newInsertEvent,void *newObj){
	edgeSet=newEdgeSet;
	xValue=x;
	insertEvent=newInsertEvent;
	eventType=INSERT;
	if(insertEvent!=NULL)
		eventType=DELETE;
	obj=newObj;
	deleteEventIndex=0;
}

SweepLineEvent::~SweepLineEvent(){
	if (eventType==DELETE) {
		delete insertEvent;
		delete obj;
	}
}

bool SweepLineEvent::isInsert() {
	return insertEvent==NULL;
}

bool SweepLineEvent::isDelete() {
	return insertEvent!=NULL;
}

SweepLineEvent* SweepLineEvent::getInsertEvent() {
	return insertEvent;
}

int SweepLineEvent::getDeleteEventIndex() {
	return deleteEventIndex;
}

void SweepLineEvent::setDeleteEventIndex(int newDeleteEventIndex) {
	deleteEventIndex=newDeleteEventIndex;
}

void* SweepLineEvent::getObject() {
	return obj;
}

/**
* ProjectionEvents are ordered first by their x-value, and then by their eventType.
* It is important that Insert events are sorted before Delete events, so that
* items whose Insert and Delete events occur at the same x-value will be
* correctly handled.
*/
int SweepLineEvent::compareTo(void *o) {
	SweepLineEvent *sle=(SweepLineEvent*) o;
	if (xValue<sle->xValue) return -1;
	if (xValue>sle->xValue) return 1;
	if (eventType<sle->eventType) return -1;
	if (eventType>sle->eventType) return 1;
	return 0;
}

int SweepLineEvent::compareTo(SweepLineEvent *sle) {
	if (xValue<sle->xValue) return -1;
	if (xValue>sle->xValue) return 1;
	if (eventType<sle->eventType) return -1;
	if (eventType>sle->eventType) return 1;
	return 0;
}

string SweepLineEvent::print() {
	char buffer[255];
	string out="SweepLineEvent:";
	sprintf(buffer," xValue=%g deleteEventIndex=%i",xValue,deleteEventIndex);
	out.append(buffer);
	out+=((eventType==INSERT)? " INSERT" : " DELETE");
	if (insertEvent!=NULL)
		out+="\n\tinsertEvent="+insertEvent->print();
	else 
		out+="\n\tinsertEvent=NULL";
	return out;
}

}