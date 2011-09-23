/**********************************************************************
 * $Id: SweepLineEvent.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

#include <geos/geomgraph/index/SweepLineEvent.h>
#include <geos/geomgraph/index/SweepLineEventObj.h>

using namespace std;

namespace geos {
namespace geomgraph { // geos.geomgraph
namespace index { // geos.geomgraph.index

SweepLineEvent::SweepLineEvent(void* newEdgeSet, double x,
	SweepLineEvent *newInsertEvent, SweepLineEventOBJ *newObj):
		edgeSet(newEdgeSet),
		obj(newObj),
		xValue(x),
		insertEvent(newInsertEvent),
		deleteEventIndex(0)
{
	if(insertEvent!=NULL) eventType=DELETE_EVENT;
	else eventType=INSERT_EVENT;
}

SweepLineEvent::~SweepLineEvent(){
	if (eventType==DELETE_EVENT) {
		delete insertEvent;
		delete obj; 
	}
}

/**
 * ProjectionEvents are ordered first by their x-value, and then by their
 * eventType.
 * It is important that Insert events are sorted before Delete events, so that
 * items whose Insert and Delete events occur at the same x-value will be
 * correctly handled.
 */
int
SweepLineEvent::compareTo(SweepLineEvent *sle)
{
	if (xValue<sle->xValue) return -1;
	if (xValue>sle->xValue) return 1;
	if (eventType<sle->eventType) return -1;
	if (eventType>sle->eventType) return 1;
	return 0;
}

string
SweepLineEvent::print()
{
	ostringstream s;

	s<<"SweepLineEvent:";
	s<<" xValue="<<xValue<<" deleteEventIndex="<<deleteEventIndex;
	s<<( (eventType==INSERT_EVENT) ? " INSERT_EVENT" : " DELETE_EVENT" );
	s<<endl<<"\tinsertEvent=";
	if (insertEvent) s<<insertEvent->print();
	else s<<"NULL";
	return s.str();
}

} // namespace geos.geomgraph.index
} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/03/15 17:16:31  strk
 * streamlined headers inclusion
 *
 * Revision 1.9  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.8  2006/03/02 14:53:40  strk
 * SweepLineEvent::DELETE=>DELETE_EVENT, INSERT=>INSERT_EVENT (#45)
 *
 * Revision 1.7  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.6  2005/11/07 12:31:24  strk
 * Changed EdgeIntersectionList to use a set<> rathern then a vector<>, and
 * to avoid dynamic allocation of initial header.
 * Inlined short SweepLineEvent methods.
 *
 * Revision 1.5  2005/11/04 15:42:52  strk
 * Fixed bug in SweepLineEventLessThen functor
 * (didn't conform to strict weak ordering).
 * Note: this was introduced by previous commit.
 *
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

