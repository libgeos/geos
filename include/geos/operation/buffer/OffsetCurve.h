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

#include <geos/operation/buffer/BufferParameters.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/constants.h>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class Geometry;
class LineString;
class Polygon;
}
namespace operation {
namespace buffer {
class OffsetCurveSection;
class SegmentMCIndex;
}
}
}

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineString;
using geos::geom::Polygon;

namespace geos {
namespace operation {
namespace buffer {

/**
 * Computes an offset curve from a geometry.
 * An offset curve is a linear geometry which is offset a given distance
 * from the input.
 * If the offset distance is positive the curve lies on the left side of the input;
 * if it is negative the curve is on the right side.
 * The curve(s) have the same direction as the input line(s).
 *
 * The offset curve is based on the boundary of the buffer for the geometry
 * at the offset distance (see BufferOp).
 * The normal mode of operation is to return the sections of the buffer boundary
 * which lie on the raw offset curve
 * (obtained via rawOffset(LineString, double).
 * The offset curve will contain multiple sections
 * if the input self-intersects or has close approaches.
 * The computed sections are ordered along the raw offset curve.
 * Sections are disjoint.  They never self-intersect, but may be rings.
 *
 *   * For a LineString the offset curve is a linear geometry
 *     (LineString or MultiLineString).
 *   * For a Point or MultiPoint the offset curve is an empty LineString.
 *   * For a Polygon the offset curve is the boundary of the polygon buffer (which
 *     may be a MultiLineString.
 *   * For a collection the output is a MultiLineString containing
 *     the offset curves of the elements.
 *
 * In "joined" mode (see setJoined(bool))
 * the sections computed for each input line are joined into a single offset curve line.
 * The joined curve may self-intersect.
 * At larger offset distances the curve may contain "flat-line" artifacts
 * in places where the input self-intersects.
 *
 * Offset curves support setting the number of quadrant segments,
 * the join style, and the mitre limit (if applicable) via
 * the BufferParameters.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL OffsetCurve {


private:

    // Members
    const Geometry& inputGeom;
    double distance;
    bool isJoined = false;

    BufferParameters bufferParams;
    double matchDistance;
    const GeometryFactory* geomFactory;

    // Methods

    std::unique_ptr<Geometry> computeCurve(
        const LineString& lineGeom, double distance);

    std::vector<std::unique_ptr<OffsetCurveSection>> computeSections(
        const LineString& lineGeom, double distance);

    std::unique_ptr<LineString> offsetSegment(
        const CoordinateSequence* pts, double distance);

    static std::unique_ptr<Polygon> getBufferOriented(
        const LineString& geom, double distance,
        BufferParameters& bufParams);

    /**
    * Extracts the largest polygon by area from a geometry.
    * Used here to avoid issues with non-robust buffer results
    * which have spurious extra polygons.
    *
    * @param geom a geometry
    * @return the polygon element of largest area
    */
    static const Polygon* extractMaxAreaPolygon(const Geometry* geom);

    void computeCurveSections(
        const CoordinateSequence* bufferRingPts,
        const CoordinateSequence& rawCurve,
        std::vector<std::unique_ptr<OffsetCurveSection>>& sections);

    /**
    * Matches the segments in a buffer ring to the raw offset curve
    * to obtain their match positions (if any).
    *
    * @param raw0 a raw curve segment start point
    * @param raw1 a raw curve segment end point
    * @param rawCurveIndex the index of the raw curve segment
    * @param bufferSegIndex the spatial index of the buffer ring segments
    * @param bufferPts the points of the buffer ring
    * @param rawCurvePos the raw curve positions of the buffer ring segments
    * @return the index of the minimum matched buffer segment
    */
    std::size_t matchSegments(
        const Coordinate& raw0, const Coordinate& raw1,
        std::size_t rawCurveIndex,
        SegmentMCIndex& bufferSegIndex,
        const CoordinateSequence* bufferPts,
        std::vector<double>& rawCurvePos);

    static double segmentMatchFrac(
        const Coordinate& p0,   const Coordinate& p1,
        const Coordinate& seg0, const Coordinate& seg1,
        double matchDistance);

    /**
    * This is only called when there is at least one ring segment matched
    * (so rawCurvePos has at least one entry != NOT_IN_CURVE).
    * The start index of the first section must be provided.
    * This is intended to be the section with lowest position
    * along the raw curve.
    * @param ringPts the points in a buffer ring
    * @param rawCurveLoc the position of buffer ring segments along the raw curve
    * @param startIndex the index of the start of a section
    * @param sections the list of extracted offset curve sections
    */
    void extractSections(
        const CoordinateSequence* ringPts,
        std::vector<double>& rawCurveLoc,
        std::size_t startIndex,
        std::vector<std::unique_ptr<OffsetCurveSection>>& sections);

