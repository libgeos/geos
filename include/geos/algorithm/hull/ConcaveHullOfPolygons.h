/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/triangulate/tri/TriList.h>

#include <set>
#include <deque>
#include <map>

namespace geos {
namespace geom {
class Coordinate;
class CoordinateSequence;
class Envelope;
class Geometry;
class GeometryCollection;
class GeometryFactory;
class LinearRing;
class Polygon;
}
namespace triangulate {
namespace tri {
class Tri;
}
}
}

#include <geos/triangulate/tri/Tri.h>


using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryCollection;
using geos::geom::GeometryFactory;
using geos::geom::LinearRing;
using geos::geom::Polygon;
using geos::triangulate::tri::Tri;
using geos::triangulate::tri::TriList;


namespace geos {
namespace algorithm { // geos::algorithm
namespace hull {      // geos::algorithm::hull


/**
 * Constructs a concave hull of a set of polygons, respecting
 * the polygons as constraints.
 * A concave hull is a possibly non-convex polygon containing all the input polygons.
 * A given set of polygons has a sequence of hulls of increasing concaveness,
 * determined by a numeric target parameter.
 * The computed hull "fills the gap" between the polygons,
 * and does not intersect their interior.
 *
 * The concave hull is constructed by removing the longest outer edges
 * of the Delaunay Triangulation of the space between the polygons,
 * until the target criterion parameter is reached.
 *
 * The target criteria are:
 *  * Maximum Edge Length - the length of the longest edge between the polygons is no larger
 *    than this value.
 *  * Maximum Edge Length Ratio - determine the Maximum Edge Length
 *    as a fraction of the difference between the longest and shortest edge lengths
 *    between the polygons. This normalizes the Maximum Edge Length to be scale-free.
 *    A value of 1 produces the convex hull; a value of 0 produces the original polygons.
 *
 * The preferred criterion is the Maximum Edge Length Ratio, since it is
 * scale-free and local (so that no assumption needs to be made about the
 * total amount of concaveness present).
 *
 * Optionally the concave hull can be allowed to contain holes, via setHolesAllowed().
 *
 * The hull can be specified as being "tight", which means it follows the outer boundaries
 * of the input polygons.
 *
 * The input polygons must form a valid MultiPolygon
 * (i.e. they must be non-overlapping).
 *
 * \author Martin Davis
 *
 */
class GEOS_DLL ConcaveHullOfPolygons {

private:

    /* Members */

    static constexpr int FRAME_EXPAND_FACTOR = 4;

    const Geometry* inputPolygons;
    const GeometryFactory* geomFactory;
    double maxEdgeLength;
    double maxEdgeLengthRatio;
    bool isHolesAllowed;
    bool isTight;

    std::set<Tri*> hullTris;
    std::deque<Tri*> borderTriQue;
    std::vector<const LinearRing*> polygonRings;
    TriList<Tri> triList;

    /**
    * Records the edge index of the longest border edge for border tris,
    * so it can be tested for length and possible removal.
    */
    std::map<Tri*, TriIndex> borderEdgeMap;

    /* Methods */

    std::unique_ptr<Geometry> createEmptyHull();

    static void extractShellRings(
        const Geometry* polygons,
        std::vector<const LinearRing*>& rings);

    void buildHullTris();

    /**
    * Creates a rectangular "frame" around the input polygons,
    * with the input polygons as holes in it.
    * The frame is large enough that the constrained Delaunay triangulation
    * of it should contain the convex hull of the input as edges.
    * The frame corner triangles can be removed to produce a
    * triangulation of the space around and between the input polygons.
    *
    * @param polygonsEnv
    * @return the frame polygon
    */
    std::unique_ptr<Polygon> createFrame(
        const Envelope* polygonsEnv);

    double computeTargetEdgeLength(
        TriList<Tri>& triList,
        const CoordinateSequence* frameCorners,
        double edgeLengthRatio) const;

    bool isFrameTri(
        const Tri* tri,
        const CoordinateSequence* frameCorners) const;

    void removeFrameCornerTris(
        TriList<Tri>& tris,
        const CoordinateSequence* frameCorners);

    /**
    * Get the tri vertex index of some point in a list,
    * or -1 if none are vertices.
    *
    * @param tri the tri to test for containing a point
    * @param pts the points to test
    * @return the vertex index of a point, or -1
    */
    TriIndex vertexIndex(
        const Tri* tri,
        const CoordinateSequence* pts) const;

    void removeBorderTris();

    void removeHoleTris();

    Tri* findHoleSeedTri() const;

    bool isHoleSeedTri(const Tri* tri) const;

    bool isBorderTri(const Tri* tri) const;

    bool isRemovable(const Tri* tri) const;

    /**
    * Tests whether a triangle touches a single polygon at all vertices.
    * If so, it is a candidate for removal if the hull polygon
    * is being kept tight to the outer boundary of the input polygons.
    * Tris which touch more than one polygon are called "bridging".
    *
    * @param tri
    * @return true if the tri touches a single polygon
    */
    bool isTouchingSinglePolygon(const Tri* tri) const;

    void addBorderTris(Tri* tri);

