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

#include <geos/algorithm/hull/ConcaveHullOfPolygons.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Polygon.h>
#include <geos/operation/overlayng/CoverageUnion.h>
#include <geos/triangulate/polygon/ConstrainedDelaunayTriangulator.h>
#include <geos/triangulate/tri/Tri.h>
#include <geos/util/IllegalArgumentException.h>


using geos::geom::Coordinate;
using geos::geom::Geometry;
using geos::geom::Envelope;
using geos::geom::GeometryCollection;
using geos::geom::GeometryFactory;
using geos::geom::LinearRing;
using geos::geom::MultiPolygon;
using geos::geom::Polygon;
using geos::operation::overlayng::CoverageUnion;
using geos::triangulate::polygon::ConstrainedDelaunayTriangulator;
using geos::triangulate::tri::Tri;


namespace geos {
namespace algorithm { // geos.algorithm
namespace hull {      // geos.algorithm.hulll

/* public static */
std::unique_ptr<Geometry>
ConcaveHullOfPolygons::concaveHullByLength(const Geometry* polygons, double maxLength)
{
    return concaveHullByLength(polygons, maxLength, false, false);
}

/* public static */
std::unique_ptr<Geometry>
ConcaveHullOfPolygons::concaveHullByLength(
    const Geometry* polygons, double maxLength,
    bool isTight, bool isHolesAllowed)
{
    ConcaveHullOfPolygons hull(polygons);
    hull.setMaximumEdgeLength(maxLength);
    hull.setHolesAllowed(isHolesAllowed);
    hull.setTight(isTight);
    return hull.getHull();
}

/* public static */
std::unique_ptr<Geometry>
ConcaveHullOfPolygons::concaveHullByLengthRatio(const Geometry* polygons, double lengthRatio)
{
    return concaveHullByLengthRatio(polygons, lengthRatio, false, false);
}

/* public static */
std::unique_ptr<Geometry>
ConcaveHullOfPolygons::concaveHullByLengthRatio(
    const Geometry* polygons, double lengthRatio,
    bool isTight, bool isHolesAllowed)
{
    ConcaveHullOfPolygons hull(polygons);
    hull.setMaximumEdgeLengthRatio(lengthRatio);
    hull.setHolesAllowed(isHolesAllowed);
    hull.setTight(isTight);
    return hull.getHull();
}

/* public static */
std::unique_ptr<Geometry>
ConcaveHullOfPolygons::concaveFillByLength(const Geometry* polygons, double maxLength)
{
    ConcaveHullOfPolygons hull(polygons);
    hull.setMaximumEdgeLength(maxLength);
    return hull.getFill();
}

/* public static */
std::unique_ptr<Geometry>
ConcaveHullOfPolygons::concaveFillByLengthRatio(const Geometry* polygons, double lengthRatio)
{
    ConcaveHullOfPolygons hull(polygons);
    hull.setMaximumEdgeLengthRatio(lengthRatio);
    return hull.getFill();
}

/* public */
ConcaveHullOfPolygons::ConcaveHullOfPolygons(const Geometry* geom)
    : inputPolygons(geom)
    , geomFactory(geom->getFactory())
    , maxEdgeLength(-1.0)
    , maxEdgeLengthRatio(-1.0)
    , isHolesAllowed(false)
    , isTight(false)
{
    if (! geom->isPolygonal()) {
        throw util::IllegalArgumentException("Input must be polygonal");
    }
}

/* public */
void
ConcaveHullOfPolygons::setMaximumEdgeLength(double edgeLength)
{
    if (edgeLength < 0)
        throw util::IllegalArgumentException("Edge length must be non-negative");
    maxEdgeLength = edgeLength;
    maxEdgeLengthRatio = -1;
}

/* public */
void
ConcaveHullOfPolygons::setMaximumEdgeLengthRatio(double edgeLengthRatio)
{
    if (edgeLengthRatio < 0 || edgeLengthRatio > 1)
        throw util::IllegalArgumentException("Edge length ratio must be in range [0,1]");
    maxEdgeLengthRatio = edgeLengthRatio;
}

/* public */
void
ConcaveHullOfPolygons::setHolesAllowed(bool p_isHolesAllowed)
{
    isHolesAllowed = p_isHolesAllowed;
}

/* public */
void
ConcaveHullOfPolygons::setTight(bool p_isTight)
{
    isTight = p_isTight;
}

/* public */
std::unique_ptr<Geometry>
ConcaveHullOfPolygons::getHull()
{
    if (inputPolygons->isEmpty()) {
        return createEmptyHull();
    }
    buildHullTris();
    return createHullGeometry(true);
}

/* public */
std::unique_ptr<Geometry>
ConcaveHullOfPolygons::getFill()
{
    isTight = true;
    if (inputPolygons->isEmpty()) {
        return createEmptyHull();
    }
    buildHullTris();
    return createHullGeometry(false);
}

/* private */
std::unique_ptr<Geometry>
ConcaveHullOfPolygons::createEmptyHull()
{
    return geomFactory->createPolygon();
}

/* private static */
void
ConcaveHullOfPolygons::extractShellRings(const Geometry* polygons, std::vector<const LinearRing*>& rings)
{
    rings.clear();
    for (std::size_t i = 0; i < polygons->getNumGeometries(); i++) {
        const Geometry* consGeom = polygons->getGeometryN(i);
        const Polygon* consPoly = static_cast<const Polygon*>(consGeom);
        const LinearRing* lr = consPoly->getExteriorRing();
        rings.push_back(lr);
    }
    return;
}


/* private */
void
ConcaveHullOfPolygons::buildHullTris()
{
    extractShellRings(inputPolygons, polygonRings);
    std::unique_ptr<Polygon> frame = createFrame(inputPolygons->getEnvelopeInternal());
    ConstrainedDelaunayTriangulator::triangulatePolygon(frame.get(), triList);
    //System.out.println(tris);

    const CoordinateSequence* framePts = frame->getExteriorRing()->getCoordinatesRO();
    if (maxEdgeLengthRatio >= 0) {
        maxEdgeLength = computeTargetEdgeLength(triList, framePts, maxEdgeLengthRatio);
    }

    removeFrameCornerTris(triList, framePts);
    removeBorderTris();
    if (isHolesAllowed) removeHoleTris();
}

/* private static */
std::unique_ptr<Polygon>
ConcaveHullOfPolygons::createFrame(const Envelope* polygonsEnv)
{
    double diam = polygonsEnv->getDiameter();
    Envelope envFrame = *polygonsEnv;
    envFrame.expandBy(FRAME_EXPAND_FACTOR * diam);
    std::unique_ptr<Geometry> frameGeom = geomFactory->toGeometry(&envFrame);
    Polygon* framePoly = dynamic_cast<Polygon*>(frameGeom.get());
    if (!framePoly) return nullptr;
    const LinearRing* frameShell = framePoly->getExteriorRing();
    std::unique_ptr<LinearRing> shell = frameShell->clone();
    std::vector<std::unique_ptr<LinearRing>> holes;
    for (const LinearRing* lr : polygonRings) {
        holes.emplace_back(lr->clone().release());
    }
    return geomFactory->createPolygon(std::move(shell), std::move(holes));
}


/* private static */
double
ConcaveHullOfPolygons::computeTargetEdgeLength(
    TriList<Tri>& tris,
    const CoordinateSequence* frameCorners,
    double edgeLengthRatio) const
{
    if (edgeLengthRatio == 0) return 0.0;
    double maxEdgeLen = -1;
    double minEdgeLen = -1;
    for (Tri* tri : tris.getTris()) {
        //-- don't include frame triangles
        if (isFrameTri(tri, frameCorners))
            continue;

        for (TriIndex i = 0; i < 3; i++) {
            //-- constraint edges are not used to determine ratio
            if (! tri->hasAdjacent(i))
                continue;

            double len = tri->getLength(i);
            if (len > maxEdgeLen)
                maxEdgeLen = len;
            if (minEdgeLen < 0 || len < minEdgeLen)
                minEdgeLen = len;
        }
    }
    //-- if ratio = 1 ensure all edges are included
    if (edgeLengthRatio == 1)
        return 2 * maxEdgeLen;

    return edgeLengthRatio * (maxEdgeLen - minEdgeLen) + minEdgeLen;
}

/* private static */
bool
ConcaveHullOfPolygons::isFrameTri(
    const Tri* tri,
    const CoordinateSequence* frameCorners) const
{
    TriIndex index = vertexIndex(tri, frameCorners);
    bool bIsFrameTri = (index >= 0);
    return bIsFrameTri;
}

/* private */
void
ConcaveHullOfPolygons::removeFrameCornerTris(
    TriList<Tri>& tris,
    const CoordinateSequence* frameCorners)
{
    hullTris.clear();
    borderTriQue.clear();
    for (Tri* tri : tris.getTris()) {
        TriIndex index = vertexIndex(tri, frameCorners);
        bool bIsFrameTri = (index >= 0);
        if (bIsFrameTri) {
            /**
             * Frame tris are adjacent to at most one border tri,
             * which is opposite the frame corner vertex.
             * Or, the opposite tri may be another frame tri,
             * which is not added as a border tri.
             */
            TriIndex oppIndex = Tri::oppEdge(index);
            Tri* oppTri = tri->getAdjacent(oppIndex);
            bool bBorderTri = oppTri != nullptr && ! isFrameTri(oppTri, frameCorners);
            if (bBorderTri) {
                addBorderTri(tri, oppIndex);
            }
            tri->remove();
        }
        else {
            hullTris.insert(tri);
        }
    }
    return;
}

/* private static */
TriIndex
ConcaveHullOfPolygons::vertexIndex(
    const Tri* tri,
    const CoordinateSequence* pts) const
{
    for (std::size_t i = 0; i < pts->size(); ++i) {
        const Coordinate& p = pts->getAt(i);
        TriIndex index = tri->getIndex(p);
        if (index >= 0)
            return index;
    }
    return -1;
}

/* private */
void
ConcaveHullOfPolygons::removeBorderTris()
{
    while (! borderTriQue.empty()) {
        Tri* tri = borderTriQue.back();
        borderTriQue.pop_back();
        //-- tri might have been removed already
        if (hullTris.find(tri) == hullTris.end()) {
            continue;
        }
        if (isRemovable(tri)) {
            addBorderTris(tri);
            removeBorderTri(tri);
        }
    }
}

/* private */
void
ConcaveHullOfPolygons::removeHoleTris()
{
    while (true) {
        Tri* holeTri = findHoleSeedTri();
        if (holeTri == nullptr)
            return;
        addBorderTris(holeTri);
        removeBorderTri(holeTri);
        removeBorderTris();
    }
}

/* private */
Tri*
ConcaveHullOfPolygons::findHoleSeedTri() const
{
    for (Tri* tri : hullTris) {
        if (isHoleSeedTri(tri))
            return tri;
    }
    return nullptr;
}

/* private */
bool
ConcaveHullOfPolygons::isHoleSeedTri(const Tri* tri) const
{
    if (isBorderTri(tri))
      return false;
    for (TriIndex i = 0; i < 3; i++) {
        if (tri->hasAdjacent(i)
            && tri->getLength(i) > maxEdgeLength)
            return true;
    }
    return false;
}

/* private */
bool
ConcaveHullOfPolygons::isBorderTri(const Tri* tri) const
{
    for (TriIndex i = 0; i < 3; i++) {
        if (! tri->hasAdjacent(i))
            return true;
    }
    return false;
}

/* private */
bool
ConcaveHullOfPolygons::isRemovable(const Tri* tri) const
{
    //-- remove non-bridging tris if keeping hull boundary tight
    if (isTight && isTouchingSinglePolygon(tri))
        return true;

    //-- check if outside edge is longer than threshold
    auto search = borderEdgeMap.find(const_cast<Tri*>(tri));
    if (search != borderEdgeMap.end()) {
        TriIndex borderEdgeIndex = search->second;
        double edgeLen = tri->getLength(borderEdgeIndex);
        if (edgeLen > maxEdgeLength)
            return true;
    }
    return false;
}

/* private */
bool
ConcaveHullOfPolygons::isTouchingSinglePolygon(const Tri* tri) const
{
    Envelope envTri;
    envelope(tri, envTri);
    for (const LinearRing* ring : polygonRings) {
        //-- optimization heuristic: a touching tri must be in ring envelope
        if (ring->getEnvelopeInternal()->intersects(envTri)) {
            if (hasAllVertices(ring, tri))
                return true;
        }
    }
    return false;
}

/* private */
void
ConcaveHullOfPolygons::addBorderTris(Tri* tri)
{
    addBorderTri(tri, 0);
    addBorderTri(tri, 1);
    addBorderTri(tri, 2);
}

/* private */
void
ConcaveHullOfPolygons::addBorderTri(Tri* tri, TriIndex index)
{
    Tri* adj = tri->getAdjacent(index);
    if (!adj)
        return;
    borderTriQue.push_back(adj);
    TriIndex borderEdgeIndex = adj->getIndex(tri);
    borderEdgeMap.insert({adj, borderEdgeIndex});
}

/* private */
void
ConcaveHullOfPolygons::removeBorderTri(Tri* tri)
{
    tri->remove();
    hullTris.erase(tri);
    borderEdgeMap.erase(tri);
}

/* private */
bool
ConcaveHullOfPolygons::hasAllVertices(const LinearRing* ring, const Tri* tri) const
{
    for (TriIndex i = 0; i < 3; i++) {
        const Coordinate& v = tri->getCoordinate(i);
        if (! hasVertex(ring, v)) {
            return false;
        }
    }
    return true;
}

/* private */
bool
ConcaveHullOfPolygons::hasVertex(const LinearRing* ring, const Coordinate& v) const
{
    for(std::size_t i = 1; i < ring->getNumPoints(); i++) {
        if (v.equals2D(ring->getCoordinateN(i))) {
            return true;
        }
    }
    return false;
}

/* private */
void
ConcaveHullOfPolygons::envelope(const Tri* tri, Envelope& env) const
{
    env.init(tri->getCoordinate(0), tri->getCoordinate(1));
    env.expandToInclude(tri->getCoordinate(2));
    return;
}

/* private */
std::unique_ptr<Geometry>
ConcaveHullOfPolygons::createHullGeometry(bool isIncludeInput)
{
    if (! isIncludeInput && hullTris.empty())
        return createEmptyHull();

    //-- union triangulation
    std::unique_ptr<Geometry> triCoverage = Tri::toGeometry(hullTris, geomFactory);
    //System.out.println(triCoverage);
    std::unique_ptr<Geometry> fillGeometry = CoverageUnion::geomunion(triCoverage.get());

    if (! isIncludeInput) {
        return fillGeometry;
    }
    if (fillGeometry->isEmpty()) {
        return inputPolygons->clone();
    }
    //-- union with input polygons
    std::vector<std::unique_ptr<Geometry>> geoms;
    geoms.emplace_back(fillGeometry.release());
    geoms.emplace_back(inputPolygons->clone().release());

    std::unique_ptr<GeometryCollection> geomColl = geomFactory->createGeometryCollection(std::move(geoms));
    std::unique_ptr<Geometry> hull = CoverageUnion::geomunion(geomColl.get());
    return hull;
}



} // namespace geos.algorithm.hull
} // namespace geos.algorithm
} // namespace geos
