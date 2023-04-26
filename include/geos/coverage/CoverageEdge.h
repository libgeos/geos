


/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LineSegment.h>
#include <geos/util.h>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class LinearRing;
class LineString;
class MultiLineString;
class GeometryFactory;
}
}

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::GeometryFactory;
using geos::geom::LinearRing;
using geos::geom::LineString;
using geos::geom::LineSegment;
using geos::geom::MultiLineString;

namespace geos {      // geos.
namespace coverage { // geos.coverage

/**
 * An edge of a polygonal coverage formed from all or a section of a polygon ring.
 * An edge may be a free ring, which is a ring which has not node points
 * (i.e. does not touch any other rings in the parent coverage).
 *
 * @author mdavis
 *
 */
class GEOS_DLL CoverageEdge {

private:

    // Members
    std::unique_ptr<CoordinateSequence> m_pts;
    std::size_t m_ringCount ;
    bool m_isFreeRing = true;

    // Methods

    static std::unique_ptr<CoordinateSequence>
    extractEdgePoints(const CoordinateSequence& ring,
        std::size_t start, std::size_t end);

    static const Coordinate&
    findDistinctPoint(
        const CoordinateSequence& pts,
        std::size_t index,
        bool isForward,
        const Coordinate& pt);


public:

    CoverageEdge(std::unique_ptr<CoordinateSequence> && pts, bool isFreeRing)
        : m_pts(pts ? std::move(pts) : detail::make_unique<CoordinateSequence>())
        , m_ringCount(0)
        , m_isFreeRing(isFreeRing)
        {}

    /**
    * Computes a key segment for a ring.
    * The key is the segment starting at the lowest vertex,
    * towards the lowest adjacent distinct vertex.
    *
    * @param ring a linear ring
    * @return a LineSegment representing the key
    */
    static LineSegment key(
        const CoordinateSequence& ring);

    /**
    * Computes a distinct key for a section of a linear ring.
    *
    * @param ring the linear ring
    * @param start index of the start of the section
    * @param end end index of the end of the section
    * @return a LineSegment representing the key
    */
    static LineSegment key(
        const CoordinateSequence& ring,
        std::size_t start,
        std::size_t end);

    static std::unique_ptr<CoverageEdge> createEdge(
        const CoordinateSequence& ring);

    static std::unique_ptr<CoverageEdge> createEdge(
        const CoordinateSequence& ring,
        std::size_t start,
        std::size_t end);

    static std::unique_ptr<MultiLineString> createLines(
        const std::vector<CoverageEdge*>& edges,
        const GeometryFactory* geomFactory);

    std::unique_ptr<LineString> toLineString(
        const GeometryFactory* geomFactory);

    /* public */
    void incRingCount()
    {
        m_ringCount++;
    }

    /* public */
    std::size_t getRingCount() const
    {
        return m_ringCount;
    }

    /**
    * Returns whether this edge is a free ring;
    * i.e. one with no constrained nodes.
    *
    * @return true if this is a free ring
    */
    bool isFreeRing() const
    {
        return m_isFreeRing;
    }

    void setCoordinates(const CoordinateSequence* pts)
    {
        m_pts = pts->clone();
    }

    const CoordinateSequence* getCoordinates() const
    {
        return m_pts.get();
    }

    const Coordinate& getEndCoordinate() const
    {
        return m_pts->getAt(m_pts->size() - 1);
    }

    const Coordinate& getStartCoordinate() const
    {
        return m_pts->getAt(0);
    }


};

} // namespace geos.coverage
} // namespace geos
