/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/geom/LineSegment.h>

#include <memory>
#include <geos/export.h>

// Forward declarations
namespace geos {
namespace geom {
class GeometryFactory;
class Geometry;
class LineString;
class CoordinateSequence;
}
}


namespace geos {
namespace algorithm { // geos::algorithm

/** \brief
 * Computes the minimum diameter of a geom::Geometry
 *
 * The minimum diameter is defined to be the width of the smallest band that
 * contains the geometry, where a band is a strip of the plane defined
 * by two parallel lines. This can be thought of as the smallest hole
 * that the geometry can be moved through, with a single rotation.
 *
 * The first step in the algorithm is computing the convex hull of the Geometry.
 * If the input Geometry is known to be convex, a hint can be supplied to
 * avoid this computation.
 *
 * This class can also be used to compute a line segment representing
 * the minimum diameter, the supporting line segment of the minimum diameter,
 * and a minimum-width rectangle of the input geometry.
 * This rectangle will have width equal to the minimum diameter, and have
 * one side parallel to the supporting segment.
 *
 * In degenerate cases the rectangle may be a LineString or a Point.
 * (Note that this may not be the enclosing rectangle with minimum area;
 * use MinimumAreaRectangle to compute this.)
 *
 * @see ConvexHull
 * @see MinimumAreaRectangle
 */
class GEOS_DLL MinimumDiameter {
private:
    const geom::Geometry* inputGeom;
    bool isConvex;

    std::unique_ptr<geom::CoordinateSequence> convexHullPts;

    geom::LineSegment minBaseSeg;
    geom::Coordinate minWidthPt;
    std::size_t minPtIndex;
    double minWidth;
    void computeMinimumDiameter();
    void computeWidthConvex(const geom::Geometry* geom);

    /**
     * Compute the width information for a ring of Coordinate.
     * Leaves the width information in the instance variables.
     *
     * @param pts
     * @return
     */
    void computeConvexRingMinDiameter(const geom::CoordinateSequence* pts);

    unsigned int findMaxPerpDistance(const geom::CoordinateSequence* pts,
                                     const geom::LineSegment* seg, unsigned int startIndex);

    static unsigned int getNextIndex(const geom::CoordinateSequence* pts,
                                     unsigned int index);

    static double computeC(double a, double b, const geom::Coordinate& p);

    static geom::LineSegment computeSegmentForLine(double a, double b, double c);

    static std::unique_ptr<geom::Geometry> computeMaximumLine(
                        const geom::CoordinateSequence* pts,
                        const geom::GeometryFactory* factory);

public:
    ~MinimumDiameter() = default;

    /** \brief
     * Compute a minimum diameter for a given [Geometry](@ref geom::Geometry).
     *
     * @param newInputGeom a Geometry
     */
    MinimumDiameter(const geom::Geometry* newInputGeom);

    /** \brief
     * Compute a minimum diameter for a given Geometry,
     * with a hint if the Geometry is convex
     * (e.g. a convex Polygon or LinearRing,
     * or a two-point LineString, or a Point).
     *
     * @param newInputGeom a Geometry which is convex
     * @param newIsConvex `true` if the input geometry is convex
     */
    MinimumDiameter(const geom::Geometry* newInputGeom,
                    const bool newIsConvex);

    /** \brief
     * Gets the length of the minimum diameter of the input Geometry.
     *
     * @return the length of the minimum diameter
     */
    double getLength();

    /** \brief
     * Gets the geom::Coordinate forming one end of the minimum diameter.
     *
     * @return a coordinate forming one end of the minimum diameter
     */
    const geom::Coordinate& getWidthCoordinate();

    /** \brief
     * Gets the segment forming the base of the minimum diameter.
     *
     * @return the segment forming the base of the minimum diameter
     */
    std::unique_ptr<geom::LineString> getSupportingSegment();

    /** \brief
     * Gets a LineString which is a minimum diameter.
     *
     * @return a LineString which is a minimum diameter
     */
    std::unique_ptr<geom::LineString> getDiameter();

    /** \brief
     * Gets the rectangular Polygon which encloses the input geometry
     * and is based on the minimum diameter supporting segment.
     *
     * The rectangle has width equal to the minimum diameter, and a longer
     * length. If the convex hill of the input is degenerate (a line or point)
     * a LineString or Point is returned.
     * This is not necessarily the rectangle with minimum area.
     * Use MinimumAreaRectangle to compute this.
     *
     * @return the the minimum-width rectangle enclosing the geometry
     * @see MinimumAreaRectangle
     */
    std::unique_ptr<geom::Geometry> getMinimumRectangle();

    /** \brief
     * Gets the minimum rectangle enclosing a geometry.
     *
     * @param geom the geometry
     * @return a rectangle enclosing the input (or a line or point if degenerate)
     * @see MinimumAreaRectangle
    */
    static std::unique_ptr<geom::Geometry> getMinimumRectangle(geom::Geometry* geom);

    /** \brief
     * Gets the length of the minimum diameter enclosing a geometry.
     * @param geom the geometry
     * @return the length of the minimum diameter of the geometry
     */
    static std::unique_ptr<geom::Geometry> getMinimumDiameter(geom::Geometry* geom);

};

} // namespace geos::algorithm
} // namespace geos
