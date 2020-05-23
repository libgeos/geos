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
#include <geos/util/IllegalArgumentException.h>

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
    ptHot(newPt),
    originalPt(newPt),
    scaleFactor(newScaleFactor),
    snapCount(0)
{
    if(scaleFactor <= 0.0) {
        throw util::IllegalArgumentException("Scale factor must be non-zero");
    }
    if(scaleFactor != 1.0) {
        ptHot.x = scaleRound(newPt.x);
        ptHot.y = scaleRound(newPt.y);
    }
    initCorners(ptHot);
}

/*public*/
void
HotPixel::incrementSnapCount()
{
    snapCount++;
}

/*public*/
int
HotPixel::getSnapCount()
{
    return snapCount;
}

/*public*/
geom::Coordinate
HotPixel::getCoordinate()
{
    return originalPt;
}

/*public*/
const Envelope&
HotPixel::getSafeEnvelope() const
{
    if(safeEnv.get() == nullptr) {
        double safeTolerance = SAFE_ENV_EXPANSION_FACTOR / scaleFactor;
        safeEnv = unique_ptr<Envelope>(new Envelope(
                        originalPt.x - safeTolerance,
                        originalPt.x + safeTolerance,
                        originalPt.y - safeTolerance,
                        originalPt.y + safeTolerance
                    ));
    }
    return *safeEnv;
}

/*private*/
void
HotPixel::initCorners(const Coordinate& p_pt)
{
    double tolerance = 0.5;
    minx = p_pt.x - tolerance;
    maxx = p_pt.x + tolerance;
    miny = p_pt.y - tolerance;
    maxy = p_pt.y + tolerance;

    corner[UPPER_RIGHT] = Coordinate(maxx, maxy);
    corner[UPPER_LEFT] = Coordinate(minx, maxy);
    corner[LOWER_LEFT] = Coordinate(minx, miny);
    corner[LOWER_RIGHT] = Coordinate(maxx, miny);
}

/*public*/
bool
HotPixel::intersects(const Coordinate& p0,
                     const Coordinate& p1) const
{
    if(scaleFactor == 1.0) {
        return intersectsScaled(p0, p1);
    }

    copyScaled(p0, p0Scaled);
    copyScaled(p1, p1Scaled);

    return intersectsScaled(p0Scaled, p1Scaled);
}

/* private */
bool
HotPixel::intersectsScaled(const Coordinate& p0,
                           const Coordinate& p1) const
{

    double const segMinx = std::min(p0.x, p1.x);
    double const segMaxx = std::max(p0.x, p1.x);
    double const segMiny = std::min(p0.y, p1.y);
    double const segMaxy = std::max(p0.y, p1.y);

    bool isOutsidePixelEnv =  maxx < segMinx
                              || minx > segMaxx
                              || maxy < segMiny
                              || miny > segMaxy;

    if(isOutsidePixelEnv) {
        return false;
    }

    bool p_intersects = intersectsToleranceSquareScaled(p0, p1);

    // Found bad envelope test
    assert(!(isOutsidePixelEnv && p_intersects));

    return p_intersects;
}

/*private*/
bool
HotPixel::intersectsToleranceSquareScaled(const Coordinate& p0,
                                    const Coordinate& p1) const
{
    bool intersectsTop = false;
    bool intersectsBottom = false;

    // check intersection with pixel left edge
    li.computeIntersection(p0, p1, corner[UPPER_LEFT], corner[LOWER_LEFT]);
    if(li.isProper()) return true;

    // check intersection with pixel right edge
    li.computeIntersection(p0, p1, corner[LOWER_RIGHT], corner[UPPER_RIGHT]);
    if(li.isProper()) return true;

    // check intersection with pixel top edge
    li.computeIntersection(p0, p1, corner[UPPER_RIGHT], corner[UPPER_LEFT]);
    if(li.isProper()) return true;
    if(li.hasIntersection()) {
        intersectsTop = true;
    }

    // check intersection with pixel bottom edge
    li.computeIntersection(p0, p1, corner[LOWER_LEFT], corner[LOWER_RIGHT]);
    if(li.isProper()) return true;
    if(li.hasIntersection()) {
        intersectsBottom = true;
    }

    // check intersection of vertical segment overlapping pixel left edge
    if (p0.x == corner[LOWER_LEFT].x && p1.x == corner[LOWER_LEFT].x) {
        if (p0.y < corner[UPPER_LEFT].y || p1.y < corner[UPPER_LEFT].y) {
            return true;
        }
    }

    // check intersection of horizontal segment overlapping pixel bottome edge
    if (p0.y == corner[LOWER_LEFT].y && p1.y == corner[LOWER_LEFT].y) {
        if (p0.x < corner[LOWER_RIGHT].x || p1.x < corner[LOWER_RIGHT].x) {
            return true;
        }
    }

    /**
     * Check for an edge crossing pixel exactly on a diagonal.
     * The code handles both diagonals.
     */
    if(intersectsTop && intersectsBottom) {
        return true;
    }

    /**
     * Tests if either endpoint snaps to this pixel.
     * This is needed because a (un-rounded) segment may
     * terminate in a hot pixel without crossing a pixel edge interior
     * (e.g. it may enter through a corner)
     */
    if(equalsPointScaled(p0)) return true;
    if(equalsPointScaled(p1)) return true;

    return false;
}

/**
* Tests if a scaled coordinate snaps (rounds) to this pixel.
*
* @param p the point to test
* @return true if the coordinate snaps to this pixel
*/
bool
HotPixel::equalsPointScaled(const geom::Coordinate& p) const
{
    return util::round(p.x) == ptHot.x
        && util::round(p.y) == ptHot.y;
}


/*private*/
bool
HotPixel::intersectsPixelClosure(const Coordinate& p0,
                                 const Coordinate& p1)
{
    li.computeIntersection(p0, p1, corner[UPPER_RIGHT], corner[UPPER_LEFT]);
    if(li.hasIntersection()) {
        return true;
    }
    li.computeIntersection(p0, p1, corner[UPPER_LEFT], corner[LOWER_LEFT]);
    if(li.hasIntersection()) {
        return true;
    }
    li.computeIntersection(p0, p1, corner[LOWER_LEFT], corner[LOWER_RIGHT]);
    if(li.hasIntersection()) {
        return true;
    }
    li.computeIntersection(p0, p1, corner[LOWER_RIGHT], corner[UPPER_RIGHT]);
    if(li.hasIntersection()) {
        return true;
    }

    return false;
}

bool
HotPixel::addSnappedNode(NodedSegmentString& segStr, size_t segIndex)
{
    const Coordinate& p0 = segStr.getCoordinate(segIndex);
    const Coordinate& p1 = segStr.getCoordinate(segIndex + 1);

    if(intersects(p0, p1)) {
        //cout << "snapped: " <<  snapPt << endl;
        segStr.addIntersection(getCoordinate(), segIndex);
        return true;
    }
    return false;
}

std::ostream&
HotPixel::operator<< (std::ostream& os)
{
    os << "HP(" << io::WKTWriter::toPoint(ptHot) << ")";
    return os;
}


} // namespace geos.noding.snapround
} // namespace geos.noding
} // namespace geos
