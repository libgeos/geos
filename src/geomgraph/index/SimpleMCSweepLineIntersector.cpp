/**********************************************************************
 * $Id: SimpleMCSweepLineIntersector.cpp 1820 2006-09-06 16:54:23Z mloskot $
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

#include <algorithm>
#include <vector>

#include <geos/geomgraph/index/SimpleMCSweepLineIntersector.h>
#include <geos/geomgraph/index/MonotoneChainEdge.h>
#include <geos/geomgraph/index/MonotoneChain.h>
#include <geos/geomgraph/index/SweepLineEvent.h>
#include <geos/geomgraph/Edge.h>

using namespace std;

namespace geos {
namespace geomgraph { // geos.geomgraph
namespace index { // geos.geomgraph.index

SimpleMCSweepLineIntersector::SimpleMCSweepLineIntersector()
	//events(new vector<SweepLineEvent*>())
{
}

SimpleMCSweepLineIntersector::~SimpleMCSweepLineIntersector()
{
	for(size_t i=0; i<events.size(); ++i)
	{
		SweepLineEvent *sle=events[i];
		if (sle->isDelete()) delete sle;
	}
	//delete events;
}

void
SimpleMCSweepLineIntersector::computeIntersections(vector<Edge*> *edges,
	SegmentIntersector *si, bool testAllSegments)
{
	if (testAllSegments)
		add(edges,NULL);
	else
		add(edges);
	computeIntersections(si);
}

void
SimpleMCSweepLineIntersector::computeIntersections(vector<Edge*> *edges0,
	vector<Edge*> *edges1, SegmentIntersector *si)
{
	add(edges0,edges0);
	add(edges1,edges1);
	computeIntersections(si);
}

void
SimpleMCSweepLineIntersector::add(vector<Edge*> *edges)
{
	for (size_t i=0; i<edges->size(); ++i)
	{
		Edge *edge=(*edges)[i];
		// edge is its own group
		add(edge, edge);
	}
}

void
SimpleMCSweepLineIntersector::add(vector<Edge*> *edges,void* edgeSet)
{
	for (size_t i=0; i<edges->size(); ++i)
	{
		Edge *edge=(*edges)[i];
		add(edge,edgeSet);
	}
}

void
SimpleMCSweepLineIntersector::add(Edge *edge, void* edgeSet)
{
	MonotoneChainEdge *mce=edge->getMonotoneChainEdge();
	vector<int> &startIndex=mce->getStartIndexes();
	size_t n = startIndex.size()-1;
	events.reserve(events.size()+(n*2));
	for(size_t i=0; i<n; ++i)
	{
		MonotoneChain *mc=new MonotoneChain(mce,i);
		SweepLineEvent *insertEvent=new SweepLineEvent(edgeSet,mce->getMinX(i),NULL,mc);
		events.push_back(insertEvent);
		events.push_back(new SweepLineEvent(edgeSet,mce->getMaxX(i),insertEvent,mc));
	}
}

/**
 * Because Delete Events have a link to their corresponding Insert event,
 * it is possible to compute exactly the range of events which must be
 * compared to a given Insert event object.
 */
void
SimpleMCSweepLineIntersector::prepareEvents()
{
	sort(events.begin(), events.end(), SweepLineEventLessThen());
	for(size_t i=0; i<events.size(); ++i)
	{
		SweepLineEvent *ev=events[i];
		if (ev->isDelete())
		{
			ev->getInsertEvent()->setDeleteEventIndex(i);
		}
	}
}

void
SimpleMCSweepLineIntersector::computeIntersections(SegmentIntersector *si)
{
	nOverlaps=0;
	prepareEvents();
	for(size_t i=0; i<events.size(); ++i)
	{
		SweepLineEvent *ev=events[i];
		if (ev->isInsert())
		{
			processOverlaps(i,ev->getDeleteEventIndex(),ev,si);
		}
	}
}

void
SimpleMCSweepLineIntersector::processOverlaps(int start, int end,
	SweepLineEvent *ev0, SegmentIntersector *si)
{
	MonotoneChain *mc0=(MonotoneChain*) ev0->getObject();

	/*
	 * Since we might need to test for self-intersections,
	 * include current insert event object in list of event objects to test.
	 * Last index can be skipped, because it must be a Delete event.
	 */
	for(int i=start; i<end; ++i)
	{
		SweepLineEvent *ev1=events[i];
		if (ev1->isInsert())
		{
			MonotoneChain *mc1=(MonotoneChain*) ev1->getObject();
			// don't compare edges in same group
			// null group indicates that edges should be compared
			if (ev0->edgeSet==NULL || (ev0->edgeSet!=ev1->edgeSet))
			{
				mc0->computeIntersections(mc1,si);
				nOverlaps++;
			}
		}
	}
}

} // namespace geos.geomgraph.index
} // namespace geos.geomgraph
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.13  2006/06/12 11:29:23  strk
 * unsigned int => size_t
 *
 * Revision 1.12  2006/03/15 17:16:31  strk
 * streamlined headers inclusion
 *
 * Revision 1.11  2006/03/09 16:46:47  strk
 * geos::geom namespace definition, first pass at headers split
 *
 * Revision 1.10  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.9  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.8  2005/11/15 10:04:37  strk
 *
 * Reduced heap allocations (vectors, mostly).
 * Enforced const-correctness, changed some interfaces
 * to use references rather then pointers when appropriate.
 *
 * Revision 1.7  2005/11/03 21:28:06  strk
 * Fixed constructors broke by previous commit
 *
 * Revision 1.6  2005/11/03 19:51:28  strk
 * Indentation changes, small vector memory allocation optimization.
 *
 * Revision 1.5  2005/10/27 14:05:19  strk
 * Added a SweepLineEventLessThen functor to be used by sort algorithm.
 *
 * Revision 1.4  2004/11/22 11:34:49  strk
 * More debugging lines and comments/indentation cleanups
 *
 * Revision 1.3  2004/10/20 17:32:14  strk
 * Initial approach to 2.5d intersection()
 *
 * Revision 1.2  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/04/14 06:04:26  ybychkov
 * "geomgraph/index" committ problem fixed.
 *
 * Revision 1.17  2004/03/19 09:49:29  ybychkov
 * "geomgraph" and "geomgraph/indexl" upgraded to JTS 1.4
 *
 * Revision 1.16  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

