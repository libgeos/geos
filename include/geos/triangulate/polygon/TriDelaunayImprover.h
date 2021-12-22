/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/triangulate/tri/TriEdge.h>
#include <geos/triangulate/tri/Tri.h>
#include <geos/triangulate/tri/TriList.h>


// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
}
}

using geos::geom::Coordinate;
using geos::triangulate::tri::TriList;
using geos::triangulate::tri::Tri;


namespace geos {
namespace triangulate {
namespace polygon {


/**
 * Improves the quality of a triangulation of Tri via
 * iterated Delaunay flipping.
 * This produces the Constrained Delaunay Triangulation
 * with the constraints being the boundary of the input triangulation.
 *
 * @author mdavis
 *
 */
class GEOS_DLL TriDelaunayImprover {

private:

    // Members
    static constexpr std::size_t MAX_ITERATION = 200;
    TriList<Tri>& triList;

    /**
    * Improves a triangulation by examining pairs of adjacent triangles
    * (forming a quadrilateral) and testing if flipping the diagonal of
    * the quadrilateral would produce two new triangles with larger minimum
    * interior angles.
    *
    * @return the number of flips that were made
    */
    std::size_t improveScan(TriList<Tri>& triList);

    /**
    * Does a flip of the common edge of two Tris if the Delaunay condition is not met.
    *
    * @param tri0 a Tri
    * @param tri1 a Tri
    * @return true if the triangles were flipped
    */
    bool improveNonDelaunay(Tri* tri, TriIndex index);

    /**
    * Tests if the quadrilateral formed by two adjacent triangles is convex.
    * opp0-adj0-adj1 and opp1-adj1-adj0 are the triangle corners
    * and hence are known to be convex.
    * The quadrilateral is convex if the other corners opp0-adj0-opp1
    * and opp1-adj1-opp0 have the same orientation (since at least one must be convex).
    *
    * @param adj0 adjacent edge vertex 0
    * @param adj1 adjacent edge vertex 1
    * @param opp0 corner vertex of triangle 0
    * @param opp1 corner vertex of triangle 1
    * @return true if the quadrilateral is convex
    */
    static bool isConvex(const Coordinate& adj0, const Coordinate& adj1,
                         const Coordinate& opp0, const Coordinate& opp1);

    /**
    * Tests if either of a pair of adjacent triangles satisfy the Delaunay condition.
    * The triangles are opp0-adj0-adj1 and opp1-adj1-adj0.
    * The Delaunay condition is not met if one opposite vertex
    * lies is in the circumcircle of the other triangle.
    *
    * @param adj0 adjacent edge vertex 0
    * @param adj1 adjacent edge vertex 1
    * @param opp0 corner vertex of triangle 0
    * @param opp1 corner vertex of triangle 1
    * @return true if the triangles are Delaunay
    */
    static bool isDelaunay(const Coordinate& adj0, const Coordinate& adj1,
                           const Coordinate& opp0, const Coordinate& opp1);

    /**
    * Tests whether a point p is in the circumcircle of a triangle abc
    * (oriented clockwise).
    * @param a a vertex of the triangle
    * @param b a vertex of the triangle
    * @param c a vertex of the triangle
    * @param p the point
    *
    * @return true if the point is in the circumcircle
    */
    static bool
    isInCircle(const Coordinate& a, const Coordinate& b,
               const Coordinate& c, const Coordinate& p);

    void improve();


public:

    TriDelaunayImprover(TriList<Tri>& p_triList)
        : triList(p_triList)
        {};

    /**
    * Improves the quality of a triangulation of Tri via
    * iterated Delaunay flipping.
    * The Tris are assumed to be linked into a Triangulation
    * (e.g. via TriangulationBuilder).
    *
    * @param triList the list of Tris to flip.
    */
    static void improve(TriList<Tri>& triList);


};



} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos

