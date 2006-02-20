/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/indexSweepline.h>
#include <stdio.h>

namespace geos {
namespace index { // geos.index
namespace sweepline { // geos.index.sweepline

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

} // namespace geos.index.sweepline
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.7  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
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

