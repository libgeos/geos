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
#include <geos/algorithm/hull/HullTri.h>

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


typedef std::priority_queue<HullTri*, std::vector<HullTri*>, HullTri::HullTriCompare> HullTriQueue;


/**
* Constructs a concave hull of a set of points.
* The hull is constructed by eroding the Delaunay Triangulation of the points
* until specified target criteria are reached.
* The target criteria are:
*
*  * Maximum Edge Length Ratio - determine the Maximum Edge Length
*    as a fraction of the difference between the longest and shortest edge lengths
*    in the Delaunay Triangulation. This normalizes the Maximum Edge Length to be scale-independent.
*  * Maximum Area Ratio - the ratio of the concave hull area to the convex
*    hull area will be no larger than this value
*
* The preferred criterium is the Maximum Edge Length Ratio, since it is
* scale-free and local (so that no assumption needs to be made about the
* total amount of concavity present.
*
* Other length criteria can be used by setting the Maximum Edge Length.
* For example, use a length relative  to the longest edge length
* in the Minimum Spanning Tree of the point set.
* Or, use a length derived from the uniformGridEdgeLength() value.
*
* The computed hull is always a single connected geom::Polygon
* (unless it is degenerate, in which case it will be a geom::Point or a geom::LineString).
* This constraint may cause the concave hull to fail to meet the target criteria.
*
* Optionally the concave hull can be allowed to contain holes.
* Note that this may be substantially slower than not permitting holes,
* and it can produce results of lower quality.
*
* @author mdavis
*/
class GEOS_DLL ConcaveHull {

public:

    ConcaveHull(const Geometry* geom)
        : inputGeometry(geom)
        , maxEdgeLength(0.0)
        , maxEdgeLengthRatio(-1.0)
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
    * using the target criteria of maximum edge length ratio.
    * The edge length ratio is a fraction of the length difference
    * between the longest and shortest edges
    * in the Delaunay Triangulation of the input points.
    *
    * @param geom the input geometry
    * @param lengthRatio the target edge length factor
    * @return the concave hull
    */
    static std::unique_ptr<Geometry> concaveHullByLengthRatio(
        const Geometry* geom, double lengthRatio);

    /**
    * Computes the concave hull of the vertices in a geometry
    * using the target criterion of maximum edge length factor,
    * and optionally allowing holes.
    * The edge length factor is a fraction of the length difference
    * between the longest and shortest edges
    * in the Delaunay Triangulation of the input points.
    *
    * @param geom the input geometry
    * @param lengthRatio the target maximum edge length
    * @param isHolesAllowed whether holes are allowed in the result
    * @return the concave hull
    */
    static std::unique_ptr<Geometry> concaveHullByLengthRatio(
        const Geometry* geom, double lengthRatio, bool isHolesAllowed);

    /**
    * Sets the target maximum edge length for the concave hull.
    * The length value must be zero or greater.
    *
    *  * The value 0.0 produces the concave hull of smallest area
    *    that is still connected.
    *  * Larger values produce less concave results.
    *    A value equal or greater than the longest Delaunay Triangulation edge length
    *    produces the convex hull.
    *
    * The uniformGridEdgeLength value may be used as
    * the basis for estimating an appropriate target maximum edge length.
    *
    * @param edgeLength a non-negative length
    */
    void setMaximumEdgeLength(double edgeLength);

    /**
    * Sets the target maximum edge length ratio for the concave hull.
    * The edge length ratio is a fraction of the length delta
    * between the longest and shortest edges
    * in the Delaunay Triangulation of the input points.
    * A value of 1.0 produces the convex hull.
    * A value of 0.0 produces a concave hull of minimum area
    * that is still connected.
    *
    * @param edgeLengthRatio a length ratio value between 0 and 1
    */
    void setMaximumEdgeLengthRatio(double edgeLengthRatio);

    /**
    * Sets whether holes are allowed in the concave hull polygon.
    *
    * @param holesAllowed true if holes are allowed in the result
    */
    void setHolesAllowed(bool holesAllowed);

    /**
    * Gets the computed concave hull.
    *
    * @return the concave hull
    */
    std::unique_ptr<Geometry> getHull();


private:

    // Members
    const Geometry* inputGeometry;
    double maxEdgeLength;
    double maxEdgeLengthRatio;
    bool isHolesAllowed;
    const GeometryFactory* geomFactory;

    static double computeTargetEdgeLength(
        TriList<HullTri>& triList,
        double edgeLengthFactor);

    void computeHull(TriList<HullTri>& triList);
    void computeHullBorder(TriList<HullTri>& triList);
    void createBorderQueue(HullTriQueue& queue, TriList<HullTri>& triList);

    /**
    * Adds a Tri to the queue.
    * Only add tris with a single border edge.
    * The ordering size is the length of the border edge.
    *
    * @param tri the Tri to add
    * @param queue the priority queue
    */
    void addBorderTri(HullTri* tri, HullTriQueue& queue);
    bool isBelowLengthThreshold(const HullTri* tri) const;
    void computeHullHoles(TriList<HullTri>& triList);

    static std::vector<HullTri*> findCandidateHoles(
        TriList<HullTri>& triList, double minEdgeLen);

    void removeHole(TriList<HullTri>& triList, HullTri* triHole);

    bool isRemovableBorder(const HullTri* tri) const;
    bool isRemovableHole(const HullTri* tri) const;

    std::unique_ptr<Geometry> toGeometry(
        TriList<HullTri>& triList,
        const GeometryFactory* factory);


};


} // geos::algorithm::hull
} // geos::algorithm
} // geos

