/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#ifndef GEOS_INDEX_SWEEPLINE_SWEEPLINEINDEX_H
#define GEOS_INDEX_SWEEPLINE_SWEEPLINEINDEX_H

#include <vector>

// Forward declarations
namespace geos {
	namespace index { 
		namespace sweepline {
			class SweepLineInterval;
			class SweepLineEvent;
			class SweepLineOverlapAction;
		}
	}
}

namespace geos {
namespace index { // geos.index
namespace sweepline { // geos:index:sweepline

/** \brief
 * A sweepline implements a sorted index on a set of intervals.
 *
 * It is used to compute all overlaps between the interval in the index.
 */
class SweepLineIndex {

public:

	SweepLineIndex();

	~SweepLineIndex();

	void add(SweepLineInterval *sweepInt);

	void computeOverlaps(SweepLineOverlapAction *action);

private:

	// FIXME: make it a real vector rather then a pointer
	std::vector<SweepLineEvent*> events;

	bool indexBuilt;

	// statistics information
	int nOverlaps;

	/**
	 * Because Delete Events have a link to their corresponding Insert event,
	 * it is possible to compute exactly the range of events which must be
	 * compared to a given Insert event object.
	 */
	void buildIndex();

	void processOverlaps(int start, int end,
			SweepLineInterval *s0,
			SweepLineOverlapAction *action);
};


} // namespace geos:index:sweepline
} // namespace geos:index
} // namespace geos

#endif // GEOS_INDEX_SWEEPLINE_SWEEPLINEINDEX_H

/**********************************************************************
 * $Log$
 * Revision 1.1  2006/03/21 10:01:30  strk
 * indexSweepline.h header split
 *
 **********************************************************************/

