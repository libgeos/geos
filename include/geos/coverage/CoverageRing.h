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

#include <geos/noding/BasicSegmentString.h>

#include <deque>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateArraySequence;
class CoordinateSequence;
class Geometry;
class GeometryFactory;
class LineString;
class LinearRing;
class Polygon;
}
}

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateArraySequence;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::Polygon;
using geos::geom::LineString;
using geos::geom::LinearRing;

namespace geos {      // geos.
namespace coverage { // geos.coverage

class GEOS_DLL CoverageRing : public noding::BasicSegmentString {

private:

    // Members
    bool m_isInteriorOnRight;
    std::vector<bool> m_isInvalid;
    std::vector<bool> m_isValid;

    static void createRings(const Polygon* poly, std::vector<CoverageRing*>& rings, std::deque<CoverageRing>& coverageRingStore);

    std::size_t findInvalidStart(std::size_t index);

    std::size_t findInvalidEnd(std::size_t index);

    std::size_t nextMarkIndex(std::size_t index);

    /**
    * Creates a line from a sequence of ring segments between startIndex and endIndex (inclusive).
    * If the endIndex < startIndex the sequence wraps around the ring endpoint.
    *
    * @param startIndex
    * @param endIndex
    * @param geomFactory
    * @return a line representing the section
    */
    std::unique_ptr<LineString> createLine(
        std::size_t startIndex,
        std::size_t endIndex,
        const GeometryFactory* geomFactory);

    std::unique_ptr<CoordinateSequence> extractSection(
        std::size_t startIndex, std::size_t endIndex);

    std::unique_ptr<CoordinateSequence> extractSectionWrap(
        std::size_t startIndex, std::size_t endIndex);

public:

    CoverageRing(CoordinateSequence* pts, bool interiorOnRight);

    CoverageRing(const LinearRing* ring, bool isShell);

    static std::vector<CoverageRing*> createRings(const Geometry* geom, std::deque<CoverageRing>& coverageRingStore);

    static std::vector<CoverageRing*> createRings(std::vector<const Polygon*>& polygons,  std::deque<CoverageRing>& coverageRingStore);

    static bool isValid(std::vector<CoverageRing*>& rings);

    bool isInteriorOnRight() const;

    /**
    * Tests if a segment is marked valid.
    *
    * @param index the segment index
    * @return true if the segment is valid
    */
    bool isValid(std::size_t index) const;

    /**
    * Tests if a segment is marked invalid.
    *
    * @param index the segment index
    * @return true if the segment is invalid
    */
    bool isInvalid(std::size_t index) const;

    /**
    * Tests whether all segments are valid.
    *
    * @return true if all segments are valid
    */
    bool isValid() const;

    /**
    * Tests whether all segments are invalid.
    *
    * @return true if all segments are invalid
    */
    bool isInvalid() const;

    /**
    * Tests whether any segment is invalid.
    *
    * @return true if some segment is invalid
    */
    bool hasInvalid() const;

    /**
    * Tests whether the validity state of a ring segment is known.
    *
    * @param i the index of the ring segment
    * @return true if the segment validity state is known
    */
    bool isKnown(std::size_t i) const;

    /**
    * Finds the previous vertex in the ring which is distinct
    * from a given coordinate value.
    *
    * @param index the index to start the search
    * @param pt a coordinate value (which may not be a ring vertex)
    * @return the previous distinct vertex in the ring
    */
    const Coordinate& findVertexPrev(std::size_t index, const Coordinate& pt) const;

    /**
    * Finds the next vertex in the ring which is distinct
    * from a given coordinate value.
    *
    * @param index the index to start the search
    * @param pt a coordinate value (which may not be a ring vertex)
    * @return the next distinct vertex in the ring
    */
    const Coordinate& findVertexNext(std::size_t index, const Coordinate& pt) const;

    /**
    * Gets the index of the previous segment in the ring.
    *
    * @param index a segment index
    * @return the index of the previous segment
    */
    std::size_t prev(std::size_t index) const;

    /**
    * Gets the index of the next segment in the ring.
    *
    * @param index a segment index
    * @return the index of the next segment
    */
    std::size_t next(std::size_t index) const;

    /**
    * Marks a segment as invalid.
    *
    * @param i the segment index
    */
    void markInvalid(std::size_t i);

    /**
    * Marks a segment as valid.
    *
    * @param i the segment index
    */
    void markValid(std::size_t i);

    void createInvalidLines(
        const GeometryFactory* geomFactory,
        std::vector<std::unique_ptr<LineString>>& lines);

};

} // namespace geos.coverage
} // namespace geos





