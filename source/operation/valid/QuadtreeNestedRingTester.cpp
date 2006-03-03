/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/valid/QuadtreeNestedRingTester.java rev. 1.11
 *
 **********************************************************************/


#include <geos/opValid.h>
#include <geos/geomgraph.h>
#include <geos/util.h>
#include <vector>

using namespace std;
using namespace geos::geomgraph;
using namespace geos::algorithm;
using namespace geos::index::quadtree;

namespace geos {
namespace operation { // geos.operation
namespace valid { // geos.operation.valid

QuadtreeNestedRingTester::QuadtreeNestedRingTester(GeometryGraph *newGraph):
	graph(newGraph),
	rings(new vector<LinearRing*>()),
	totalEnv(new Envelope()),
	qt(NULL),
	nestedPt(NULL)
{
}

QuadtreeNestedRingTester::~QuadtreeNestedRingTester()
{
	delete rings;
	delete totalEnv;
	delete qt;
}

Coordinate *
QuadtreeNestedRingTester::getNestedPoint()
{
	return nestedPt;
}

void
QuadtreeNestedRingTester::add(LinearRing *ring)
{
	rings->push_back(ring);
	const Envelope *envi=ring->getEnvelopeInternal();
	totalEnv->expandToInclude(envi);
}

bool
QuadtreeNestedRingTester::isNonNested()
{
	buildQuadtree();
	for(unsigned int i=0, ni=rings->size(); i<ni; ++i)
	{
		LinearRing *innerRing=(*rings)[i];
		const CoordinateSequence *innerRingPts=innerRing->getCoordinatesRO();
		const Envelope *envi=innerRing->getEnvelopeInternal();

		vector<void*> results;
		qt->query(envi, results);
		for(unsigned int j=0, nj=results.size(); j<nj; ++j)
		{
			LinearRing *searchRing=(LinearRing*)results[j];
			const CoordinateSequence *searchRingPts=searchRing->getCoordinatesRO();

			if (innerRing==searchRing) continue;

			const Envelope *e1=innerRing->getEnvelopeInternal();
			const Envelope *e2=searchRing->getEnvelopeInternal();
			if (!e1->intersects(e2)) continue;

			const Coordinate *innerRingPt=IsValidOp::findPtNotNode(innerRingPts, searchRing, graph);
			Assert::isTrue((innerRingPt!=NULL), "Unable to find a ring point not a node of the search ring");

			bool isInside=CGAlgorithms::isPointInRing(*innerRingPt,searchRingPts);
			if (isInside) {
				/*
				 * innerRingPt is const just because the input
				 * CoordinateSequence is const. If the input
				 * Polygon survives lifetime of this object
				 * we are safe.
				 */
				nestedPt=const_cast<Coordinate *>(innerRingPt);
				return false;
			}
		}
	}
	return true;
}

void
QuadtreeNestedRingTester::buildQuadtree()
{
	qt=new Quadtree();
	for(unsigned int i=0; i<rings->size(); ++i)
	{
		LinearRing *ring=(*rings)[i];
		const Envelope *env=ring->getEnvelopeInternal();
		qt->insert(env,ring);
	}
}

} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.19  2006/03/03 10:46:22  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.18  2006/02/23 11:54:21  strk
 * - MCIndexPointSnapper
 * - MCIndexSnapRounder
 * - SnapRounding BufferOp
 * - ScaledNoder
 * - GEOSException hierarchy cleanups
 * - SpatialIndex memory-friendly query interface
 * - GeometryGraph::getBoundaryNodes memory-friendly
 * - NodeMap::getBoundaryNodes memory-friendly
 * - Cleanups in geomgraph::Edge
 * - Added an XML test for snaprounding buffer (shows leaks, working on it)
 *
 * Revision 1.17  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.16  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.15  2006/01/31 19:07:34  strk
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
 * Revision 1.14  2005/11/04 11:04:09  strk
 * Ported revision 1.38 of IsValidOp.java (adding closed Ring checks).
 * Changed NestedRingTester classes to use Coorinate pointers
 * rather then actual objects, to speedup NULL tests.
 * Added JTS port revision when applicable.
 *
 * Revision 1.13  2004/07/27 16:35:47  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.12  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.11  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.10  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.9  2003/11/07 01:23:42  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 * Revision 1.8  2003/10/16 08:50:00  strk
 * Memory leak fixes. Improved performance by mean of more 
 * calls to new getCoordinatesRO() when applicable.
 *
 **********************************************************************/