    /**
    * Adds an adjacent tri to the current border.
    * The adjacent edge is recorded as the border edge for the tri.
    * Note that only edges adjacent to another tri can become border edges.
    * Since constraint-adjacent edges do not have an adjacent tri,
    * they can never be on the border and thus will not be removed
    * due to being shorter than the length threshold.
    * The tri containing them may still be removed via another edge, however.
    *
    * @param tri the tri adjacent to the tri to be added to the border
    * @param index the index of the adjacent tri
    */
    void addBorderTri(Tri* tri, TriIndex index);

    void removeBorderTri(Tri* tri);

    bool hasAllVertices(const LinearRing* ring, const Tri* tri) const;

    bool hasVertex(const LinearRing* ring, const Coordinate& v) const;

    void envelope(const Tri* tri, Envelope& env) const;

    std::unique_ptr<Geometry> createHullGeometry(bool isIncludeInput);


public:

    /**
    * Computes a concave hull of set of polygons
    * using the target criterion of maximum edge length.
    *
    * @param polygons the input polygons
    * @param maxLength the target maximum edge length
    * @return the concave hull
    */
    static std::unique_ptr<Geometry>
    concaveHullByLength(const Geometry* polygons, double maxLength);

    /**
    * Computes a concave hull of set of polygons
    * using the target criterion of maximum edge length,
    * and allowing control over whether the hull boundary is tight
    * and can contain holes.
    *
    * @param polygons the input polygons
    * @param maxLength the target maximum edge length
    * @param isTight true if the hull should be tight to the outside of the polygons
    * @param isHolesAllowed true if holes are allowed in the hull polygon
    * @return the concave hull
    */
    static std::unique_ptr<Geometry>
    concaveHullByLength(
        const Geometry* polygons, double maxLength,
        bool isTight, bool isHolesAllowed);

    /**
    * Computes a concave hull of set of polygons
    * using the target criterion of maximum edge length ratio.
    *
    * @param polygons the input polygons
    * @param lengthRatio the target maximum edge length ratio
    * @return the concave hull
    */
    static std::unique_ptr<Geometry>
    concaveHullByLengthRatio(const Geometry* polygons, double lengthRatio);

    /**
    * Computes a concave hull of set of polygons
    * using the target criterion of maximum edge length ratio,
    * and allowing control over whether the hull boundary is tight
    * and can contain holes.
    *
    * @param polygons the input polygons
    * @param lengthRatio the target maximum edge length ratio
    * @param isTight true if the hull should be tight to the outside of the polygons
    * @param isHolesAllowed true if holes are allowed in the hull polygon
    * @return the concave hull
    */
    static std::unique_ptr<Geometry>
    concaveHullByLengthRatio(
        const Geometry* polygons, double lengthRatio,
        bool isTight, bool isHolesAllowed);

    /**
    * Computes a concave fill area between a set of polygons,
    * using the target criterion of maximum edge length.
    *
    * @param polygons the input polygons
    * @param maxLength the target maximum edge length
    * @return the concave fill
    */
    static std::unique_ptr<Geometry>
    concaveFillByLength(const Geometry* polygons, double maxLength);

    /**
    * Computes a concave fill area between a set of polygons,
    * using the target criterion of maximum edge length ratio.
    *
    * @param polygons the input polygons
    * @param lengthRatio the target maximum edge length ratio
    * @return the concave fill
    */
    static std::unique_ptr<Geometry>
    concaveFillByLengthRatio(const Geometry* polygons, double lengthRatio);

    /**
    * Creates a new instance for a given geometry.
    *
    * @param geom the input geometry
    */
    ConcaveHullOfPolygons(const Geometry* geom);

    /**
    * Sets the target maximum edge length for the concave hull.
    * The length value must be zero or greater.
    *
    *   * The value 0.0 produces the input polygons.
    *   * Larger values produce less concave results.
    * Above a certain large value the result is the convex hull of the input.
    *
    * The edge length ratio provides a scale-free parameter which
    * is intended to produce similar concave results for a variety of inputs.
    *
    * @param edgeLength a non-negative length
    */
    void setMaximumEdgeLength(double edgeLength);

    /**
    * Sets the target maximum edge length ratio for the concave hull.
    * The edge length ratio is a fraction of the difference
    * between the longest and shortest edge lengths
    * in the Delaunay Triangulation of the area between the input polygons.
    * (Roughly speaking, it is a fraction of the difference between
    * the shortest and longest distances between the input polygons.)
    * It is a value in the range 0 to 1.
    *
    *   * The value 0.0 produces the original input polygons.
    *   * The value 1.0 produces the convex hull.
    *
    * @param edgeLengthRatio a length factor value between 0 and 1
    */
    void setMaximumEdgeLengthRatio(double edgeLengthRatio);

    /**
    * Sets whether holes are allowed in the concave hull polygon.
    *
    * @param p_isHolesAllowed true if holes are allowed in the result
    */
    void setHolesAllowed(bool p_isHolesAllowed);

    /**
    * Sets whether the boundary of the hull polygon is kept
    * tight to the outer edges of the input polygons.
    *
    * @param p_isTight true if the boundary is kept tight
    */
    void setTight(bool p_isTight);

    /**
    * Gets the computed concave hull.
    *
    * @return the concave hull
    */
    std::unique_ptr<Geometry> getHull();

    /**
    * Gets the concave fill, which is the area between the input polygons,
    * subject to the concaveness control parameter.
    *
    * @return the concave fill
    */
    std::unique_ptr<Geometry> getFill();


};



} // geos::algorithm::hull
} // geos::algorithm
} // geos
