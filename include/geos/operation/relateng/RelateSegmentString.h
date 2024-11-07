/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/noding/BasicSegmentString.h>
#include <geos/export.h>


#include <string>
#include <sstream>


// Forward declarations
namespace geos {
namespace geom {
    class CoordinateXY;
    class CoordinateSequence;
    class Geometry;
}
namespace operation {
namespace relateng {
    class RelateGeometry;
    class NodeSection;
}
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace relateng { // geos.operation.relateng



class GEOS_DLL RelateSegmentString : public geos::noding::BasicSegmentString {
    using BasicSegmentString = geos::noding::BasicSegmentString;
    using Geometry = geos::geom::Geometry;
    using CoordinateXY = geos::geom::CoordinateXY;
    using CoordinateSequence = geos::geom::CoordinateSequence;

private:

    // Members
    bool m_isA;
    int m_dimension;
    int m_id;
    int m_ringId;
    const RelateGeometry* m_inputGeom;
    const Geometry* m_parentPolygonal = nullptr;

    // Constructor
    RelateSegmentString(
        const CoordinateSequence* pts,
        bool isA,
        int dimension,
        int id,
        int ringId,
        const Geometry* poly,
        const RelateGeometry* inputGeom)
        : BasicSegmentString(const_cast<CoordinateSequence*>(pts), nullptr)
        , m_isA(isA)
        , m_dimension(dimension)
        , m_id(id)
        , m_ringId(ringId)
        , m_inputGeom(inputGeom)
        , m_parentPolygonal(poly)
        {}


    // Methods

    static const RelateSegmentString* createSegmentString(
        const CoordinateSequence* pts,
        bool isA, int dim, int elementId, int ringId,
        const Geometry* poly, const RelateGeometry* parent);

    /**
     *
     * @param ss
     * @param segIndex
     * @param pt
     * @return the previous vertex, or null if none exists
     */
    const CoordinateXY* prevVertex(
        std::size_t segIndex,
        const CoordinateXY* pt) const;

    /**
     * @param ss
     * @param segIndex
     * @param pt
     * @return the next vertex, or null if none exists
     */
    const CoordinateXY* nextVertex(
        std::size_t segIndex,
        const CoordinateXY* pt) const;


public:

    static const RelateSegmentString* createLine(
        const CoordinateSequence* pts,
        bool isA, int elementId,
        const RelateGeometry* parent);

    static const RelateSegmentString* createRing(
        const CoordinateSequence* pts,
        bool isA, int elementId, int ringId,
        const Geometry* poly, const RelateGeometry* parent);

    inline bool isA() const {
        return m_isA;
    }

    inline const RelateGeometry* getGeometry() const {
        return m_inputGeom;
    }

    inline const Geometry* getPolygonal() const {
        return m_parentPolygonal;
    }

    NodeSection* createNodeSection(std::size_t segIndex, const CoordinateXY intPt) const;

    /**
     * Tests if a segment intersection point has that segment as its
     * canonical containing segment.
     * Segments are half-closed, and contain their start point but not the endpoint,
     * except for the final segment in a non-closed segment string, which contains
     * its endpoint as well.
     * This test ensures that vertices are assigned to a unique segment in a segment string.
     * In particular, this avoids double-counting intersections which lie exactly
     * at segment endpoints.
     *
     * @param segIndex the segment the point may lie on
     * @param pt the point
     * @return true if the segment contains the point
     */
    bool isContainingSegment(std::size_t segIndex, const CoordinateXY* pt) const;

};

} // namespace geos.operation.relateng
} // namespace geos.operation
} // namespace geos

