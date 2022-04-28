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

#include <geos/geom/Coordinate.h>
#include <geos/noding/SegmentSetMutualIntersector.h>

#include <unordered_map>
#include <vector>

// Forward declarations
namespace geos {
namespace geom {
class Envelope;
class Geometry;
class CoordinateSequence;
class LinearRing;
}
namespace noding {
class SegmentString;
}
}

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::Polygon;
using geos::geom::LinearRing;


namespace geos {
namespace triangulate {
namespace polygon {



/**
 * Transforms a polygon with holes into a single self-touching (invalid) ring
 * by connecting holes to the exterior shell or to another hole.
 * The holes are added from the lowest upwards.
 * As the resulting shell develops, a hole might be added to what was
 * originally another hole.
 *
 * There is no attempt to optimize the quality of the join lines.
 * In particular, a hole which already touches at a vertex may be
 * joined at a different vertex.
 */
class GEOS_DLL PolygonHoleJoiner {

private:

    // Members

    static constexpr double EPS = 1.0E-4;

    std::vector<Coordinate> shellCoords;

    // orderedCoords is a copy of shellCoords for sort purposes
    std::set<Coordinate> shellCoordsSorted;

    // Key: starting end of the cut; Value: list of the other end of the cut
    std::unordered_map<Coordinate, std::vector<Coordinate>, Coordinate::HashCode> cutMap;

    std::unique_ptr<noding::SegmentSetMutualIntersector> polygonIntersector;
    const Polygon* inputPolygon;

    // The segstrings allocated in createPolygonIntersector need a
    // place to hold ownership through the lifecycle of the hole joiner
    std::vector<std::unique_ptr<noding::SegmentString>> polySegStringStore;

    // Methods

    static std::vector<Coordinate> ringCoordinates(const LinearRing* ring);

    void joinHoles();

    /**
    * Joins a single hole to the current shellRing.
    *
    * @param hole the hole to join
    */
    void joinHole(const LinearRing* hole);

    /**
    * Get the ith shellvertex in shellCoords[] that the current should add after
    *
    * @param shellVertex Coordinate of the shell vertex
    * @param holeVertex  Coordinate of the hole vertex
    * @return the ith shellvertex
    */
    std::size_t getShellCoordIndex(const Coordinate& shellVertex, const Coordinate& holeVertex);

    /**
    * Find the index of the coordinate in ShellCoords ArrayList,
    * skipping over some number of matches
    *
    * @param coord
    * @return
    */
    std::size_t getShellCoordIndexSkip(const Coordinate& coord, std::size_t numSkip);

    /**
    * Gets a list of shell vertices that could be used to join with the hole.
    * This list contains only one item if the chosen vertex does not share the same
    * x value with holeCoord
    *
    * @param holeCoord the hole coordinates
    * @return a list of candidate join vertices
    */
    std::vector<Coordinate> findLeftShellVertices(const Coordinate& holeCoord);

    /**
    * Determine if a line segment between a hole vertex
    * and a shell vertex lies inside the input polygon.
    *
    * @param holeCoord a hole coordinate
    * @param shellCoord a shell coordinate
    * @return true if the line lies inside the polygon
    */
    bool isJoinable(const Coordinate& holeCoord, const Coordinate& shellCoord) const;

    /**
    * Tests whether a line segment crosses the polygon boundary.
    *
    * @param p0 a vertex
    * @param p1 a vertex
    * @return true if the line segment crosses the polygon boundary
    */
    bool crossesPolygon(const Coordinate& p0, const Coordinate& p1) const;

    /**
    * Add hole at proper position in shell coordinate list.
    * Also adds hole points to ordered coordinates.
    *
    * @param shellVertexIndex
    * @param holeCoords
    * @param holeVertexIndex
    */
    void addHoleToShell(std::size_t shellVertexIndex, const CoordinateSequence* holeCoords, std::size_t holeVertexIndex);

    /**
    * Sort the holes by minimum X, minimum Y.
    *
    * @param poly polygon that contains the holes
    * @return a list of ordered hole geometry
    */
    static std::vector<const LinearRing*> sortHoles(const Polygon* poly);

    /**
    * Gets a list of indices of the leftmost vertices in a ring.
    *
    * @param geom the hole ring
    * @return index of the left most vertex
    */
    static std::vector<std::size_t> findLeftVertices(const LinearRing* ring);

    std::unique_ptr<noding::SegmentSetMutualIntersector> createPolygonIntersector(const Polygon* polygon);


public:

    PolygonHoleJoiner(const Polygon* p_inputPolygon);

    static std::vector<Coordinate> join(const Polygon* inputPolygon);
    static std::unique_ptr<Polygon> joinAsPolygon(const Polygon* inputPolygon);

    /**
    * Computes the joined ring.
    *
    * @return the points in the joined ring
    */
    std::vector<Coordinate> compute();


};



} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos
