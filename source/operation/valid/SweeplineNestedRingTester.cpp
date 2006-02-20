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
 **********************************************************************/

#include <geos/opValid.h>
#include <geos/geosAlgorithm.h>
#include <geos/indexSweepline.h>
#include <geos/util.h>

using namespace geos::algorithm;
using namespace geos::index::sweepline;

namespace geos {
namespace operation { // geos.operation
namespace valid { // geos.operation.valid

SweeplineNestedRingTester::OverlapAction::OverlapAction(SweeplineNestedRingTester *p)
{
	isNonNested=true;
	parent=p;
}

void
SweeplineNestedRingTester::OverlapAction::overlap(SweepLineInterval *s0, SweepLineInterval *s1)
{
	LinearRing *innerRing=(LinearRing*) s0->getItem();
	LinearRing *searchRing=(LinearRing*) s1->getItem();
	if (innerRing==searchRing) return;
	if (parent->isInside(innerRing,searchRing))
		isNonNested=false;
};


bool
SweeplineNestedRingTester::isNonNested()
{
	buildIndex();
	OverlapAction *action=new OverlapAction(this);
	sweepLine->computeOverlaps(action);
	return action->isNonNested;
}

void
SweeplineNestedRingTester::buildIndex()
{
	sweepLine=new SweepLineIndex();
	for(unsigned int i=0, n=rings.size(); i<n; i++) {
		LinearRing *ring=rings[i];
		const Envelope *env=ring->getEnvelopeInternal();
		SweepLineInterval *sweepInt=new SweepLineInterval(env->getMinX(),env->getMaxX(),ring);
		sweepLine->add(sweepInt);
	}
}

bool
SweeplineNestedRingTester::isInside(LinearRing *innerRing,LinearRing *searchRing)
{
	CoordinateSequence *innerRingPts=innerRing->getCoordinates();
	CoordinateSequence *searchRingPts=searchRing->getCoordinates();

	if (!innerRing->getEnvelopeInternal()->intersects(searchRing->getEnvelopeInternal()))
		return false;
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
		return true;
	}
	return false;
}

} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.14  2006/02/20 10:14:18  strk
 * - namespaces geos::index::*
 * - Doxygen documentation cleanup
 *
 * Revision 1.13  2006/02/19 19:46:50  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.12  2006/01/31 19:07:34  strk
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
 * Revision 1.11  2005/11/04 11:04:09  strk
 * Ported revision 1.38 of IsValidOp.java (adding closed Ring checks).
 * Changed NestedRingTester classes to use Coorinate pointers
 * rather then actual objects, to speedup NULL tests.
 * Added JTS port revision when applicable.
 *
 * Revision 1.10  2004/07/27 16:35:47  strk
 * Geometry::getEnvelopeInternal() changed to return a const Envelope *.
 * This should reduce object copies as once computed the envelope of a
 * geometry remains the same.
 *
 * Revision 1.9  2004/07/08 19:34:50  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.8  2004/07/02 13:28:29  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.7  2004/03/29 06:59:25  ybychkov
 * "noding/snapround" package ported (JTS 1.4);
 * "operation", "operation/valid", "operation/relate" and "operation/overlay" upgraded to JTS 1.4;
 * "geom" partially upgraded.
 *
 * Revision 1.6  2003/11/07 01:23:43  pramsey
 * Add standard CVS headers licence notices and copyrights to all cpp and h
 * files.
 *
 *
 **********************************************************************/