    std::size_t findSectionStart(
        const std::vector<double>& loc,
        std::size_t end);

    std::size_t findSectionEnd(
        const std::vector<double>& loc,
        std::size_t start,
        std::size_t firstStartIndex);

    static std::size_t nextIndex(std::size_t i, std::size_t size);
    static std::size_t prevIndex(std::size_t i, std::size_t size);


public:

    // Constants
    static constexpr int MATCH_DISTANCE_FACTOR = 10000;

    /**
    * A QuadSegs minimum value that will prevent generating
    * unwanted offset curve artifacts near end caps.
    */
    static constexpr int MIN_QUADRANT_SEGMENTS = 8;

    /**
    * Creates a new instance for computing an offset curve for a geometry at a given distance.
    * with default quadrant segments (BufferParameters::DEFAULT_QUADRANT_SEGMENTS)
    * and join style (BufferParameters::JOIN_STYLE).
    *
    * @param geom the geometry to offset
    * @param dist the offset distance (positive for left, negative for right)
    *
    * @see BufferParameters
    */
    OffsetCurve(const Geometry& geom, double dist)
        : inputGeom(geom)
        , distance(dist)
        , matchDistance(std::abs(dist)/MATCH_DISTANCE_FACTOR)
        , geomFactory(geom.getFactory())
        {
            if (!std::isfinite(dist)) {
                throw util::IllegalArgumentException("OffsetCurve distance must be a finite value");
            }
        };

    /**
    * Creates a new instance for computing an offset curve for a geometry at a given distance.
    * setting the quadrant segments and join style and mitre limit
    * via {@link BufferParameters}.
    *
    * @param geom the geometry to offset
    * @param dist the offset distance (positive for left, negative for right)
    * @param bp the buffer parameters to use
    */
    OffsetCurve(const Geometry& geom, double dist, BufferParameters& bp)
        : inputGeom(geom)
        , distance(dist)
        , matchDistance(std::abs(dist)/MATCH_DISTANCE_FACTOR)
        , geomFactory(geom.getFactory())
        {
            if (!std::isfinite(dist)) {
                throw util::IllegalArgumentException("OffsetCurve distance must be a finite value");
            }
            //-- set buffer params, leaving cap style as the default CAP_ROUND

            /**
            * Prevent using a very small QuadSegs value, to avoid
            * offset curve artifacts near the end caps.
            */
            int quadSegs = bp.getQuadrantSegments();
            if (quadSegs < MIN_QUADRANT_SEGMENTS) {
                quadSegs = MIN_QUADRANT_SEGMENTS;
            }
            bufferParams.setQuadrantSegments(quadSegs);

            bufferParams.setJoinStyle( bp.getJoinStyle());
            bufferParams.setMitreLimit( bp.getMitreLimit());
        };

    /**
    * Computes a single curve line for each input linear component,
    * by joining curve sections in order along the raw offset curve.
    * The default mode is to compute separate curve sections.
    *
    * @param pIsJoined true if joined mode should be used.
    */
    void setJoined(bool pIsJoined);

    static std::unique_ptr<Geometry> getCurve(
        const Geometry& geom,
        double dist,
        int quadSegs,
        BufferParameters::JoinStyle joinStyle,
        double mitreLimit);

    static std::unique_ptr<Geometry> getCurve(
        const Geometry& geom, double dist);

    /**
    * Computes the offset curve of a geometry at a given distance,
    * joining curve sections into a single line for each input line.
    *
    * @param geom a geometry
    * @param dist the offset distance (positive for left, negative for right)
    * @return the joined offset curve
    */
    static std::unique_ptr<Geometry> getCurveJoined(
        const Geometry& geom, double dist);

    /**
    * Gets the computed offset curve lines.
    *
    * @return the offset curve geometry
    */
    std::unique_ptr<Geometry> getCurve();

    /**
    * Gets the raw offset curve for a line at a given distance.
    * The quadrant segments, join style and mitre limit can be specified
    * via BufferParameters.
    *
    * The raw offset line may contain loops and other artifacts which are
    * not present in the true offset curve.
    *
    * @param line the line to offset
    * @param distance the offset distance (positive for left, negative for right)
    * @param bufParams the buffer parameters to use
    * @return the raw offset curve points
    */
    static std::unique_ptr<CoordinateSequence> rawOffsetCurve(
        const LineString& line,
        double distance,
        BufferParameters& bufParams);

    /**
    * Gets the raw offset curve for a line at a given distance,
    * with default buffer parameters.
    *
    * @param line the line to offset
    * @param distance the offset distance (positive for left, negative for right)
    * @return the raw offset curve points
    */
    static std::unique_ptr<CoordinateSequence> rawOffset(
        const LineString& line,
        double distance);

};

} // namespace geos::operation::buffer
} // namespace geos::operation
} // namespace geos



