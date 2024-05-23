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
    std::vector<bool> m_isMatched;

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

    geom::Envelope getEnvelope(std::size_t start, std::size_t end);

    /**
    * Tests if all rings have known status (matched or invalid)
    * for all segments.
    *
    * @param rings a list of rings
    * @return true if all ring segments have known status
    */
    static bool isKnown(std::vector<CoverageRing*>& rings);

    /**
    * Reports if the ring has canonical orientation,
    * with the polygon interior on the right (shell is CW).
    *
    * @return true if the polygon interior is on the right
    */
    bool isInteriorOnRight() const;

    /**
    * Marks a segment as invalid.
    *
    * @param i the segment index
    */
    void markInvalid(std::size_t index);

    /**
    * Marks a segment as matched.
    *
    * @param i the segment index
    */
    void markMatched(std::size_t index);

    /**
    * Tests if all segments in the ring have known status
    * (matched or invalid).
    *
    * @return true if all segments have known status
    */
    bool isKnown() const;

    /**
    * Tests if a segment is marked invalid.
    *
    * @param index the segment index
    * @return true if the segment is invalid
    */
    bool isInvalid(std::size_t i) const;

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

    void createInvalidLines(
        const GeometryFactory* geomFactory,
        std::vector<std::unique_ptr<LineString>>& lines);

};

} // namespace geos.coverage
} // namespace geos





