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
#include <algorithm>

using namespace std;

namespace geos {
namespace index { // geos.index
namespace sweepline { // geos.index.sweepline

SweepLineIndex::SweepLineIndex() {
	events=new vector<indexSweepLineEvent*>();
	nOverlaps=0;
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

} // namespace geos.index.sweepline
} // namespace geos.index
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.8  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.7  2004/12/08 13:54:43  strk
 * gcc warnings checked and fixed, general cleanups.
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

