/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2021 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>

#include <geos/operation/valid/PolygonTopologyAnalyzer.h>
#include <geos/operation/valid/TopologyValidationError.h>


// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class Geometry;
class Point;
class MultiPoint;
class LineString;
class LinearRing;
class Polygon;
class MultiPolygon;
class GeometryCollection;
}
namespace algorithm {
namespace locate {
class IndexedPointInAreaLocator;
}
}
}


namespace geos {      // geos.
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

/**
 * Implements the algorithms required to compute the <code>isValid()</code> method
 * for Geometry.
 * See the documentation for the various geometry types for a specification of validity.
 *
 * @version 1.7
 */
class GEOS_DLL IsValidOp {

private:

    static constexpr int MIN_SIZE_LINESTRING = 2;
    static constexpr int MIN_SIZE_RING = 4;

    /**
    * The geometry being validated
    */
    const geom::Geometry* inputGeometry;
    /**
    * If the following condition is true GEOS will validate
    * inverted shells and exverted holes (the ESRI SDE model)
    */
    bool isInvertedRingValid = false;
    std::unique_ptr<TopologyValidationError> validErr;

    bool hasInvalidError()
    {
        return validErr != nullptr;
    }


    void logInvalid(int code, const geom::Coordinate* pt);

    bool isValidGeometry(const geom::Geometry* g);

    /**
     * Tests validity of a Point.
     */
    bool isValid(const geom::Point* g);

    /**
     * Tests validity of a MultiPoint.
     */
    bool isValid(const geom::MultiPoint* g);

    /**
     * Tests validity of a LineString.
     * Almost anything goes for linestrings!
     */
    bool isValid(const geom::LineString* g);

    /**
     * Tests validity of a LinearRing.
     */
    bool isValid(const geom::LinearRing* g);

    /**
     * Tests the validity of a polygon.
     * Sets the validErr flag.
     */
    bool isValid(const geom::Polygon* g);

    /**
     * Tests validity of a MultiPolygon.
     *
     * @param g
     * @return
     */
    bool isValid(const geom::MultiPolygon* g);

    /**
     * Tests validity of a GeometryCollection.
     *
     * @param gc
     * @return
     */
    bool isValid(const geom::GeometryCollection* gc);

    void checkCoordinateInvalid(const geom::CoordinateSequence* coords);
    void checkCoordinateInvalid(const geom::Polygon* poly);
    void checkRingNotClosed(const geom::LinearRing* ring);
    void checkRingsNotClosed(const geom::Polygon* poly);
    void checkRingsTooFewPoints(const geom::Polygon* poly);
    void checkRingTooFewPoints(const geom::LinearRing* ring);

    /**
     * Check the number of non-repeated points is at least a given size.
     *
     * @param line
     * @param minSize
     */
    void checkTooFewPoints(const geom::LineString* line, std::size_t minSize);

    /**
     * Test if the number of non-repeated points in a line
     * is at least a given minimum size.
     *
     * @param line the line to test
     * @param minSize the minimum line size
     * @return true if the line has the required number of non-repeated points
     */
    bool isNonRepeatedSizeAtLeast(const geom::LineString* line, std::size_t minSize);

    void checkAreaIntersections(PolygonTopologyAnalyzer& areaAnalyzer);

    /**
     * Check whether a ring self-intersects (except at its endpoints).
     *
     * @param ring the linear ring to check
     */
    void checkSelfIntersectingRing(const geom::LinearRing* ring);


    /**
     * Tests that each hole is inside the polygon shell.
     * This routine assumes that the holes have previously been tested
     * to ensure that all vertices lie on the shell or on the same side of it
     * (i.e. that the hole rings do not cross the shell ring).
     * Given this, a simple point-in-polygon test of a single point in the hole can be used,
     * provided the point is chosen such that it does not lie on the shell.
     *
     * @param poly the polygon to be tested for hole inclusion
     */
    void checkHolesOutsideShell(const geom::Polygon* poly);

