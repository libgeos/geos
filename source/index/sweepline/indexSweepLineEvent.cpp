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
 * Revision 1.6  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/


#include <geos/indexSweepline.h>
#include <stdio.h>

namespace geos {

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
}

