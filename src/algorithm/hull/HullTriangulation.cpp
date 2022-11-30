/**********************************************************************
 *
 * GEOS - const Geometry* Engine Open Source
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

#include <geos/algorithm/hull/HullTriangulation.h>
#include <geos/algorithm/hull/HullTri.h>

#include <geos/triangulate/DelaunayTriangulationBuilder.h>
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
#include <geos/triangulate/quadedge/QuadEdge.h>
#include <geos/triangulate/tri/TriangulationBuilder.h>
#include <geos/operation/overlayng/CoverageUnion.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateList.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Triangle.h>
#include <geos/util/IllegalStateException.h>
#include <geos/util/Assert.h>


using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateList;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::Polygon;
using geos::geom::Triangle;
using geos::triangulate::DelaunayTriangulationBuilder;
using geos::triangulate::tri::TriangulationBuilder;
using geos::operation::overlayng::CoverageUnion;

namespace geos {
namespace algorithm { // geos.algorithm
namespace hull {      // geos.algorithm.hulll


/* public static */
void
HullTriangulation::createDelaunayTriangulation(
    const Geometry* geom,
    TriList<HullTri>& triList)
{
    //TODO: implement a DT on Tris directly?
    DelaunayTriangulationBuilder dt;
    dt.setSites(*geom);
    QuadEdgeSubdivision& subdiv = dt.getSubdivision();
    toTris(subdiv, triList);
    return;
}


/* private static */
void
HullTriangulation::toTris(
    QuadEdgeSubdivision& subdiv,
    TriList<HullTri>& triList)
{
    HullTriVisitor visitor(triList);
    subdiv.visitTriangles(&visitor, false);
    TriangulationBuilder tb;
    for (auto* tri : triList) {
        tb.add(static_cast<Tri*>(tri));
    }
    return;
}


/* private static */
std::unique_ptr<Geometry>
HullTriangulation::geomunion(
    TriList<HullTri>& triList,
    const GeometryFactory* factory)
{
    std::vector<std::unique_ptr<Polygon>> polys;

    for (auto* tri : triList) {
        std::unique_ptr<Polygon> poly = tri->toPolygon(factory);
        polys.emplace_back(poly.release());
    }
    std::unique_ptr<Geometry> geom = factory->buildGeometry(std::move(polys));
    return CoverageUnion::geomunion(geom.get());
}


/* public static */
std::unique_ptr<Geometry>
HullTriangulation::traceBoundaryPolygon(
    TriList<HullTri>& triList,
    const GeometryFactory* factory)
{
    if (triList.size() == 1) {
        HullTri* tri = triList[0];
        return tri->toPolygon(factory);
    }
    auto&& pts = traceBoundary(triList);
    return factory->createPolygon(std::move(pts));
}


/* private static */
CoordinateSequence
HullTriangulation::traceBoundary(TriList<HullTri>& triList)
{
    HullTri* triStart = findBorderTri(triList);

    CoordinateSequence coordList;
    HullTri* tri = triStart;
    do {
        TriIndex borderIndex = tri->boundaryIndexCCW();
        //-- add border vertex
        coordList.add(tri->getCoordinate(borderIndex), false);
        TriIndex nextIndex = Tri::next(borderIndex);
        //-- if next edge is also border, add it and move to next
        if (tri->isBoundary(nextIndex)) {
            coordList.add(tri->getCoordinate(nextIndex), false);
            borderIndex = nextIndex;
        }
        //-- find next border tri CCW around non-border edge
        tri = nextBorderTri(tri);
    } while (tri != triStart);
    coordList.closeRing();
    return coordList;
}


/* private static */
HullTri*
HullTriangulation::findBorderTri(TriList<HullTri>& triList)
{
    for (auto* tri : triList) {
        if (tri->isBorder()) return tri;
    }
    util::Assert::shouldNeverReachHere("No border triangles found");
    return nullptr;
}


/* public static */
HullTri*
HullTriangulation::nextBorderTri(HullTri* triStart)
{
    HullTri* tri = triStart;
    // start at first non-border edge CW
    TriIndex index = Tri::next(tri->boundaryIndexCW());
    // scan CCW around vertex for next border tri
    do {
        HullTri* adjTri = static_cast<HullTri*>(tri->getAdjacent(index));
        if (adjTri == tri) {
            throw util::IllegalStateException("No outgoing border edge found");
        }
        index = Tri::next(adjTri->getIndex(tri));
        tri = adjTri;
    }
    while (! tri->isBoundary(index));
    return (tri);
}


/* HullTriVisitor ------------------------------------------------------------ */

void
HullTriangulation::HullTriVisitor::visit(
    std::array<QuadEdge*, 3>& triEdges)
{
    const Coordinate& p0 = triEdges[0]->orig().getCoordinate();
    const Coordinate& p1 = triEdges[1]->orig().getCoordinate();
    const Coordinate& p2 = triEdges[2]->orig().getCoordinate();
    if (Triangle::isCCW(p0, p1, p2)) {
        triList.add(p0, p2, p1);
    }
    else {
        triList.add(p0, p1, p2);
    }
}



} // namespace geos.algorithm.hull
} // namespace geos.algorithm
} // namespace geos

