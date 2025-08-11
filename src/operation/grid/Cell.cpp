/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018-2025 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <numeric>

#include <geos/algorithm/Length.h>
#include <geos/operation/grid/Cell.h>
#include <geos/operation/grid/Crossing.h>
#include <geos/operation/grid/TraversalAreas.h>

#define DEBUG_CELL 0
#include <iomanip>

using geos::geom::CoordinateXY;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;

namespace geos::operation::grid {

static Crossing
crossing(const geom::Envelope& e, const CoordinateXY& c1, const CoordinateXY& c2)
{
    // vertical line
    if (c1.x == c2.x) {
        if (c2.y >= e.getMaxY()) {
            return Crossing{ Side::TOP, c1.x, e.getMaxY() };
        } else if (c2.y <= e.getMinY()) {
            return Crossing{ Side::BOTTOM, c1.x, e.getMinY() };
        } else {
            throw std::runtime_error("Never get here.");
        }
    }

    // horizontal line
    if (c1.y == c2.y) {
        if (c2.x >= e.getMaxX()) {
            return Crossing{ Side::RIGHT, e.getMaxX(), c1.y };
        } else if (c2.x <= e.getMinX()) {
            return Crossing{ Side::LEFT, e.getMinX(), c1.y };
        } else {
            throw std::runtime_error("Never get here");
        }
    }

    const double m = std::abs((c2.y - c1.y) / (c2.x - c1.x));

    const bool up = c2.y > c1.y;
    const bool right = c2.x > c1.x;

    if (up) {

        if (right) {
            // 1st quadrant
            const double y2 = c1.y + m * (e.getMaxX() - c1.x);

            if (y2 < e.getMaxY()) {
                return Crossing{ Side::RIGHT, e.getMaxX(), std::clamp(y2, e.getMinY(), e.getMaxY()) };
            } else {
                double x2 = c1.x + (e.getMaxY() - c1.y) / m;
                return Crossing{ Side::TOP, std::clamp(x2, e.getMinX(), e.getMaxX()), e.getMaxY() };
            }
        } else {
            // 2nd quadrant
            const double y2 = c1.y + m * (c1.x - e.getMinX());

            if (y2 < e.getMaxY()) {
                return Crossing{ Side::LEFT, e.getMinX(), std::clamp(y2, e.getMinY(), e.getMaxY()) };
            } else {
                double x2 = c1.x - (e.getMaxY() - c1.y) / m;
                return Crossing{ Side::TOP, std::clamp(x2, e.getMinX(), e.getMaxX()), e.getMaxY() };
            }
        }
    } else {
        // For downward segments, we calculate constructed coordinates relative to c2, not c1. This is so the
        // same coordinate will be calculated regardless of the segment orientation. This is important for maintaining
        // valid polygon coverages (the same segment will be processed with opposite orientation along shared
        // boundaries)

        if (right) {
            // 4th quadrant
            const double y2 = c2.y + m * (c2.x - e.getMaxX());

            if (y2 > e.getMinY()) {
                return Crossing{ Side::RIGHT, e.getMaxX(), std::clamp(y2, e.getMinY(), e.getMaxY()) };
            } else {
                double x2 = c2.x - (e.getMinY() - c2.y) / m;
                return Crossing{ Side::BOTTOM, std::clamp(x2, e.getMinX(), e.getMaxX()), e.getMinY() };
            }
        } else {
            // 3rd quadrant
            const double y2 = c2.y + m * (e.getMinX() - c2.x);

            if (y2 > e.getMinY()) {
                return Crossing{ Side::LEFT, e.getMinX(), std::clamp(y2, e.getMinY(), e.getMaxY()) };
            } else {
                double x2 = c2.x + (e.getMinY() - c2.y) / m;
                return Crossing{ Side::BOTTOM, std::clamp(x2, e.getMinX(), e.getMaxX()), e.getMinY() };
            }
        }
    }
}


double
Cell::getHeight() const
{
    return m_box.getHeight();
}

double
Cell::getWidth() const
{
    return m_box.getWidth();
}

double
Cell::getArea() const
{
    return m_box.getArea();
}

Side
Cell::getSide(const CoordinateXY& c) const
{
    if (c.x == m_box.getMinX()) {
        return Side::LEFT;
    } else if (c.x == m_box.getMaxX()) {
        return Side::RIGHT;
    } else if (c.y == m_box.getMinY()) {
        return Side::BOTTOM;
    } else if (c.y == m_box.getMaxY()) {
        return Side::TOP;
    }

    return Side::NONE;
}

void
Cell::forceExit()
{
    if (getLastTraversal().isExited()) {
        return;
    }

    const CoordinateXY& last = getLastTraversal().getLastCoordinate();

    if (getLocation(last) == Location::BOUNDARY) {
        getLastTraversal().forceExit(getSide(last));
    }
}

Cell::Location
Cell::getLocation(const CoordinateXY& c) const
{
    if (m_box.containsProperly(c)) {
        return Cell::Location::INSIDE;
    }

    if (m_box.contains(c)) {
        return Cell::Location::BOUNDARY;
    }

    return Cell::Location::OUTSIDE;
}

Traversal&
Cell::traversal_in_progress()
{
    if (m_traversals.empty() || m_traversals.back().isExited() || m_traversals.back().isClosedRing()) {
        m_traversals.emplace_back();
    }

    return m_traversals[m_traversals.size() - 1];
}

Traversal&
Cell::getLastTraversal()
{
    return m_traversals.at(m_traversals.size() - 1);
}

bool
Cell::take(const CoordinateXY& c, const CoordinateXY* prev_original, const void* parentage)
{
    Traversal& t = traversal_in_progress();

    if (t.isEmpty()) {
#if DEBUG_CELL
        std::cout << std::setprecision(17);
        std::cout << "Entering " << m_box << " from " << getSide(c) << " at " << c << std::endl;
#endif

        t.enter(c, getSide(c), parentage);
        return true;
    }

    if (getLocation(c) != Location::OUTSIDE) {
#if DEBUG_CELL
        std::cout << "Still in " << m_box << " with " << c << std::endl;
#endif

        t.add(c);

        if (t.isClosedRing()) {
            t.forceExit(Side::NONE);
        }

        return true;
    }

    // We need to calculate the coordinate of the cell exit point using only uninterpolated coordinates.
    // (The previous point in the traversal may be an interpolated coordinate.) If an interpolated coordinate
    // is used, it can cause an error in the relative position two traversals, inverting the fraction of
    // the cell that is considered covered. (See robustness regression test #7).
    Crossing x = prev_original ? crossing(m_box, *prev_original, c) : crossing(m_box, t.getLastCoordinate(), c);
    t.exit(x.getCoord(), x.getSide());

#if DEBUG_CELL
    std::cout << "Leaving " << m_box << " from " << x.getSide() << " at " << x.getCoord();
    std::cout << " on the way to " << c << std::endl;
#endif

    return false;
}

double
Cell::getTraversalLength() const
{
    return std::accumulate(m_traversals.begin(), m_traversals.end(), 0.0, [](double tot, const Traversal& t) {
        return tot + algorithm::Length::ofLine(t.getCoordinates());
    });
}

bool
Cell::isDetermined() const
{
    for (const auto& t : m_traversals) {
        if (t.isClosedRing()) {
            if (!t.isClosedRingWithArea()) {
                continue;
            }
        } else if (!t.isTraversed()) {
            continue;
        }

        if (t.hasMultipleUniqueCoordinates()) {
            return true;
        }
    }

    return false;
}

std::vector<const Traversal*>
Cell::getTraversals() const
{
    std::vector<const Traversal*> traversals;
    traversals.reserve(m_traversals.size());

    for (const auto& t : m_traversals) {
        if (t.isTraversed() || t.isClosedRingWithArea()) {
            traversals.push_back(&t);
        }
    }

    return traversals;
}

double
Cell::getCoveredFraction() const
{
    auto coord_lists = getTraversals();
    return TraversalAreas::getLeftHandArea(m_box, coord_lists) / getArea();
}

std::unique_ptr<Geometry>
Cell::getCoveredPolygons(const GeometryFactory& gfact) const
{
    auto coord_lists = getTraversals();
    return TraversalAreas::getLeftHandRings(gfact, m_box, coord_lists);
}

}
