/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2022 Martin Davis.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/coverage/CoverageBoundarySegmentFinder.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>


using geos::geom::CoordinateSequence;
using geos::geom::Geometry;
using geos::geom::LineSegment;


namespace geos {     // geos
namespace coverage { // geos.coverage

// public static
LineSegment::UnorderedSet
CoverageBoundarySegmentFinder::findBoundarySegments(
    const std::vector<const Geometry*>& geoms,
    util::ProgressFunction* progressFunction)
{
    LineSegment::UnorderedSet segs;
    CoverageBoundarySegmentFinder finder(segs);

    util::Progress progress(progressFunction, geoms.size());

    for (const auto& geom : geoms) {
        geom->apply_ro(finder);
        progress.update();
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
