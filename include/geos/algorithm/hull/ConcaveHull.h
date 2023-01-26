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
* The hull is constructed by removing border triangles
* of the Delaunay Triangulation of the points
* as long as their "size" is larger than the target criterion.
* The target criteria are:
*
*  * Maximum Edge Length Ratio - determines the Maximum Edge Length
*    by a fraction of the difference between
*    the longest and shortest edge lengths
*    in the Delaunay Triangulation. This normalizes the
*    Maximum Edge Length to be scale-independent.
*  * Maximum Area Ratio - the ratio of the concave hull area to the convex
*    hull area will be no larger than this value
*  * Alpha - produces Alpha-shapes, by removing border triangles
*    with a circumradius greater than alpha.
*    Large values produce the convex hull; a value of 0
*    produces maximum concaveness.
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
* Optionally the concave hull can be allowed
* to contain holes by calling setHolesAllowed(boolean).
*
* @author mdavis
*/
class GEOS_DLL ConcaveHull {

public:

    ConcaveHull(const Geometry* geom)
        : inputGeometry(geom)
        , maxEdgeLengthRatio(-1.0)
        , alpha(-1)
        , isHolesAllowed(false)
        , criteriaType(PARAM_EDGE_LENGTH)
        , maxSizeInHull(0.0)
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
        const Geometry* geom,
        double lengthRatio,
        bool isHolesAllowed);

    /**
    * Computes the alpha shape of a geometry as a polygon.
    * The alpha parameter is the radius of the eroding disc.
    *
    * @param geom the input geometry
    * @param alpha the radius of the eroding disc
    * @param isHolesAllowed whether holes are allowed in the result
    * @return the alpha shape polygon
    */
    static std::unique_ptr<Geometry> alphaShape(
        const Geometry* geom,
        double alpha,
        bool isHolesAllowed);

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
    * Sets the alpha parameter to compute an alpha shape of the input.
    * Alpha is the radius of the eroding disc.
    * Border triangles with circumradius greater than alpha are removed.
    *
    * @param newAlpha the alpha radius
    */
    void setAlpha(double newAlpha);

    /**
    * Gets the computed concave hull.
    *
    * @return the concave hull
    */
    std::unique_ptr<Geometry> getHull();


private:

    // Constants
    static constexpr int PARAM_EDGE_LENGTH = 1;
    static constexpr int PARAM_ALPHA = 2;

    // Members
    const Geometry* inputGeometry;
    double maxEdgeLengthRatio;
    double alpha;
    bool isHolesAllowed;
    int criteriaType;
    double maxSizeInHull;
    const GeometryFactory* geomFactory;

    // Methods
    static double computeTargetEdgeLength(
        TriList<HullTri>& triList,
        double edgeLengthFactor);

    void computeHull(TriList<HullTri>& triList);
    void computeHullBorder(TriList<HullTri>& triList);
    void createBorderQueue(HullTriQueue& queue, TriList<HullTri>& triList);

    /**
    * Adds a Tri to the queue.
    * Only add tris with a single border edge.
    * since otherwise that would risk isolating a vertex if
    * the tri ends up being eroded from the hull.
    * Sets the tri size according to the threshold parameter being used.
    *
    * @param tri the Tri to add
    * @param queue the priority queue
    */
    void addBorderTri(HullTri* tri, HullTriQueue& queue);
    void computeHullHoles(TriList<HullTri>& triList);
    void setSize(HullTri* tri);


    /**
    * Finds tris which may be the start of holes.
    * Only tris which have a long enough edge and which do not touch the current hull
    * boundary are included.
    * This avoids the risk of disconnecting the result polygon.
    * The list is sorted in decreasing order of edge length.
    * The list is sorted in decreasing order of size.
    *
    * @param triList
    * @param maxSizeInHull maximum tri size which is not in a hole
    * @return
    */
    static std::vector<HullTri*> findCandidateHoles(
        TriList<HullTri>& triList, double maxSizeInHull);

    void removeHole(TriList<HullTri>& triList, HullTri* triHole);
    void setSize(TriList<HullTri>& triList);

    /**
    * Tests if a tri is included in the hull.
    * Tris with size less than the maximum are included in the hull.
    *
    * @param tri the tri to test
    * @return true if the tri is included in the hull
    */
    bool isInHull(const HullTri* tri) const;

    bool isRemovableBorder(const HullTri* tri) const;
    bool isRemovableHole(const HullTri* tri) const;

    std::unique_ptr<Geometry> toGeometry(
        TriList<HullTri>& triList,
        const GeometryFactory* factory);


};


} // geos::algorithm::hull
} // geos::algorithm
} // geos

