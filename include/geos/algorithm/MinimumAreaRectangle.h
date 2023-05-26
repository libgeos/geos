/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <vector>
#include <memory>

// Forward declarations
namespace geos {
	namespace geom {
        class CoordinateSequence;
        class CoordinateXY;
        class Geometry;
        class GeometryFactory;
        class LineSegment;
        class LineString;
        class Polygon;
	}
}

using geos::geom::CoordinateSequence;
using geos::geom::CoordinateXY;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::LineSegment;
using geos::geom::LineString;
using geos::geom::Polygon;


namespace geos {
namespace algorithm { // geos::algorithm


/**
 * Computes the minimum-area rectangle enclosing a Geometry.
 * Unlike the Envelope, the rectangle may not be axis-parallel.
 *
 * The first step in the algorithm is computing the convex hull of the Geometry.
 * If the input Geometry is known to be convex, a hint can be supplied to
 * avoid this computation.
 *
 * In degenerate cases the minimum enclosing geometry
 * may be a LineString or a Point.
 *
 * The minimum-area enclosing rectangle does not necessarily
 * have the minimum possible width.
 * Use MinimumDiameter to compute this.
 *
 * @see MinimumDiameter
 * @see ConvexHull
 *
 */
class GEOS_DLL MinimumAreaRectangle {

private:

    // Members
    const Geometry* m_inputGeom;
    bool m_isConvex;

    // Methods
    std::unique_ptr<Geometry> getMinimumRectangle();

    std::unique_ptr<Geometry> computeConvex(const Geometry* convexGeom);

    /**
    * Computes the minimum-area rectangle for a convex ring of Coordinate.
    *
    * This algorithm uses the "dual rotating calipers" technique.
    * Performance is linear in the number of segments.
    *
    * @param ring the convex ring to scan
    */
    std::unique_ptr<Polygon> computeConvexRing(const CoordinateSequence* ring);

    std::size_t findFurthestVertex(
        const CoordinateSequence* pts,
        const LineSegment& baseSeg,
        std::size_t startIndex,
        int orient);

    bool isFurtherOrEqual(double d1, double d2, int orient);

    static double orientedDistance(
        const LineSegment& seg,
        const CoordinateXY& p,
        int orient);

    static std::size_t getNextIndex(
        const CoordinateSequence* ring,
        std::size_t index);

    /**
    * Creates a line of maximum extent from the provided vertices
    * @param pts the vertices
    * @param factory the geometry factory
    * @return the line of maximum extent
    */
    static std::unique_ptr<LineString> computeMaximumLine(
        const CoordinateSequence* pts,
        const GeometryFactory* factory);


public:

    /**
    * Compute a minimum-area rectangle for a given Geometry.
    *
    * @param inputGeom a Geometry
    */
    MinimumAreaRectangle(const Geometry* inputGeom)
        : m_inputGeom(inputGeom)
        , m_isConvex(false)
        {};

    /**
    * Compute a minimum rectangle for a Geometry,
    * with a hint if the geometry is convex
    * (e.g. a convex Polygon or LinearRing,
    * or a two-point LineString, or a Point).
    *
    * @param inputGeom a Geometry which is convex
    * @param isConvex true if the input geometry is convex
    */
    MinimumAreaRectangle(const Geometry* inputGeom, bool isConvex)
        : m_inputGeom(inputGeom)
        , m_isConvex(isConvex)
        {};

    /**
    * Gets the minimum-area rectangular Polygon which encloses the input geometry.
    * If the convex hull of the input is degenerate (a line or point)
    * a LineString or Point is returned.
    *
    * @param geom the geometry
    * @return the minimum rectangle enclosing the geometry
    */
    static std::unique_ptr<Geometry> getMinimumRectangle(const Geometry* geom);

};


} // namespace geos::algorithm
} // namespace geos

