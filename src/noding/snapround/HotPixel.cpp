/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/snapround/HotPixel.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/noding/snapround/HotPixel.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>

#ifndef GEOS_INLINE
# include "geos/noding/snapround/HotPixel.inl"
#endif

#include <algorithm> // for std::min and std::max
#include <cassert>
#include <memory>

using namespace std;
using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding
namespace snapround { // geos.noding.snapround

HotPixel::HotPixel(const Coordinate& newPt, double newScaleFactor,
		LineIntersector& newLi)
	:
	li(newLi),
	pt(newPt),
	originalPt(newPt),
	scaleFactor(newScaleFactor)
{
	if (scaleFactor != 1.0) {
		assert( scaleFactor != 0 ); // or should it be an IllegalArgumentException ?
		pt.x=scale(pt.x);
		pt.y=scale(pt.y);
	}
	initCorners(pt);
}

const Envelope&
HotPixel::getSafeEnvelope() const
{
	static const double SAFE_ENV_EXPANSION_FACTOR = 0.75;

	if (safeEnv.get() == NULL) {
		double safeTolerance = SAFE_ENV_EXPANSION_FACTOR / scaleFactor;
		safeEnv = auto_ptr<Envelope>(new Envelope(originalPt.x - safeTolerance,
			originalPt.x + safeTolerance,
			originalPt.y - safeTolerance,
			originalPt.y + safeTolerance
			));
	}
	return *safeEnv;
}

/*private*/
void
HotPixel::initCorners(const Coordinate& pt) 
{
	double tolerance = 0.5;
	minx = pt.x - tolerance;
	maxx = pt.x + tolerance;
	miny = pt.y - tolerance;
	maxy = pt.y + tolerance;

	corner.resize(4);
	corner[0] = Coordinate(maxx, maxy);
	corner[1] = Coordinate(minx, maxy);
	corner[2] = Coordinate(minx, miny);
	corner[3] = Coordinate(maxx, maxy);
}

bool
HotPixel::intersects(const Coordinate& p0,
		const Coordinate& p1) const
{
	if (scaleFactor == 1.0) return intersectsScaled(p0, p1);

	copyScaled(p0, p0Scaled);
	copyScaled(p1, p1Scaled);

	return intersectsScaled(p0Scaled, p1Scaled);
}

/* private */
bool
HotPixel::intersectsScaled(const Coordinate& p0,
		const Coordinate& p1) const
{

	double const segMinx = (std::min)(p0.x, p1.x);
	double const segMaxx = (std::max)(p0.x, p1.x);
	double const segMiny = (std::min)(p0.y, p1.y);
	double const segMaxy = (std::max)(p0.y, p1.y);

	bool isOutsidePixelEnv =  maxx < segMinx
                         || minx > segMaxx
                         || maxy < segMiny
                         || miny > segMaxy;

	if (isOutsidePixelEnv) return false;

	bool intersects = intersectsToleranceSquare(p0, p1);

	// Found bad envelope test
	assert(!(isOutsidePixelEnv && intersects));

	return intersects;
}

/*private*/
bool
HotPixel::intersectsToleranceSquare(const Coordinate& p0,
		const Coordinate& p1) const
{
    bool intersectsLeft = false;
    bool intersectsBottom = false;

    li.computeIntersection(p0, p1, corner[0], corner[1]);
    if (li.isProper()) return true;

    li.computeIntersection(p0, p1, corner[1], corner[2]);
    if (li.isProper()) return true;
    if (li.hasIntersection()) intersectsLeft = true;

    li.computeIntersection(p0, p1, corner[2], corner[3]);
    if (li.isProper()) return true;
    if (li.hasIntersection()) intersectsBottom = true;

    li.computeIntersection(p0, p1, corner[3], corner[0]);
    if (li.isProper()) return true;

    if (intersectsLeft && intersectsBottom) return true;

    if (p0.equals2D(pt)) return true;
    if (p1.equals2D(pt)) return true;

    return false;
}

/*private*/
bool
HotPixel::intersectsPixelClosure(const Coordinate& p0,
		const Coordinate& p1)
{
    li.computeIntersection(p0, p1, corner[0], corner[1]);
    if (li.hasIntersection()) return true;
    li.computeIntersection(p0, p1, corner[1], corner[2]);
    if (li.hasIntersection()) return true;
    li.computeIntersection(p0, p1, corner[2], corner[3]);
    if (li.hasIntersection()) return true;
    li.computeIntersection(p0, p1, corner[3], corner[0]);
    if (li.hasIntersection()) return true;

    return false;
}

bool
HotPixel::addSnappedNode(NodedSegmentString& segStr, size_t segIndex)
{
	const Coordinate& p0 = segStr.getCoordinate(segIndex);
	const Coordinate& p1 = segStr.getCoordinate(segIndex + 1);

	if (intersects(p0, p1))
	{
		//cout << "snapped: " <<  snapPt << endl;
		segStr.addIntersection(getCoordinate(), segIndex);
		return true;
	}
	return false;
}


} // namespace geos.noding.snapround
} // namespace geos.noding
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.8  2006/05/03 17:51:24  strk
 * system headers included after package headers
 *
 * Revision 1.7  2006/03/24 09:52:41  strk
 * USE_INLINE => GEOS_INLINE
 *
 * Revision 1.6  2006/03/14 12:55:56  strk
 * Headers split: geomgraphindex.h, nodingSnapround.h
 *
 * Revision 1.5  2006/03/03 10:46:21  strk
 * Removed 'using namespace' from headers, added missing headers in .cpp files, removed useless includes in headers (bug#46)
 *
 * Revision 1.4  2006/02/21 16:53:49  strk
 * MCIndexPointSnapper, MCIndexSnapRounder
 *
 * Revision 1.3  2006/02/19 19:46:49  strk
 * Packages <-> namespaces mapping for most GEOS internal code (uncomplete, but working). Dir-level libs for index/ subdirs.
 *
 * Revision 1.2  2006/02/18 21:08:09  strk
 * - new CoordinateSequence::applyCoordinateFilter method (slow but useful)
 * - SegmentString::getCoordinates() doesn't return a clone anymore.
 * - SegmentString::getCoordinatesRO() obsoleted.
 * - SegmentString constructor does not promises constness of passed
 *   CoordinateSequence anymore.
 * - NEW ScaledNoder class
 * - Stubs for MCIndexPointSnapper and  MCIndexSnapRounder
 * - Simplified internal interaces of OffsetCurveBuilder and OffsetCurveSetBuilder
 *
 * Revision 1.1  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 **********************************************************************/
