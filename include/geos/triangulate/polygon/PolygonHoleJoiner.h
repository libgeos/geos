/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2023 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/noding/SegmentIntersector.h>
#include <geos/noding/BasicSegmentString.h>
#include <geos/noding/SegmentSetMutualIntersector.h>
#include <geos/constants.h>

#include <set>
#include <limits>

// Forward declarations
namespace geos {
namespace geom {
class Envelope;
class Geometry;
class LinearRing;
}
namespace noding {
}
}

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::Polygon;
using geos::geom::LinearRing;
using geos::noding::BasicSegmentString;
using geos::noding::SegmentSetMutualIntersector;


namespace geos {
namespace triangulate {
namespace polygon {



/**
 * Transforms a polygon with holes into a single self-touching (invalid) ring
 * by joining holes to the exterior shell or to another hole
 * with out-and-back line segments.
 * The holes are added in order of their envelopes (leftmost/lowest first).
 * As the result shell develops, a hole may be added to what was
 * originally another hole.
 *
 * There is no attempt to optimize the quality of the join lines.
 * In particular, holes may be joined by lines longer than is optimal.
 * However, holes which touch the shell or other holes are joined at the touch point.
 *
 * The class does not require the input polygon to have normal
 * orientation (shell CW and rings CCW).
 * The output ring is always CW.
 */
class GEOS_DLL PolygonHoleJoiner {

private:

    // Members

    const Polygon* inputPolygon;

    //-- normalized, sorted and noded polygon rings
    std::unique_ptr<CoordinateSequence> shellRing;
    std::vector<std::unique_ptr<CoordinateSequence>> holeRings;

    //-- indicates whether a hole should be testing for touching
    std::vector<bool> isHoleTouchingHint;

    CoordinateSequence joinedRing;

    // a sorted and searchable version of the joinedRing
    std::set<Coordinate> joinedPts;

    std::unique_ptr<SegmentSetMutualIntersector> boundaryIntersector;

    // holding place for some BasicSegmentStrings
    std::vector<std::unique_ptr<BasicSegmentString>> polySegStringStore;


    // Classes
    class InteriorIntersectionDetector;
    friend class PolygonHoleJoiner::InteriorIntersectionDetector;


    void extractOrientedRings(const Polygon* polygon);
    static std::unique_ptr<CoordinateSequence> extractOrientedRing(const LinearRing* ring, bool isCW);
    void nodeRings();
    void joinHoles();

    void joinHole(std::size_t index, const CoordinateSequence& holeCoords);

    /**
    * Joins a hole to the shell only if the hole touches the shell.
    * Otherwise, reports the hole is non-touching.
    *
    * @param holeCoords the hole to join
    * @return true if the hole was touching, false if not
    */
    bool joinTouchingHole(const CoordinateSequence& holeCoords);

    /**
    * Finds the vertex index of a hole where it touches the
    * current shell (if it does).
    * If a hole does touch, it must touch at a single vertex
    * (otherwise, the polygon is invalid).
    *
    * @param holeCoords the hole
    * @return the index of the touching vertex, or -1 if no touch
    */
    std::size_t findHoleTouchIndex(const CoordinateSequence& holeCoords);

    /**
    * Joins a single non-touching hole to the current joined ring.
    *
    * @param holeCoords the hole to join
    */
    void joinNonTouchingHole(
        const CoordinateSequence& holeCoords);

    const Coordinate& findJoinableVertex(
        const Coordinate& holeJoinCoord);

    /**
    * Gets the join ring vertex index that the hole is joined after.
    * A vertex can occur multiple times in the join ring, so it is necessary
    * to choose the one which forms a corner having the
    * join line in the ring interior.
    *
    * @param joinCoord the join ring vertex
    * @param holeJoinCoord the hole join vertex
    * @return the join ring vertex index to join after
    */
    std::size_t findJoinIndex(
        const Coordinate& joinCoord,
        const Coordinate& holeJoinCoord);

    /**
    * Tests if a line between a ring corner vertex and a given point
    * is interior to the ring corner.
    *
    * @param ring a ring of points
    * @param ringIndex the index of a ring vertex
    * @param linePt the point to be joined to the ring
    * @return true if the line to the point is interior to the ring corner
    */
    static bool isLineInterior(
        const CoordinateSequence& ring,
        std::size_t ringIndex,
        const Coordinate& linePt);

    static std::size_t prev(std::size_t i, std::size_t size);
    static std::size_t next(std::size_t i, std::size_t size);

    /**
    * Add hole vertices at proper position in shell vertex list.
    * This code assumes that if hole touches (shell or other hole),
    * it touches at a node.  This requires an initial noding step.
    * In this case, the code avoids duplicating join vertices.
    *
    * Also adds hole points to ordered coordinates.
    *
    * @param joinIndex index of join vertex in shell
    * @param holeCoords the vertices of the hole to be inserted
    * @param holeJoinIndex index of join vertex in hole
    */
    void addJoinedHole(
        std::size_t joinIndex,
        const CoordinateSequence& holeCoords,
        std::size_t holeJoinIndex);

    /**
    * Creates the new section of vertices for ad added hole,
    * including any required vertices from the shell at the join point,
    * and ensuring join vertices are not duplicated.
    *
    * @param holeCoords the hole vertices
    * @param holeJoinIndex the index of the join vertex
    * @param joinPt the shell join vertex
    * @return a list of new vertices to be added
    */
    std::vector<Coordinate> createHoleSection(
        const CoordinateSequence& holeCoords,
        std::size_t holeJoinIndex,
        const Coordinate& joinPt);

    /**
    * Sort the hole rings by minimum X, minimum Y.
    *
    * @param poly polygon that contains the holes
    * @return a list of sorted hole rings
    */
    static std::vector<const LinearRing*> sortHoles(
        const Polygon* poly);

    static std::size_t findLowestLeftVertexIndex(
        const CoordinateSequence& holeCoords);

    /**
    * Tests whether the interior of a line segment intersects the polygon boundary.
    * If so, the line is not a valid join line.
    *
    * @param p0 a segment vertex
    * @param p1 the other segment vertex
    * @return true if the segment interior intersects a polygon boundary segment
    */
    bool intersectsBoundary(
        const Coordinate& p0,
        const Coordinate& p1);

    std::unique_ptr<SegmentSetMutualIntersector> createBoundaryIntersector();


public:

    PolygonHoleJoiner(const Polygon* p_inputPolygon)
        : inputPolygon(p_inputPolygon)
        , boundaryIntersector(nullptr)
        {};

    /**
    * Joins the shell and holes of a polygon
    * and returns the result as an (invalid) Polygon.
    *
    * @param p_inputPolygon the polygon to join
    * @return the result polygon
    */
    static std::unique_ptr<Polygon> joinAsPolygon(
        const Polygon* p_inputPolygon);

    /**
    * Joins the shell and holes of a polygon
    * and returns the result as sequence of Coordinates.
    *
    * @param p_inputPolygon the polygon to join
    * @return the result coordinates
    */
    static std::unique_ptr<CoordinateSequence> join(
        const Polygon* p_inputPolygon);

    /**
    * Computes the joined ring.
    *
    * @return the points in the joined ring
    */
    std::unique_ptr<CoordinateSequence> compute();

};


} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos
