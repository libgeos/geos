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

#include <geos/index/sweepline/SweepLineIndex.h>
#include <geos/index/sweepline/SweepLineEvent.h>
#include <geos/index/sweepline/SweepLineInterval.h>
#include <geos/index/sweepline/SweepLineOverlapAction.h>

#include <algorithm>

using namespace std;

namespace geos {
namespace index { // geos.index
namespace sweepline { // geos.index.sweepline

SweepLineIndex::SweepLineIndex()
	:
	nOverlaps(0)
{
	//events=new vector<SweepLineEvent*>();
	//nOverlaps=0;
}

SweepLineIndex::~SweepLineIndex()
{
	//delete events;
}

void
SweepLineIndex::add(SweepLineInterval *sweepInt)
{
	// FIXME: who's going to delete the newly-created events ?
	SweepLineEvent *insertEvent=new SweepLineEvent(sweepInt->getMin(),NULL,sweepInt);
	events.push_back(insertEvent);
	events.push_back(new SweepLineEvent(sweepInt->getMax(), insertEvent, sweepInt));
}

/*private*/
void
SweepLineIndex::buildIndex()
{
	if (indexBuilt) return;

	sort(events.begin(), events.end(), SweepLineEventLessThen());
	for(unsigned int i=0, n=events.size(); i<n; i++) {
		SweepLineEvent *ev=events[i];
		if (ev->isDelete()) {
			ev->getInsertEvent()->setDeleteEventIndex(i);
		}
	}
	indexBuilt = true;
}

void
SweepLineIndex::computeOverlaps(SweepLineOverlapAction *action)
{
	nOverlaps = 0;
	buildIndex();
	for(unsigned int i=0, n=events.size(); i<n; i++) {
		SweepLineEvent *ev=events[i];
		if (ev->isInsert()) {
			processOverlaps(i, ev->getDeleteEventIndex(),
					ev->getInterval(), action);
		}
	}
}

void
SweepLineIndex::processOverlaps(int start, int end,
		SweepLineInterval *s0, SweepLineOverlapAction *action)
{
	/**
	 * Since we might need to test for self-intersections,
	 * include current insert event object in list of event objects to test.
	 * Last index can be skipped, because it must be a Delete event.
	 */
	for(int i=start; i<end; i++)
	{
		SweepLineEvent *ev=events[i];
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
 * Revision 1.9  2006/03/21 10:01:30  strk
 * indexSweepline.h header split
 *
 * Revision 1.8  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 **********************************************************************/

