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

#ifndef GEOS_INDEXSWEEPLINE_H
#define GEOS_INDEXSWEEPLINE_H

#include <memory>
#include <vector>
#include <geos/platform.h>


namespace geos {

/// Provides classes for various kinds of spatial indexes.
namespace index { // geos.index

/// \brief
/// Contains classes which implement a sweepline algorithm
/// for scanning geometric data structures.
///
namespace sweepline { // geos.index.sweepline

// forward declarations
class indexSweepLineEvent;
class SweepLineOverlapAction;
class SweepLineInterval;

// temp typedefs aimed at a closer mapping to JTS
typedef indexSweepLineEvent SweepLineEvent;

class SweepLineInterval {
public:
	SweepLineInterval(double newMin, double newMax);
	SweepLineInterval(double newMin, double newMax, void* newItem);
	double getMin();
	double getMax();
	void* getItem();
private:
	double min, max;
	void* item;
};

class SweepLineOverlapAction {
public:
	virtual void overlap(SweepLineInterval *s0,SweepLineInterval *s1)=0;
};

class indexSweepLineEvent {
public:
	enum {
		INSERT = 1,
		DELETE
	};
	indexSweepLineEvent(double x,indexSweepLineEvent *newInsertEvent,SweepLineInterval *newSweepInt);
	bool isInsert();
	bool isDelete();
	indexSweepLineEvent* getInsertEvent();
	int getDeleteEventIndex();
	void setDeleteEventIndex(int newDeleteEventIndex);
	SweepLineInterval* getInterval();
	/**
	* ProjectionEvents are ordered first by their x-value, and then by their eventType.
	* It is important that Insert events are sorted before Delete events, so that
	* items whose Insert and Delete events occur at the same x-value will be
	* correctly handled.
	*/
	int compareTo(indexSweepLineEvent *pe);
	int compareTo(void *o);
private:
	double xValue;
	int eventType;
	indexSweepLineEvent *insertEvent; // null if this is an INSERT event
	int deleteEventIndex;
	SweepLineInterval *sweepInt;
};

/*
 * A sweepline implements a sorted index on a set of intervals.
 * It is used to compute all overlaps between the interval in the index.
 */
class SweepLineIndex {
public:
	SweepLineIndex();
	~SweepLineIndex();
	void add(SweepLineInterval *sweepInt);
	void computeOverlaps(SweepLineOverlapAction *action);
private:
	std::vector<indexSweepLineEvent*> *events;
	bool indexBuilt;
	// statistics information
	int nOverlaps;
	/**
	* Because Delete Events have a link to their corresponding Insert event,
	* it is possible to compute exactly the range of events which must be
	* compared to a given Insert event object.
	*/
	void buildIndex();
	void processOverlaps(int start,int end,SweepLineInterval *s0,SweepLineOverlapAction *action);
};

bool isleLessThen(indexSweepLineEvent *first,indexSweepLineEvent *second);

} // namespace geos.index.sweepline
} // namespace geos.index
} // namespace geos

#endif

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.2  2004/07/19 13:19:31  strk
 * Documentation fixes
 *
 * Revision 1.1  2004/07/02 13:20:42  strk
 * Header files moved under geos/ dir.
 *
 * Revision 1.5  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

