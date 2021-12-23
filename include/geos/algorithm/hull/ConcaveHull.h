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

#include <geos/geom/Triangle.h>
#include <geos/triangulate/tri/Tri.h>
#include <geos/triangulate/tri/TriList.h>
#include <geos/triangulate/quadedge/TriangleVisitor.h>

#include <queue>
#include <deque>

namespace geos {
namespace geom {
class Coordinate;
class Geometry;
class GeometryFactory;
}
namespace triangulate {
namespace quadedge {
class Quadedge;
class QuadEdgeSubdivision;
}
}
}

using geos::geom::Coordinate;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::Triangle;
using geos::triangulate::quadedge::QuadEdge;
using geos::triangulate::quadedge::QuadEdgeSubdivision;
using geos::triangulate::quadedge::TriangleVisitor;
using geos::triangulate::tri::Tri;
using geos::triangulate::tri::TriList;

namespace geos {
namespace algorithm { // geos::algorithm
namespace hull {      // geos::algorithm::hull



class HullTri : public Tri
{
    private:

        double m_size;
        bool m_isMarked = false;

    public:

        HullTri(const Coordinate& c0, const Coordinate& c1, const Coordinate& c2)
            : Tri(c0, c1, c2)
            , m_size(Triangle::longestSideLength(c0, c1, c2))
            {};

        double getSize() const;
        bool isMarked() const;
        void setSizeToBorder();
        void setMarked(bool marked);
        bool isBorder() const;
        bool isBorder(TriIndex index) const;
        TriIndex borderIndex() const;
        double lengthOfLongestEdge() const;
        double lengthOfBorder() const;
        HullTri* nextBorderTri() ;
        int compareTo(const HullTri* o) const;
        static bool isConnected(TriList<HullTri>& triList, HullTri* exceptTri);
        static void clearMarks(TriList<HullTri>& triList);
        static HullTri* findTri(TriList<HullTri>& triList, Tri* exceptTri);
        static bool isAllMarked(TriList<HullTri>& triList);
        static void markConnected(HullTri* triStart, HullTri* exceptTri);

        /**
        * Gets the most CCW border edge index.
        * This assumes there is at least one non-border edge.
        *
        * @return the CCW border edge index
        */
        TriIndex borderIndexCCW() const;

        /**
        * Gets the most CW border edge index.
        * This assumes there is at least one non-border edge.
        *
        * @return the CW border edge index
        */
        TriIndex borderIndexCW() const;

}; // HullTri


class HullTriVisitor : public TriangleVisitor
{

    private:

        TriList<HullTri>& triList;

    public:

        HullTriVisitor(TriList<HullTri>& p_triList)
            : triList(p_triList)
            {};

        void visit(std::array<QuadEdge*, 3>& triEdges);

}; // HullTriVisitor


// Sort in inverse order (largest to smallest) in
// the std::priority_queue
struct HullTriCompare {
    bool operator()(const HullTri* a, const HullTri* b)
    {
        return a->getSize() < b->getSize();
    }
};

typedef std::priority_queue<HullTri*, std::vector<HullTri*>, HullTriCompare> HullTriQueue;


/**
* Constructs a concave hull of a set of points.
* The hull is constructed by eroding the Delaunay Triangulation of the points
* until specified target criteria are reached.
* The target criteria are:
*
*  * Maximum Edge Length - the length of the longest edge of the hull will be
*    no larger than this value
*  * Maximum Area Ratio - the ratio of the concave hull area to the convex
*    hull area will be no larger than this value
*
* Usually only a single criteria is specified, but both may be provided.
*
* The computed hull is always a single connected Polygon.
* This constraint may cause the concave hull to not fully meet the target criteria.
*
* Optionally the concave hull can be allowed to contain holes.
* Note that this may be substantially slower than not permitting holes,
* and it can produce results of low quality.
*
* @author mdavis
*/
class GEOS_DLL ConcaveHull {

public:

    ConcaveHull(const Geometry* geom)
        : inputGeometry(geom)
        , maxEdgeLength(0.0)
        , maxAreaRatio(0.0)
        , isHolesAllowed(false)
        , geomFactory(geom->getFactory())
        {};

    /**
    * Computes the approximate edge length of
    * a uniform square grid having the same number of
    * points as a geometry and the same area as its convex hull.
    * This value can be used to determine a suitable length threshold value
    * for computing a concave hull.
    * A value from 2 to 4 times the uniform grid length
    * seems to produce reasonable results.
    *
    * @param geom a geometry
    * @return the approximate uniform grid length
    */
    static double uniformEdgeLength(const Geometry* geom);

    /**
    * Computes the concave hull of the vertices in a geometry
    * using the target criteria of maximum edge length.
    *
    * @param geom the input geometry
    * @param maxLength the target maximum edge length
    * @return the concave hull
    */
    static std::unique_ptr<Geometry> concaveHullByLength(
        const Geometry* geom, double maxLength);

