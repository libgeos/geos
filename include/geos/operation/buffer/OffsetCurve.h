/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/index/chain/MonotoneChainSelectAction.h>
#include <geos/operation/buffer/BufferParameters.h>


#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4251) // warning C4251: needs to have dll-interface to be used by clients of class
#endif

// Forward declarations
namespace geos {
namespace geom {
class Geometry;
class LineString;
class LinearRing;
class Polygon;
class CoordinateSequence;
class Coordinate;
}
namespace operation {
namespace buffer {
class SegmentMCIndex;
}
}
namespace index {
namespace chain {
class MonotoneChain;
}
}
}

using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineString;
using geos::geom::LinearRing;
using geos::geom::Polygon;
using geos::geom::CoordinateSequence;
using geos::geom::Coordinate;

namespace geos {
namespace operation {
namespace buffer {

/**
 * Computes an offset curve from a geometry.
 * The offset curve is a linear geometry which is offset a specified distance
 * from the input.
 * If the offset distance is positive the curve lies on the left side of the input;
 * if it is negative the curve is on the right side.
 *
 * The offset curve of a line is a LineString which
 * The offset curve of a Point is an empty LineString.
 * The offset curve of a Polygon is the boundary of the polygon buffer (which
 * may be a MultiLineString.
 * For a collection the output is a MultiLineString of the element offset curves.
 *
 * The offset curve is computed as a single contiguous section of the geometry buffer boundary.
 * In some geometric situations this definition is ill-defined.
 * This algorithm provides a "best-effort" interpretation.
 * In particular:
 *
 *  * For self-intersecting lines, the buffer boundary includes
 *    offset lines for both left and right sides of the input line.
 *    Only a single contiguous portion on the specified side is returned.
 *  * If the offset corresponds to buffer holes, only the largest hole is used.
 *
 * Offset curves support setting the number of quadrant segments,
 * the join style, and the mitre limit (if applicable) via
 * the BufferParameters.
 *
 * @author Martin Davis
 */
class GEOS_DLL OffsetCurve {


private:

    // Constants
    static constexpr int NEARNESS_FACTOR = 10000;

    // Members
    const Geometry& inputGeom;
    double distance;
    BufferParameters bufferParams;
    double matchDistance;
    const GeometryFactory* geomFactory;

    // Methods

    std::unique_ptr<LineString> computeCurve(const LineString& lineGeom, double distance);

    std::unique_ptr<LineString> offsetSegment(const CoordinateSequence* pts, double distance);

    static std::unique_ptr<Polygon> getBufferOriented(const LineString& geom, double distance, BufferParameters& bufParms);

    /**
    * Extracts the largest polygon by area from a geometry.
    * Used here to avoid issues with non-robust buffer results which have spurious extra polygons.
    *
    * @param geom a geometry
    * @return the polygon element of largest area
    */
    static std::unique_ptr<Polygon> extractMaxAreaPolygon(const Geometry& geom);

    static std::unique_ptr<LinearRing> extractLongestHole(const Polygon& poly);

    std::unique_ptr<LineString> computeCurve(
        const CoordinateSequence* bufferPts,
        std::vector<CoordinateSequence*>& rawOffsetList);

    int markMatchingSegments(const Coordinate& p0, const Coordinate& p1,
        SegmentMCIndex& segIndex, const CoordinateSequence* bufferPts,
        std::vector<bool>& isInCurve);

    static double subsegmentMatchFrac(const Coordinate& p0, const Coordinate& p1,
        const Coordinate& seg0, const Coordinate& seg1, double matchDistance);

    /**
    * Extracts a section of a ring of coordinates, starting at a given index,
    * and keeping coordinates which are flagged as being required.
    *
    * @param ring the ring of points
    * @param startIndex the index of the start coordinate
    * @param isExtracted flag indicating if coordinate is to be extracted
    * @return
    */
    static void extractSection(const CoordinateSequence* ring, int iStartIndex,
            std::vector<bool>& isExtracted, std::vector<Coordinate>& extractedPoints);

    static std::size_t next(std::size_t i, std::size_t size);


    /* private */
    class MatchCurveSegmentAction : public index::chain::MonotoneChainSelectAction
    {

    private:

        const Coordinate& p0;
        const Coordinate& p1;
        const CoordinateSequence* bufferPts;
        double matchDistance;
        std::vector<bool>& isInCurve;
        double minFrac = -1;
        int minCurveIndex = -1;

    public:

        MatchCurveSegmentAction(
            const Coordinate& p_p0, const Coordinate& p_p1,
            const CoordinateSequence* p_bufferPts, double p_matchDistance,
            std::vector<bool>& p_isInCurve)
            : p0(p_p0)
            , p1(p_p1)
            , bufferPts(p_bufferPts)
            , matchDistance(p_matchDistance)
            , isInCurve(p_isInCurve)
            , minFrac(-1)
            , minCurveIndex(-1)
            {};

        void select(const index::chain::MonotoneChain& mc, std::size_t segIndex) override;
        void select(const geom::LineSegment& seg) override { (void)seg; return; };

        int getMinCurveIndex() { return minCurveIndex; }
    };


public:

    /**
    * Creates a new instance for computing an offset curve for a geometryat a given distance.
    * with default quadrant segments BufferParameters::DEFAULT_QUADRANT_SEGMENTS
    * and join style BufferParameters::JOIN_STYLE.
    *
    * @param geom the geometry to offset
    * @param dist the offset distance (positive = left, negative = right)
    *
    * \see BufferParameters
    */
    OffsetCurve(const Geometry& geom, double dist)
        : inputGeom(geom)
        , distance(dist)
        , matchDistance(std::abs(dist)/NEARNESS_FACTOR)
        , geomFactory(geom.getFactory())
        {};

    /**
    * Creates a new instance for computing an offset curve for a geometry at a given distance.
    * allowing the quadrant segments and join style and mitre limit to be set
    * via BufferParameters.
    *
    * @param geom
    * @param dist
    * @param bp
    */
    OffsetCurve(const Geometry& geom, double dist, BufferParameters& bp)
        : inputGeom(geom)
        , distance(dist)
        , bufferParams(bp)
        , matchDistance(std::abs(dist)/NEARNESS_FACTOR)
        , geomFactory(geom.getFactory())
        {};

    /**
    * Computes the offset curve of a geometry at a given distance,
    * and for a specified quadrant segments, join style and mitre limit.
    *
    * @param geom a geometry
    * @param dist the offset distance (positive = left, negative = right)
    * @param quadSegs the quadrant segments (-1 for default)
    * @param joinStyle the join style (-1 for default)
    * @param mitreLimit the mitre limit (-1 for default)
    * @return the offset curve
    */
    static std::unique_ptr<Geometry> getCurve(
        const Geometry& geom,
        double dist, int quadSegs, BufferParameters::JoinStyle joinStyle, double mitreLimit);

    static std::unique_ptr<Geometry> getCurve(const Geometry& geom, double dist);
    std::unique_ptr<Geometry> getCurve();

    /**
    * Gets the raw offset line.
    * The quadrant segments and join style and mitre limit to be set
    * via BufferParameters.
    *
    * The raw offset line may contain loops and other artifacts which are
    * not present in the true offset curve.
    * The raw offset line is matched to the buffer ring (which is clean)
    * to extract the offset curve.
    *
    * @param geom the linestring to offset
    * @param dist the offset distance
    * @param bufParams the buffer parameters to use
    * @param lineList the vector to populate with the return value
    * @return the raw offset line
    */
    static void rawOffset(const LineString& geom, double dist, BufferParameters& bufParams, std::vector<CoordinateSequence*>& lineList);
    static void rawOffset(const LineString& geom, double dist, std::vector<CoordinateSequence*>& lineList);

};


} // namespace geos::operation::buffer
} // namespace geos::operation
} // namespace geos

#ifdef _MSC_VER
#pragma warning(pop)
#endif


