/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <sstream>
#include <geos/geomgraphindex.h>

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
	ostringstream s;

	s<<"SweepLineEvent:";
	s<<" xValue="<<xValue<<" deleteEventIndex="<<deleteEventIndex;
	s<<( (eventType==INSERT) ? " INSERT" : " DELETE" );
	s<<endl<<"\tinsertEvent=";
	if (insertEvent) s<<insertEvent->print();
	else s<<"NULL";
	return s.str();
}

bool
SweepLineEventLessThen::operator()(const SweepLineEvent *f,
	const SweepLineEvent *s) const
{
	if (f->xValue<s->xValue) return true;
	if (f->eventType<s->eventType) return true;
	return false;
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.4  2005/10/27 14:05:20  strk
 * Added a SweepLineEventLessThen functor to be used by sort algorithm.
 *
 * Revision 1.3  2005/01/28 09:47:51  strk
 * Replaced sprintf uses with ostringstream.
 *
 * Revision 1.2  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/14 06:04:26  ybychkov
 * "geomgraph/index" committ problem fixed.
 *
 * Revision 1.13  2004/03/19 09:49:29  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.12  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.11  2003/10/15 15:30:32  strk
 * Declared a SweepLineEventOBJ from which MonotoneChain and 
 * SweepLineSegment derive to abstract SweepLineEvent object 
 * previously done on void * pointers.
 * No more compiler warnings...
 *
 **********************************************************************/

