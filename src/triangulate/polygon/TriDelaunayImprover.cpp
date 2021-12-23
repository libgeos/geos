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


#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/triangulate/quadedge/TrianglePredicate.h>
#include <geos/triangulate/tri/Tri.h>
#include <geos/triangulate/tri/TriList.h>
#include <geos/triangulate/polygon/TriDelaunayImprover.h>



namespace geos {
namespace triangulate {
namespace polygon {


/* public static */
void
TriDelaunayImprover::improve(TriList<Tri>& triList)
{
    TriDelaunayImprover improver(triList);
    improver.improve();
}

/* private */
void
TriDelaunayImprover::improve()
{
    for (std::size_t i = 0; i < MAX_ITERATION; i++) {
        std::size_t improveCount = improveScan(triList);
        //System.out.println("improve #" + i + " - count = " + improveCount);
        if (improveCount == 0) {
            return;
        }
    }
}

/* private */
std::size_t
TriDelaunayImprover::improveScan(TriList<Tri>& tris)
{
    std::size_t improveCount = 0;
    if (tris.size() == 0) return 0; // Fend off infinite loop
    for (std::size_t i = 0; i < tris.size() - 1; i++) {
        Tri* tri = tris[i];
        for (TriIndex j = 0; j < 3; j++) {
            if (improveNonDelaunay(tri, j)) {
                improveCount++;
            }
        }
    }
    return improveCount;
}

/* private */
bool
TriDelaunayImprover::improveNonDelaunay(Tri* tri, TriIndex index)
{
    if (tri == nullptr) {
        return false;
    }
    Tri* tri1 = tri->getAdjacent(index);
    if (tri1 == nullptr) {
        return false;
    }
    //tri0.validate();
    //tri1.validate();

    TriIndex index1 = tri1->getIndex(tri);
    const Coordinate& adj0 = tri->getCoordinate(index);
    const Coordinate& adj1 = tri->getCoordinate(Tri::next(index));
    const Coordinate& opp0 = tri->getCoordinate(Tri::oppVertex(index));
    const Coordinate& opp1 = tri1->getCoordinate(Tri::oppVertex(index1));

    /**
     * The candidate new edge is opp0 - opp1.
     * Check if it is inside the quadrilateral formed by the two triangles.
     * This is the case if the quadrilateral is convex.
     */
    if (!isConvex(adj0, adj1, opp0, opp1)) {
        return false;
    }

    /**
     * The candidate edge is inside the quadrilateral. Check to see if the flipping
     * criteria is met. The flipping criteria is to flip if the two triangles are
     * not Delaunay (i.e. one of the opposite vertices is in the circumcircle of the
     * other triangle).
     */
    if (!isDelaunay(adj0, adj1, opp0, opp1)) {
        tri->flip(index);
        return true;
    }
    return false;
}

/* private static */
bool
TriDelaunayImprover::isConvex(
    const Coordinate& adj0, const Coordinate& adj1,
    const Coordinate& opp0, const Coordinate& opp1)
{
    int dir0 = algorithm::Orientation::index(opp0, adj0, opp1);
    int dir1 = algorithm::Orientation::index(opp1, adj1, opp0);
    bool isConvex = (dir0 == dir1);
    return isConvex;
}


/* private static */
bool
TriDelaunayImprover::isDelaunay(
    const Coordinate& adj0, const Coordinate& adj1,
    const Coordinate& opp0, const Coordinate& opp1)
{
    if (isInCircle(adj0, adj1, opp0, opp1)) return false;
    if (isInCircle(adj1, adj0, opp1, opp0)) return false;
    return true;
}


/* private static */
bool
TriDelaunayImprover::isInCircle(
    const Coordinate& a, const Coordinate& b,
    const Coordinate& c, const Coordinate& p)
{
    return triangulate::quadedge::TrianglePredicate::isInCircleRobust(a, c, b, p);
}



} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos

