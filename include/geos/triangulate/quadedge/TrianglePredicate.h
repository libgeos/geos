/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2012 Excensus LLC.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: triangulate/quadedge/TrianglePredicate.java r524
 *
 **********************************************************************/

#pragma once

#include <geos/export.h>
#include <geos/geom/Location.h>

namespace geos {
namespace geom {
class CoordinateXY;
}
}

namespace geos {
namespace triangulate {
namespace quadedge {

/** \brief
 * Algorithms for computing values and predicates
 * associated with triangles.
 *
 * For some algorithms extended-precision
 * implementations are provided, which are more robust
 * (i.e. they produce correct answers in more cases).
 * Also, some more robust formulations of
 * some algorithms are provided, which utilize
 * normalization to the origin.
 *
 * @author JTS: Martin Davis
 * @author Benjamin Campbell
 *
 */
class GEOS_DLL TrianglePredicate {
public:
    using CoordinateXY = geos::geom::CoordinateXY;

    /**
     * Tests if a point is inside the circle defined by
     * the triangle with vertices a, b, c (oriented counter-clockwise).
     * This test uses simple
     * double-precision arithmetic, and thus may not be robust.
     *
     * @param a a vertex of the triangle
     * @param b a vertex of the triangle
     * @param c a vertex of the triangle
     * @param p the point to test
     * @return true if this point is inside the circle defined by the points a, b, c
     */
    static geom::Location isInCircleNonRobust(
        const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c,
        const CoordinateXY& p);

    /**
     * Tests if a point is inside the circle defined by
     * the triangle with vertices a, b, c (oriented counter-clockwise).
     * This test uses simple
     * double-precision arithmetic, and thus is not 10% robust.
     * However, by using normalization to the origin
     * it provides improved robustness and increased performance.
     * <p>
     * Based on code by J.R.Shewchuk.
     *
     *
     * @param a a vertex of the triangle
     * @param b a vertex of the triangle
     * @param c a vertex of the triangle
     * @param p the point to test
     * @return true if this point is inside the circle defined by the points a, b, c
     */
    static geom::Location isInCircleNormalized(
        const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c,
        const CoordinateXY& p);

private:
    /**
     * Computes twice the area of the oriented triangle (a, b, c), i.e., the area is positive if the
     * triangle is oriented counterclockwise.
     *
     * @param a a vertex of the triangle
     * @param b a vertex of the triangle
     * @param c a vertex of the triangle
     */
    static double triArea(const CoordinateXY& a,
                          const CoordinateXY& b, const CoordinateXY& c);

public:
    /**
     * Tests if a point is inside the circle defined by
     * the triangle with vertices a, b, c (oriented counter-clockwise).
     * This method uses more robust computation.
     *
     * @param a a vertex of the triangle
     * @param b a vertex of the triangle
     * @param c a vertex of the triangle
     * @param p the point to test
     * @return true if this point is inside the circle defined by the points a, b, c
     */
    static geom::Location isInCircleRobust(
        const CoordinateXY& a, const CoordinateXY& b, const CoordinateXY& c,
        const CoordinateXY& p);
} ;


} // namespace geos.triangulate.quadedge
} // namespace geos.triangulate
} // namespace geos


