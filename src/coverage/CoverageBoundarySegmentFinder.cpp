/*
 * Copyright (c) 2022 Martin Davis.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License 2.0
 * and Eclipse Distribution License v. 1.0 which accompanies this distribution.
 * The Eclipse Public License is available at http://www.eclipse.org/legal/epl-v20.html
 * and the Eclipse Distribution License is available at
 *
 * http://www.eclipse.org/org/documents/edl-v10.php.
 */

#include <geos/coverage/CoverageBoundarySegmentFinder.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>


using geos::geom::CoordinateSequence;
using geos::geom::Geometry;


namespace geos {     // geos
namespace coverage { // geos.coverage

// public static
LineSegment::UnorderedSet
CoverageBoundarySegmentFinder::findBoundarySegments(
    const std::vector<const Geometry*>& geoms)
{
    LineSegment::UnorderedSet segs;
    CoverageBoundarySegmentFinder finder(segs);
    for (const Geometry* geom : geoms) {
        geom->apply_ro(finder);
    }
    return segs;
}

// private static
LineSegment
CoverageBoundarySegmentFinder::createSegment(
    const CoordinateSequence& seq, std::size_t i)
{
    LineSegment seg(seq.getAt(i), seq.getAt(i + 1));
    seg.normalize();
    return seg;
}


// public
void
CoverageBoundarySegmentFinder::filter_ro(
    const CoordinateSequence& seq, std::size_t i)
{
    //-- final point does not start a segment
    if (i >= seq.size() - 1)
        return;

    LineSegment seg = createSegment(seq, i);



    if (m_boundarySegs.find(seg) != m_boundarySegs.end()) {
        m_boundarySegs.erase(seg);
    }
    else {
        m_boundarySegs.insert(seg);
    }
}


/* public static */
bool
CoverageBoundarySegmentFinder::isBoundarySegment(
    const LineSegment::UnorderedSet& boundarySegs,
    const CoordinateSequence* seq,
    std::size_t i)
{
    LineSegment seg = CoverageBoundarySegmentFinder::createSegment(*seq, i);
    return boundarySegs.find(seg) != boundarySegs.end();
}





} // namespace geos.coverage
} // namespace geos
