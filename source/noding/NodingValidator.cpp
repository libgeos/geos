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
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 ********************************************************************** 
 *
 * Last port: noding/NodingValidator.java rev. 1.6 (JTS-1.7)
 *
 **********************************************************************/

#include <sstream>
#include <geos/noding.h>
#include <util.h>

namespace geos {

/*public*/
void
NodingValidator::checkValid() 
{
	checkEndPtVertexIntersections();
	checkInteriorIntersections();
	checkCollapses();
}

/*private*/
void
NodingValidator::checkCollapses() const
{
	for (SegmentString::NonConstVect::const_iterator
		it = segStrings.begin(), itEnd = segStrings.end();
		it != itEnd;
		++it)
	{
		const SegmentString* ss = *it;
		checkCollapses(*ss);
	}
}

/* private */
void
NodingValidator::checkCollapses(const SegmentString& ss) const
{
	const CoordinateSequence& pts = *(ss.getCoordinatesRO());
	for (unsigned int i=0, n=pts.getSize()-2; i<n; ++i)
	{
		checkCollapse(pts[i], pts[i + 1], pts[i + 2]);
	}
}

/* private */
void
NodingValidator::checkCollapse(const Coordinate& p0,
		const Coordinate& p1, const Coordinate& p2) const
{
	if (p0.equals2D(p2))
		throw GEOSException("found non-noded collapse at " +
			p0.toString() + ", " +
			p1.toString() + ", " +
			p2.toString());
}

/*private*/
void
NodingValidator::checkInteriorIntersections() 
{
	for (SegmentString::NonConstVect::const_iterator
		it = segStrings.begin(), itEnd = segStrings.end();
		it != itEnd;
		++it)
	{
		SegmentString* ss0 = *it;
		for (SegmentString::NonConstVect::const_iterator
			j = segStrings.begin(), jEnd = segStrings.end();
			j != jEnd; ++j)
		{
        		const SegmentString* ss1 = *j;
			checkInteriorIntersections(*ss0, *ss1);
		}
	}
 
}

/* private */
void
NodingValidator::checkInteriorIntersections(const SegmentString& ss0,
		const SegmentString& ss1) 
{
	const CoordinateSequence& pts0 = *(ss0.getCoordinatesRO());
	const CoordinateSequence& pts1 = *(ss1.getCoordinatesRO());
	for (unsigned int i0=0, n0=pts0.size(); i0<n0-1; i0++) {
		for (unsigned int i1=0, n1=pts1.size(); i1<n1-1; i1++) {
			checkInteriorIntersections(ss0, i0, ss1, i1);
		}
	}
}


/* private */
void
NodingValidator::checkInteriorIntersections(
		const SegmentString& e0, unsigned int segIndex0,
		const SegmentString& e1, unsigned int segIndex1) 
{
	if (&e0 == &e1 && segIndex0 == segIndex1) return;
	const Coordinate& p00 = e0.getCoordinatesRO()->getAt(segIndex0);
	const Coordinate& p01 = e0.getCoordinatesRO()->getAt(segIndex0 + 1);
	const Coordinate& p10 = e1.getCoordinatesRO()->getAt(segIndex1);
	const Coordinate& p11 = e1.getCoordinatesRO()->getAt(segIndex1 + 1);

	li.computeIntersection(p00, p01, p10, p11);
	if (li.hasIntersection()) {
		if (li.isProper()
			|| hasInteriorIntersection(li, p00, p01)
			|| hasInteriorIntersection(li, p10, p11))
		{
			throw GEOSException(
				"found non-noded intersection at "
				+ p00.toString() + "-" + p01.toString()
				+ " and "
				+ p10.toString() + "-" + p11.toString());
		}
	}
}

/* private */
void
NodingValidator::checkEndPtVertexIntersections() const
{
	for (SegmentString::NonConstVect::const_iterator
		it = segStrings.begin(), itEnd = segStrings.end();
		it != itEnd;
		++it)
	{
		const SegmentString* ss = *it;
		const CoordinateSequence& pts = *(ss->getCoordinatesRO());
		checkEndPtVertexIntersections(pts[0], segStrings);
		checkEndPtVertexIntersections(pts[pts.size() - 1], segStrings);
	}
}

/* private */
void
NodingValidator::checkEndPtVertexIntersections(const Coordinate& testPt,
		const SegmentString::NonConstVect& segStrings) const
{
	for (SegmentString::NonConstVect::const_iterator
		it = segStrings.begin(), itEnd = segStrings.end();
		it != itEnd;
		++it)
	{
		const SegmentString* ss0 = *it;
		const CoordinateSequence& pts = *(ss0->getCoordinatesRO());
		for (unsigned int j=1, n=pts.size()-1; j<n; ++j)
		{
			if (pts[j].equals(testPt))
			{
				stringstream s;
				s<<"found endpt/interior pt intersection ";
				s<<"at index "<<j<<" :pt "<<testPt.toString();
				throw GEOSException(s.str());
			}
		}
	}
}

 

 

/* private */
bool
NodingValidator::hasInteriorIntersection(const LineIntersector& aLi,
		const Coordinate& p0, const Coordinate& p1) const
{
	for (int i=0, n=aLi.getIntersectionNum(); i<n; i++)
	{
		const Coordinate &intPt=aLi.getIntersection(i);
		if (!(intPt==p0 || intPt==p1))
			return true;
	}
	return false;
}


} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.10  2006/02/15 17:19:18  strk
 * NodingValidator synced with JTS-1.7, added CoordinateSequence::operator[]
 * and size() to easy port maintainance.
 *
 * Revision 1.9  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 * Revision 1.8  2006/02/09 15:52:47  strk
 * GEOSException derived from std::exception; always thrown and cought by const ref.
 *
 * Revision 1.7  2006/01/31 19:07:34  strk
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
 * Revision 1.6  2005/11/25 11:31:21  strk
 * Removed all CoordinateSequence::getSize() calls embedded in for loops.
 *
 * Revision 1.5  2005/06/24 11:09:43  strk
 * Dropped RobustLineIntersector, made LineIntersector a concrete class.
 * Added LineIntersector::hasIntersection(Coordinate&,Coordinate&,Coordinate&)
 * to avoid computing intersection point (Z) when it's not necessary.
 *
 * Revision 1.4  2004/11/01 16:43:04  strk
 * Added Profiler code.
 * Temporarly patched a bug in DoubleBits (must check drawbacks).
 * Various cleanups and speedups.
 *
 * Revision 1.3  2004/07/08 19:34:49  strk
 * Mirrored JTS interface of CoordinateSequence, factory and
 * default implementations.
 * Added CoordinateArraySequenceFactory::instance() function.
 *
 * Revision 1.2  2004/07/02 13:28:27  strk
 * Fixed all #include lines to reflect headers layout change.
 * Added client application build tips in README.
 *
 * Revision 1.1  2004/03/26 07:48:30  ybychkov
 * "noding" package ported (JTS 1.4)
 *
 *
 **********************************************************************/

