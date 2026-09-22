/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/distance/CoordinateSequenceLocation.java
 * (locationtech/jts DirectedHausdorffDistance)
 *
 **********************************************************************/

#include <geos/geom/CoordinateSequence.h>
#include <geos/operation/distance/CoordinateSequenceLocation.h>

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;

namespace geos {
namespace operation {
namespace distance {

CoordinateSequenceLocation::CoordinateSequenceLocation(
    const CoordinateSequence* p_seq, std::size_t p_index, const Coordinate& p_pt)
    : seq(p_seq)
    , index(p_index)
    , pt(p_pt)
{
    if (seq && index >= seq->size() && seq->size() > 0) {
        index = seq->size() - 1;
    }
}

const Coordinate&
CoordinateSequenceLocation::getCoordinate() const
{
    return pt;
}

std::size_t
CoordinateSequenceLocation::getIndex() const
{
    return index;
}

bool
CoordinateSequenceLocation::isSameSegment(const CoordinateSequenceLocation& f) const
{
    if (seq != f.seq) {
        return false;
    }
    if (index == f.index) {
        return true;
    }
    //-- check for end pt same as start point of next segment
    if (isNext(index, f.index)) {
        const Coordinate& endPt = seq->getAt(index + 1);
        return f.pt.equals2D(endPt);
    }
    if (isNext(f.index, index)) {
        const Coordinate& endPt = f.seq->getAt(index + 1);
        return pt.equals2D(endPt);
    }
    return false;
}

bool
CoordinateSequenceLocation::isNext(std::size_t p_index, std::size_t index1) const
{
    if (index1 == p_index + 1) {
        return true;
    }
    // JTS writes `index1 == 0 && isRing && index1 == size-1`, which is
    // unreachable. The ring wrap is last segment (size-2) adjacent to 0.
    if (seq && seq->isRing() && index1 == 0 && p_index + 1 == seq->size() - 1) {
        return true;
    }
    return false;
}

Coordinate
CoordinateSequenceLocation::getEndPoint(int i) const
{
    if (i == 0) {
        return seq->getAt(index);
    }
    return seq->getAt(index + 1);
}

std::size_t
CoordinateSequenceLocation::normalize(std::size_t p_index) const
{
    if (seq && p_index >= seq->size() - 1 && seq->isRing()) {
        return 0;
    }
    return p_index;
}

} // namespace distance
} // namespace operation
} // namespace geos
