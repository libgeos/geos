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

#include <geos/algorithm/hull/HullTri.h>
#include <geos/triangulate/tri/TriList.h>
#include <geos/triangulate/quadedge/TriangleVisitor.h>


namespace geos {
namespace geom {
class Geometry;
class GeometryFactory;
class Coordinate;
}
namespace triangulate {
namespace quadedge {
class QuadEdge;
class QuadEdgeSubdivision;
}
}
}

namespace geos {
namespace algorithm { // geos::algorithm
namespace hull {      // geos::algorithm::hull


class HullTriangulation
{
    using Geometry = geos::geom::Geometry;
    using GeometryFactory = geos::geom::GeometryFactory;
    using Coordinate = geos::geom::Coordinate;
    template<typename TriType>
    using TriList = geos::triangulate::tri::TriList<TriType>;
    using QuadEdge = geos::triangulate::quadedge::QuadEdge;
    using QuadEdgeSubdivision = geos::triangulate::quadedge::QuadEdgeSubdivision;
    using TriangleVisitor = geos::triangulate::quadedge::TriangleVisitor;

private:

    static void toTris(
        QuadEdgeSubdivision& subdiv,
        TriList<HullTri>& triList);

    /**
    * Extracts the coordinates of the edges along the boundary of a triangulation,
    * by tracing CW around the border triangles.
    * Assumption: there are at least 2 tris, they are connected,
    * and there are no holes.
    * So each tri has at least one non-boundary edge, and there is only one boundary.
    *
    * @param triList the triangulation
    * @return the points in the boundary of the triangulation
    */
    static geom::CoordinateSequence traceBoundary(
        TriList<HullTri>& triList);

    static HullTri* findBorderTri(
        TriList<HullTri>& triList);



public:

    HullTriangulation() {};

    static void createDelaunayTriangulation(
        const Geometry* geom,
        TriList<HullTri>& triList);

    /**
    * Creates a Polygon representing the area of a triangulation
    * which is connected and contains no holes.
    *
    * @param triList the triangulation
    * @param geomFactory the geometry factory to use
    * @return the area polygon
    */
    static std::unique_ptr<Geometry> traceBoundaryPolygon(
        TriList<HullTri>& triList,
        const GeometryFactory* factory);

    static HullTri* nextBorderTri(HullTri* triStart);

    /**
    * Creates a polygonal geometry representing the area of a triangulation
    * which may be disconnected or contain holes.
    *
    * @param triList the triangulation
    * @param geomFactory the geometry factory to use
    * @return the area polygonal geometry
    */
    static std::unique_ptr<Geometry> geomunion(
        TriList<HullTri>& triList,
        const GeometryFactory* factory);


    class HullTriVisitor : public TriangleVisitor
    {

        private:

            TriList<HullTri>& triList;

        public:

            HullTriVisitor(TriList<HullTri>& p_triList)
                : triList(p_triList)
                {};

            void visit(std::array<QuadEdge*, 3>& triEdges) override;

    }; // HullTriVisitor


}; // HullTriangulation



} // geos::algorithm::hull
} // geos::algorithm
} // geos