    /**
     * Checks if a polygon hole lies inside its shell
     * and if not returns a point indicating this.
     * The hole is known to be wholly inside or outside the shell,
     * so it suffices to find a single point which is interior or exterior,
     * or check the edge topology at a point on the boundary of the shell.
     *
     * @param hole the hole to test
     * @param shell the polygon shell to test against
     * @return a hole point outside the shell, or null if it is inside
     */
    const Coordinate * findHoleOutsideShellPoint(
        const geom::LinearRing* hole,
        const geom::LinearRing* shell);

    /**
     * Checks if any polygon hole is nested inside another.
     * Assumes that holes do not cross (overlap),
     * This is checked earlier.
     *
     * @param poly the polygon with holes to test
     */
    void checkHolesNested(const geom::Polygon* poly);

    /**
     * Checks that no element polygon is in the interior of another element polygon.
     *
     * Preconditions:
     *
     *  * shells do not partially overlap
     *  * shells do not touch along an edge
     *  * no duplicate rings exist
     *
     * These have been confirmed by the PolygonTopologyAnalyzer.
     */
    void checkShellsNested(const geom::MultiPolygon* mp);

    void checkInteriorDisconnected(PolygonTopologyAnalyzer& areaAnalyzer);


public:

    /**
     * Creates a new validator for a geometry.
     *
     * @param p_inputGeometry the geometry to validate
     */
    IsValidOp(const geom::Geometry* p_inputGeometry)
        : inputGeometry(p_inputGeometry)
        , validErr(nullptr)
        {};

    /**
     * Sets whether polygons using <b>Self-Touching Rings</b> to form
     * holes are reported as valid.
     * If this flag is set, the following Self-Touching conditions
     * are treated as being valid:
     *
     *  * the shell ring self-touches to create a hole touching the shell
     *  * a hole ring self-touches to create two holes touching at a point
     *
     * The default (following the OGC SFS standard)
     * is that this condition is not valid (false).
     *
     * Self-Touching Rings which disconnect the
     * the polygon interior are still considered to be invalid
     * (these are <b>invalid</b> under the SFS, and many other
     * spatial models as well).
     * This includes:
     *
     *  * exverted ("bow-tie") shells which self-touch at a single point
     *  * inverted shells with the inversion touching the shell at another point
     *  * exverted holes with exversion touching the hole at another point
     *  * inverted ("C-shaped") holes which self-touch at a single point causing an island to be formed
     *  * inverted shells or exverted holes which form part of a chain of touching rings
     *    (which disconnect the interior)
     *
     * @param p_isValid states whether geometry with this condition is valid
     */
    void setSelfTouchingRingFormingHoleValid(bool p_isValid)
    {
        isInvertedRingValid = p_isValid;
    };

    /**
     * Tests whether a Geometry is valid.
     * @param geom the Geometry to test
     * @return true if the geometry is valid
     */
    static bool isValid(const geom::Geometry* geom)
    {
        IsValidOp ivo(geom);
        return ivo.isValid();
    };

    static bool isValid(const geom::Coordinate& coord)
    {
        return isValid(&coord);
    }

    /**
     * Tests the validity of the input geometry.
     *
     * @return true if the geometry is valid
     */
    bool isValid();

    /**
     * Checks whether a coordinate is valid for processing.
     * Coordinates are valid if their x and y ordinates are in the
     * range of the floating point representation.
     *
     * @param coord the coordinate to validate
     * @return <code>true</code> if the coordinate is valid
     */
    static bool isValid(const geom::Coordinate* coord);

    /**
     * Computes the validity of the geometry,
     * and if not valid returns the validation error for the geometry,
     * or null if the geometry is valid.
     *
     * @return the validation error, if the geometry is invalid
     * or null if the geometry is valid
     */
    const TopologyValidationError* getValidationError();


};


} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