    static std::unique_ptr<Geometry> concaveHullByLength(
        const Geometry* geom, double maxLength, bool isHolesAllowed);

    /**
    * Computes the concave hull of the vertices in a geometry
    * using the target criteria of maximum area ratio.
    *
    * @param geom the input geometry
    * @param areaRatio the target maximum area ratio
    * @return the concave hull
    */
    static std::unique_ptr<Geometry> concaveHullByArea(
        const Geometry* geom, double areaRatio);

    /**
    * Sets the target maximum edge length for the concave hull.
    * A value of 0.0 produces a concave hull of minimum area
    * that is still connected.
    * The uniformEdgeLength() may be used as
    * the basis for estimating an appropriate target maximum edge length.
    *
    * @param edgeLength a non-negative length
    *
    * @see uniformEdgeLength()
    */
    void setMaximumEdgeLength(double edgeLength);

    /**
    * Sets the target maximum concave hull area as a ratio of the convex hull area.
    * A value of 1.0 produces the convex hull
    * (unless a maximum edge length is also specified).
    * A value of 0.0 produces a concave hull with the smallest area
    * that is still connected.
    *
    * @param areaRatio a ratio value between 0 and 1
    */
    void setMaximumAreaRatio(double areaRatio);

    /**
    * Sets whether holes are allowed in the concave hull polygon.
    *
    * @param isHolesAllowed true if holes are allowed in the result
    */
    void setHolesAllowed(bool p_isHolesAllowed);

    /**
    * Gets the computed concave hull.
    *
    * @return the concave hull
    */
    std::unique_ptr<Geometry> getHull();

    static HullTri* findBorderTri(TriList<HullTri>& triList);


private:

    // Members
    const Geometry* inputGeometry;
    double maxEdgeLength;
    double maxAreaRatio;
    bool isHolesAllowed;
    const GeometryFactory* geomFactory;

    void computeHull(TriList<HullTri>& triList);
    void initQueue(HullTriQueue& queue, TriList<HullTri>& triList);

    /**
    * Adds a Tri to the queue.
    * Only add tris with a single border edge.
    * The ordering size is the length of the border edge.
    *
    * @param tri the Tri to add
    * @param queue the priority queue
    */
    void addBorderTri(HullTri* tri, HullTriQueue& queue);
    bool isBelowAreaThreshold(double areaConcave, double areaConvex) const;
    bool isBelowLengthThreshold(const HullTri* tri) const;

    /**
    * Tests whether a Tri can be removed while preserving
    * the connectivity of the hull.
    *
    * @param tri the Tri to test
    * @param triList
    * @return true if the Tri can be removed
    */
    bool isRemovable(HullTri* tri, TriList<HullTri>& triList);

    static bool hasVertexSingleAdjacent(
        const HullTri* tri,
        const TriList<HullTri>& triList);

    /**
    * The degree of a Tri vertex is the number of tris containing it.
    * This must be done by searching the entire triangulation,
    * since the containing tris may not be adjacent or edge-connected.
    *
    * @param v a vertex coordinate
    * @param triList the triangulation
    * @return the degree of the vertex
    */
    static std::size_t degree(
        const Coordinate& v,
        const TriList<HullTri>& triList);

    /**
    * Tests if a tri is the only one connecting its 2 adjacents.
    * Assumes that the tri is on the border of the triangulation
    * and that the triangulation does not contain holes
    *
    * @param tri the tri to test
    * @return true if the tri is the only connection
    */
    static bool isConnecting(const HullTri* tri);

    /**
    * A vertex of a triangle is interior if it
    * is fully surrounded by triangles.
    *
    * @param tri a tri containing the vertex
    * @param index the vertex index
    * @return true if the vertex is interior
    */
    static bool isInteriorVertex(
        const HullTri* triStart, TriIndex index);

    static TriIndex adjacent2VertexIndex(const HullTri* tri);

    static void createDelaunayTriangulation(
        const Geometry* geom, TriList<HullTri>& triList);

    static void toTris(
        QuadEdgeSubdivision& subdiv,
        TriList<HullTri>& triList);

    std::unique_ptr<Geometry> toPolygon(
        TriList<HullTri>& triList,
        const GeometryFactory* factory);

    std::unique_ptr<Geometry> extractPolygon(
        TriList<HullTri>& triList,
        const GeometryFactory* factory);

    static std::unique_ptr<Geometry> geomunion(
        TriList<HullTri>& triList,
        const GeometryFactory* factory);

    /**
    * Extracts the coordinates along the border of a triangulation,
    * by tracing CW around the border triangles.
    * Assumption: there are at least 2 tris, they are connected,
    * and there are no holes.
    * So each tri has at least one non-border edge, and there is only one border.
    *
    * @param triList the triangulation
    * @return the border of the triangulation
    */
    static std::vector<Coordinate> traceBorder(
        TriList<HullTri>& triList);

};


} // geos::algorithm::hull
} // geos::algorithm
} // geos

