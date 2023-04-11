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

#include <geos/index/VertexSequencePackedRtree.h>
#include <geos/triangulate/tri/TriList.h>
#include <geos/triangulate/tri/Tri.h>
#include <geos/constants.h>

#include <array>
#include <memory>
#include <limits>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class Polygon;
class Envelope;
}
}

using geos::geom::Coordinate;
using geos::geom::Polygon;
using geos::geom::Envelope;
using geos::triangulate::tri::Tri;
using geos::triangulate::tri::TriList;
using geos::index::VertexSequencePackedRtree;

namespace geos {
namespace triangulate {
namespace polygon {


/**
 * Triangulates a polygon using the Ear-Clipping technique.
 * The polygon is provided as a closed list of contiguous vertices
 * defining its boundary.
 * The vertices must have clockwise orientation.
 *
 * The polygon boundary must not self-cross,
 * but may self-touch at points or along an edge.
 * It may contain repeated points, which are treated as a single vertex.
 * By default every vertex is triangulated,
 * including ones which are "flat" (the adjacent segments are collinear).
 * These can be removed by setting setSkipFlatCorners(boolean)
 *
 * The polygon representation does not allow holes.
 * Polygons with holes can be triangulated by preparing them
 * with {@link PolygonHoleJoiner}.
 *
 * @author Martin Davis
 *
 */
class GEOS_DLL PolygonEarClipper {

private:

    // Members

    bool isFlatCornersSkipped = false;

    /**
    * The polygon vertices are provided in CW orientation.
    * Thus for convex interior angles
    * the vertices forming the angle are in CW orientation.
    */
    const CoordinateSequence& vertex;
    std::vector<std::size_t> vertexNext;
    std::size_t vertexSize;

    // first available vertex index
    std::size_t vertexFirst;

    // indices for current corner
    std::array<std::size_t, 3> cornerIndex;

    /**
    * Indexing vertices improves ear intersection testing performance a lot.
    * The polyShell vertices are contiguous, so are suitable for an SPRtree.
    */
    VertexSequencePackedRtree vertexCoordIndex;

    // Methods

    std::vector<std::size_t> createNextLinks(std::size_t size) const;

    bool isValidEar(std::size_t cornerIndex, const std::array<Coordinate, 3>& corner);

    /**
    * Finds another vertex intersecting the corner triangle, if any.
    * Uses the vertex spatial index for efficiency.
    *
    * Also finds any vertex which is a duplicate of the corner apex vertex,
    * which then requires a full scan of the vertices to confirm ear is valid.
    * This is usually a rare situation, so has little impact on performance.
    *
    * @param cornerIndex the index of the corner apex vertex
    * @param corner the corner vertices
    * @return the index of an intersecting or duplicate vertex, or NO_COORD_INDEX if none
    */
    std::size_t findIntersectingVertex(std::size_t cornerIndex, const std::array<Coordinate, 3>& corner) const;

    /**
    * Scan all vertices in current ring to check if any are duplicates
    * of the corner apex vertex, and if so whether the corner ear
    * intersects the adjacent segments and thus is invalid.
    *
    * @param cornerIndex the index of the corner apex
    * @param corner the corner vertices
    * @return true if the corner ia a valid ear
    */
    bool isValidEarScan(std::size_t cornerIndex, const std::array<Coordinate, 3>& corner) const;

    /* private  */
    static Envelope envelope(const std::array<Coordinate, 3>& corner);

    /**
    * Remove the corner apex vertex and update the candidate corner location.
    */
    void removeCorner();

    bool isRemoved(std::size_t vertexIndex) const;

    void initCornerIndex();

    /**
    * Fetch the corner vertices from the indices.
    *
    * @param corner an array for the corner vertices
    */
    void fetchCorner(std::array<Coordinate, 3>& cornerVertex) const;

    /**
    * Move to next corner.
    */
    void nextCorner(std::array<Coordinate, 3>& cornerVertex);

    /**
    * Get the index of the next available shell coordinate starting from the given
    * index.
    *
    * @param index candidate position
    * @return index of the next available shell coordinate
    */
    std::size_t nextIndex(std::size_t index) const;

    bool isConvex(const std::array<Coordinate, 3>& pts) const;

    bool isFlat(const std::array<Coordinate, 3>& pts) const;

    /**
    * Detects if a corner has repeated points (AAB or ABB), or is collapsed (ABA).
    * @param pts the corner points
    * @return true if the corner is flat or collapsed
    */
    bool isCornerInvalid(const std::array<Coordinate, 3>& pts) const;


public:

    /**
    * Creates a new ear-clipper instance.
    *
    * @param polyShell the polygon vertices to process
    */
    PolygonEarClipper(const geom::CoordinateSequence& polyShell);

    /**
    * Triangulates a polygon via ear-clipping.
    *
    * @param polyShell the vertices of the polygon
    * @param triListResult vector to fill in with the resultant Tri s
    */
    static void triangulate(const geom::CoordinateSequence& polyShell, TriList<Tri>& triListResult);

    /**
    * Sets whether flat corners formed by collinear adjacent line segments
    * are included in the triangulation.
    * Skipping flat corners reduces the number of triangles in the output.
    * However, it produces a triangulation which does not include
    * all input vertices.  This may be undesirable for downstream processes
    * (such as computing a Constrained Delaunay Triangulation for
    * purposes of computing the medial axis).
    *
    * The default is to include all vertices in the result triangulation.
    * This still produces a valid triangulation, with no zero-area triangles.
    *
    * Note that repeated vertices are always skipped.
    *
    * @param p_isFlatCornersSkipped whether to skip collinear vertices
    */
    void setSkipFlatCorners(bool p_isFlatCornersSkipped);

    void compute(TriList<Tri>& triList);

    std::unique_ptr<Polygon> toGeometry() const;


};



} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos
