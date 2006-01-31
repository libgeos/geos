/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding.h>
//#include <geos/indexQuadtree.h>
#include <geos/profiler.h>

#define DEBUG 0
//#define PROFILE 1

namespace geos {

#if PROFILE
static Profiler *profiler = Profiler::instance();
#endif

MCQuadtreeNoder::MCQuadtreeNoder():
	chains(new vector<indexMonotoneChain*>),
	index(new STRtree()),
	idCounter(0),
	nOverlaps(0)
{
}

MCQuadtreeNoder::~MCQuadtreeNoder(){
	unsigned int size=chains->size();
	for (unsigned int i=0; i<size; i++)
	{
		delete (*chains)[i];
	}
	delete chains;
	delete index;
}

vector<SegmentString*> *
MCQuadtreeNoder::node(vector<SegmentString*> *inputSegStrings)
{
#if PROFILE
	static Profile *prof = profiler->get("MCQuadtreeNoder::node(vector<SegmentString*>*)");
	prof->start();
#endif

	unsigned int size=inputSegStrings->size();
	for(unsigned int i=0; i<size;i++) {
		add((*inputSegStrings)[i]);
	}

	intersectChains();
	//System.out.println("MCQuadtreeNoder: # chain overlaps = " + nOverlaps);

	vector<SegmentString*> *nodedSegStrings=getNodedEdges(inputSegStrings);

#if PROFILE
	prof->stop();
#endif
	return nodedSegStrings;
}

void
MCQuadtreeNoder::intersectChains()
{
#if PROFILE
	static Profile *queryprof = Profiler::instance()->get("MCQuadtreeNoder::intersectChains query");
	static Profile *overlprof = Profiler::instance()->get("MCQuadtreeNoder::intersectChains computeOverlap");
#endif

	SegmentOverlapAction overlapAction(segInt);

	unsigned int chainsize=chains->size();
	for (unsigned int i=0; i<chainsize; i++) {
		indexMonotoneChain *queryChain=(*chains)[i];
#if PROFILE
		queryprof->start();
#endif
		vector<void*> *overlapChains = index->query(queryChain->getEnvelope());
#if PROFILE
		queryprof->stop();
#endif
		unsigned int overlapsize=overlapChains->size();
		for (unsigned int j=0; j<overlapsize; j++) {
			indexMonotoneChain *testChain=(indexMonotoneChain*)(*overlapChains)[j];
			/**
			 * following test makes sure we only compare each
			 * pair of chains once
			 * and that we don't compare a chain to itself
			 */
			if (testChain->getId()>queryChain->getId()) {
#if PROFILE
	overlprof->start();
#endif
				queryChain->computeOverlaps(testChain, &overlapAction);
#if PROFILE
	overlprof->stop();
#endif
				nOverlaps++;
			}
		}
		delete overlapChains;
	}

	//delete overlapAction;
}

void
MCQuadtreeNoder::add(SegmentString *segStr)
{
#if PROFILE
	static Profile *prof = profiler->get("MCQuadtreeNoder::add(SegmentString*)");
	prof->start();
#endif
	vector<indexMonotoneChain*> *segChains=MonotoneChainBuilder::getChains((CoordinateSequence*)segStr->getCoordinatesRO(),segStr);
#if DEBUG
	cerr<<"MCQuadtreeNoder: adding "<<segChains->size()<<" items in STRtree index"<<endl;
#endif
	unsigned int size=segChains->size();
	for (unsigned int i=0; i<size; i++) {
		indexMonotoneChain *mc=(*segChains)[i];
		mc->setId(idCounter++);
		index->insert(mc->getEnvelope(), mc);
		chains->push_back(mc);
	}
	delete segChains;

#if PROFILE
	prof->stop();
#endif
}

MCQuadtreeNoder::SegmentOverlapAction::SegmentOverlapAction(nodingSegmentIntersector *newSi){
	si=newSi;
}

void
MCQuadtreeNoder::SegmentOverlapAction::overlap(indexMonotoneChain *mc1, int start1, indexMonotoneChain *mc2, int start2)
{
	SegmentString *ss1=(SegmentString*) mc1->getContext();
	SegmentString *ss2=(SegmentString*) mc2->getContext();
	si->processIntersections(ss1, start1, ss2, start2);
}

} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.20  2006/01/31 19:07:34  strk
 * - Renamed DefaultCoordinateSequence to CoordinateArraySequence.
 * - Moved GetNumGeometries() and GetGeometryN() interfaces
 *   from GeometryCollection to Geometry class.
 * - Added getAt(int pos, Coordinate &to) funtion to CoordinateSequence class.
 * - Reworked automake scripts to produce a static lib for each subdir and
 *   then link all subsystem's libs togheter
 * - Moved C-API in it's own top-level dir capi/
 * - Moved source/bigtest and source/test to tests/bigtest and test/xmltester
 * - Fixed PointLocator handling of LinearRings
 * - Changed CoordinateArrayFilter to reduce memory copies
 * - Changed UniqueCoordinateArrayFilter to reduce memory copies
 * - Added CGAlgorithms::isPointInRing() version working with
 *   Coordinate::ConstVect type (faster!)
 * - Ported JTS-1.7 version of ConvexHull with big attention to
 *   memory usage optimizations.
 * - Improved XMLTester output and user interface
 * - geos::geom::util namespace used for geom/util stuff
 * - Improved memory use in geos::geom::util::PolygonExtractor
 * - New ShortCircuitedGeometryVisitor class
 * - New operation/predicate package
 *
 * Revision 1.19  2005/02/22 17:10:47  strk
 * Reduced CoordinateSequence::getSize() calls.
 *
 * Revision 1.18  2005/02/15 17:15:13  strk
 * Inlined most Envelope methods, reserved() memory for some vectors when
 * the usage was known a priori.
 *
 * Revision 1.17  2005/02/05 05:44:47  strk
 * Changed geomgraph nodeMap to use Coordinate pointers as keys, reduces
 * lots of other Coordinate copies.
 *
 * Revision 1.16  2005/02/01 16:09:46  strk
 * more profiling labels
 *
 * Revision 1.15  2005/01/31 15:41:03  strk
 * Small optimizations.
 *
 * Revision 1.14  2004/11/08 15:58:13  strk
 * More performance tuning.
 *
 * Revision 1.13  2004/11/04 19:08:07  strk
 * Cleanups, initializers list, profiling.
 *
 * Revision 1.12  2004/11/02 14:11:28  strk
 * Added more profiling.
 *
 * Revision 1.11  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.10  2004/07/13 08:33:53  strk
 * Added missing virtual destructor to virtual classes.
 * Fixed implicit unsigned int -> int casts
 *
 * Revision 1.9  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.8  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2004/06/16 13:13:25  strk
 * Changed interface of SegmentString, now copying CoordinateSequence argument.
 * Fixed memory leaks associated with this and MultiGeometry constructors.
 * Other associated fixes.
 *
 * Revision 1.6  2004/05/05 16:39:50  strk
 * reduced explicit local objects allocation
 *
 * Revision 1.5  2004/05/05 15:51:29  strk
 * Fixed big leak in intersectChains()
 *
 * Revision 1.4  2004/05/03 22:56:44  strk
 * leaks fixed, exception specification omitted.
 *
 * Revision 1.3  2004/04/19 16:14:52  strk
 * Some memory leaks plugged in noding algorithms.
 *
 * Revision 1.2  2004/04/16 12:48:07  strk
 * Leak fixes.
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/

