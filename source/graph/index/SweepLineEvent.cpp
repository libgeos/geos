/*
* $Log$
* Revision 1.11  2003/10/15 15:30:32  strk
* Declared a SweepLineEventOBJ from which MonotoneChain and SweepLineSegment
* derive to abstract SweepLineEvent object previously done on void * pointers.
* No more compiler warnings...
*
*/
#include <stdio.h>
#include "../../headers/graphindex.h"

namespace geos {

SweepLineEvent::SweepLineEvent(void* newEdgeSet,double x,SweepLineEvent *newInsertEvent,SweepLineEventOBJ *newObj){
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

SweepLineEventOBJ* SweepLineEvent::getObject() const {
	return obj;
}

/**
* ProjectionEvents are ordered first by their x-value, and then by their eventType.
* It is important that Insert events are sorted before Delete events, so that
* items whose Insert and Delete events occur at the same x-value will be
* correctly handled.
*/
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

