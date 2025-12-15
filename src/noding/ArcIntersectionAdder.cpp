/**********************************************************************
*
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding/ArcIntersectionAdder.h>
#include <geos/noding/NodableArcString.h>
#include <geos/noding/NodedSegmentString.h>

namespace geos::noding {

void
ArcIntersectionAdder::processIntersections(ArcString& e0, std::size_t segIndex0, ArcString& e1, std::size_t segIndex1)
{
    // don't bother intersecting a segment with itself
    if(&e0 == &e1 && segIndex0 == segIndex1) {
        return;
    }

    const geom::CircularArc& arc0 = e0.getArc(segIndex0);
    const geom::CircularArc& arc1 = e1.getArc(segIndex1);

    m_intersector.intersects(arc0, arc1);

    if (m_intersector.getResult() == algorithm::CircularArcIntersector::NO_INTERSECTION) {
        return;
    }

    for (std::uint8_t i = 0; i < m_intersector.getNumPoints(); i++) {
        detail::down_cast<NodableArcString*>(&e0)->addIntersection(m_intersector.getPoint(i), segIndex0);
        detail::down_cast<NodableArcString*>(&e1)->addIntersection(m_intersector.getPoint(i), segIndex1);
    }

    for (std::uint8_t i = 0; i < m_intersector.getNumArcs(); i++) {
        const auto& arc = m_intersector.getArc(i);
        for (size_t j : {0u, 2u}) {
            arc.applyAt(j, [&e0, &segIndex0, &e1, &segIndex1](const auto& pt) {
                detail::down_cast<NodableArcString*>(&e0)->addIntersection(pt, segIndex0);
                detail::down_cast<NodableArcString*>(&e1)->addIntersection(pt, segIndex1);
            });
        }
    }
}

void
ArcIntersectionAdder::processIntersections(ArcString& e0, std::size_t segIndex0, SegmentString& e1, std::size_t segIndex1)
{
// don't bother intersecting a segment with itself

    const geom::CircularArc& arc = e0.getArc(segIndex0);

    // FIXME get useSegEndpoints from somewhere
    constexpr bool useSegEndpoints = false;
    m_intersector.intersects(arc, *e1.getCoordinates(), segIndex1, segIndex1 + 1, useSegEndpoints);

    if (m_intersector.getResult() == algorithm::CircularArcIntersector::NO_INTERSECTION) {
        return;
    }

    for (std::uint8_t i = 0; i < m_intersector.getNumPoints(); i++) {
        detail::down_cast<NodableArcString*>(&e0)->addIntersection(geom::CoordinateXYZM{m_intersector.getPoint(i)}, segIndex0);
        detail::down_cast<NodedSegmentString*>(&e1)->addIntersection(geom::CoordinateXYZM{m_intersector.getPoint(i)}, segIndex1);
    }


}

void
ArcIntersectionAdder::processIntersections(SegmentString& e0, std::size_t segIndex0, SegmentString& e1, std::size_t segIndex1)
{
    using geom::CoordinateXY;

// don't bother intersecting a segment with itself
    if(&e0 == &e1 && segIndex0 == segIndex1) {
        return;
    }

    m_intersector.intersects(*e0.getCoordinates(), segIndex0, segIndex0 + 1,
                             *e1.getCoordinates(), segIndex1, segIndex1 + 1);

#if 0
    const CoordinateXY& p0 = e0.getCoordinate(segIndex0);
    const CoordinateXY& p1 = e0.getCoordinate(segIndex0 + 1);
    const CoordinateXY& q0 = e1.getCoordinate(segIndex1);
    const CoordinateXY& q1 = e1.getCoordinate(segIndex1 + 1);

    m_intersector.intersects(p0, p1, q0, q1);
#endif

    if (m_intersector.getResult() == algorithm::CircularArcIntersector::NO_INTERSECTION) {
        return;
    }

// todo collinear?

    static_cast<NodedSegmentString&>(e0).addIntersection(m_intersector.getPoint(0), segIndex0);


}

}