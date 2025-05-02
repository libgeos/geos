/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2020 Martin Davis
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/construct/ExactMaxInscribedCircle.java
 * https://github.com/locationtech/jts/commit/8d5ced1b784d232e1eecf5df4b71873e8822336a
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/geom/LineSegment.h>

#include <array>
// #include <queue>



namespace geos {
namespace geom {
class CoordinateSequence;
class Geometry;
class Polygon;
}
}

namespace geos {
namespace algorithm { // geos::algorithm
namespace construct { // geos::algorithm::construct

/**
 * Computes the Maximum Inscribed Circle for some kinds of convex polygons.
 * It determines the circle center point by computing Voronoi node points
 * and testing them for distance to generating edges.
 * This is more precise than iterated approximation,
 * and faster for small polygons (such as triangles and convex quadrilaterals).
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL ExactMaxInscribedCircle {

    using CoordinateSequence = geos::geom::CoordinateSequence;
    using CoordinateXY = geos::geom::CoordinateXY;
    using LineSegment = geos::geom::LineSegment;
    using Polygon = geos::geom::Polygon;

public:

    ExactMaxInscribedCircle();

    /**
     * Tests whether a given geometry is supported by this class.
     * Currently only triangles and convex quadrilaterals are supported.
     *
     * @param geom an areal geometry
     * @return true if the geometry shape can be evaluated
     */
    static bool isSupported(const geom::Geometry* geom);

    static std::pair<CoordinateXY, CoordinateXY> computeRadius(const Polygon* polygon);


private:

    static bool isTriangle(const Polygon* polygon);

    static bool isQuadrilateral(const Polygon* polygon);

    static std::pair<CoordinateXY, CoordinateXY> computeTriangle(const CoordinateSequence* ring);

    /**
     * The Voronoi nodes of a convex polygon occur at the intersection point
     * of two bisectors of each triplet of edges.
     * The Maximum Inscribed Circle center is the node
     * is the farthest distance from the generating edges.
     * For a quadrilateral there are 4 distinct edge triplets,
     * at each edge with its adjacent edges.
     *
     * @param ring the polygon ring
     * @return an array containing the incircle center and radius points
     */
    static std::pair<CoordinateXY, CoordinateXY> computeConvexQuadrilateral(const CoordinateSequence* ring);

    static std::array<LineSegment, 4> computeBisectors(const CoordinateSequence* ptsCW,
                                                       double diameter);

    static CoordinateXY nearestEdgePt(const CoordinateSequence* ring,
                                      const CoordinateXY& pt);

    static LineSegment computeConvexBisector(const CoordinateSequence* pts,
                                             std::size_t index, double len);

    static bool isConvex(const Polygon* polygon);

    static bool isConvex(const CoordinateSequence* ring);

    static bool isConcave(const CoordinateXY& p0,
                          const CoordinateXY& p1,
                          const CoordinateXY& p2);

    static bool isPointInConvexRing(const CoordinateSequence* ringCW,
                                const CoordinateXY& p);

};


} // geos::algorithm::construct
} // geos::algorithm
} // geos
