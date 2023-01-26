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

#include <geos/algorithm/hull/ConcaveHull.h>
#include <geos/algorithm/hull/HullTri.h>
#include <geos/algorithm/hull/HullTriangulation.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/operation/overlayng/CoverageUnion.h>
#include <geos/triangulate/DelaunayTriangulationBuilder.h>
#include <geos/triangulate/quadedge/QuadEdge.h>
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
#include <geos/triangulate/tri/TriangulationBuilder.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/IllegalStateException.h>
#include <geos/util/Assert.h>


using geos::geom::Coordinate;
using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::Polygon;
using geos::operation::overlayng::CoverageUnion;
using geos::triangulate::DelaunayTriangulationBuilder;
using geos::triangulate::quadedge::QuadEdge;
using geos::triangulate::quadedge::QuadEdgeSubdivision;
using geos::triangulate::quadedge::TriangleVisitor;
using geos::triangulate::tri::Tri;
using geos::triangulate::tri::TriList;
using geos::triangulate::tri::TriangulationBuilder;


namespace geos {
namespace algorithm { // geos.algorithm
namespace hull {      // geos.algorithm.hulll


/* public static */
double
ConcaveHull::uniformEdgeLength(const Geometry* geom)
{
    double areaCH = geom->convexHull()->getArea();
    double numPts = static_cast<double>(geom->getNumPoints());
    return std::sqrt(areaCH / numPts);
}


/* public static */
std::unique_ptr<Geometry>
ConcaveHull::concaveHullByLength(const Geometry* geom, double maxLength)
{
    return concaveHullByLength(geom, maxLength, false);
}


/* public static */
std::unique_ptr<Geometry>
ConcaveHull::concaveHullByLength(
    const Geometry* geom,
    double maxLength,
    bool holesAllowed)
{
    ConcaveHull hull(geom);
    hull.setMaximumEdgeLength(maxLength);
    hull.setHolesAllowed(holesAllowed);
    return hull.getHull();
}


/* public static */
std::unique_ptr<Geometry>
ConcaveHull::concaveHullByLengthRatio(
    const Geometry* geom,
    double lengthFactor)
{
    return concaveHullByLengthRatio(geom, lengthFactor, false);
}

/* public static */
std::unique_ptr<Geometry>
ConcaveHull::concaveHullByLengthRatio(
    const Geometry* geom,
    double lengthFactor,
    bool holesAllowed)
{
    ConcaveHull hull(geom);
    hull.setMaximumEdgeLengthRatio(lengthFactor);
    hull.setHolesAllowed(holesAllowed);
    return hull.getHull();
}

/* public static */
std::unique_ptr<Geometry>
ConcaveHull::alphaShape(
    const Geometry* geom,
    double alpha,
    bool holesAllowed)
{
    ConcaveHull hull(geom);
    hull.setAlpha(alpha);
    hull.setHolesAllowed(holesAllowed);
    return hull.getHull();
}

/* public */
void
ConcaveHull::setMaximumEdgeLength(double edgeLength)
{
    if (edgeLength < 0)
        throw util::IllegalArgumentException("Edge length must be non-negative");
    maxSizeInHull = edgeLength;
    maxEdgeLengthRatio = -1.0;
    criteriaType = PARAM_EDGE_LENGTH;
}


/* public */
void
ConcaveHull::setMaximumEdgeLengthRatio(double edgeLengthRatio)
{
    if (edgeLengthRatio < 0 || edgeLengthRatio > 1)
        throw util::IllegalArgumentException("Edge length ratio must be in range [0,1]");
    maxEdgeLengthRatio = edgeLengthRatio;
    criteriaType = PARAM_EDGE_LENGTH;
}


/* public */
void
ConcaveHull::setHolesAllowed(bool holesAllowed)
{
    isHolesAllowed = holesAllowed;
}

/* public */
void
ConcaveHull::setAlpha(double newAlpha)
{
    alpha = newAlpha;
    maxSizeInHull = newAlpha;
    criteriaType = PARAM_ALPHA;
}

/* public */
std::unique_ptr<Geometry>
ConcaveHull::getHull()
{
    if (inputGeometry->isEmpty()) {
        return geomFactory->createPolygon();
    }
    TriList<HullTri> triList;
    HullTriangulation::createDelaunayTriangulation(inputGeometry, triList);
    setSize(triList);

    if (maxEdgeLengthRatio >= 0) {
        maxSizeInHull = computeTargetEdgeLength(triList, maxEdgeLengthRatio);
    }
    if (triList.empty()) {
        return inputGeometry->convexHull();
    }
    computeHull(triList);
    std::unique_ptr<Geometry> hull = toGeometry(triList, geomFactory);
    return hull;
}

/* private */
void
ConcaveHull::setSize(TriList<HullTri>& triList)
{
    for (auto* tri : triList) {
        if (criteriaType == PARAM_EDGE_LENGTH) {
            tri->setSizeToLongestEdge();
        }
        else {
            tri->setSizeToCircumradius();
        }
    }
}

/* private */
bool
ConcaveHull::isInHull(const HullTri* tri) const
{
    return tri->getSize() < maxSizeInHull;
}

/* private static */
double
ConcaveHull::computeTargetEdgeLength(
    TriList<HullTri>& triList,
    double edgeLengthRatio)
{
    if (edgeLengthRatio == 0) return 0;
    double maxEdgeLen = -1;
    double minEdgeLen = -1;
    for (auto* tri : triList) {
        for (TriIndex i = 0; i < 3; i++) {
            double len = tri->getCoordinate(i).distance(tri->getCoordinate(HullTri::next(i)));
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

/* private */
void
ConcaveHull::computeHull(TriList<HullTri>& triList)
{
    computeHullBorder(triList);
    if (isHolesAllowed) {
        computeHullHoles(triList);
    }
}


/* private */
void
ConcaveHull::computeHullBorder(TriList<HullTri>& triList)
{
    // Sort the HullTri from largest (at head) to smallest
    HullTriQueue queue;
    createBorderQueue(queue, triList);

    // process tris in order of decreasing size (edge length or circumradius)
    while (! queue.empty()) {

        HullTri* tri = queue.top();
        queue.pop();

        if (isInHull(tri))
            break;

        if (isRemovableBorder(tri)) {
            //-- the non-null adjacents are now on the border
            HullTri* adj0 = static_cast<HullTri*>(tri->getAdjacent(0));
            HullTri* adj1 = static_cast<HullTri*>(tri->getAdjacent(1));
            HullTri* adj2 = static_cast<HullTri*>(tri->getAdjacent(2));

            tri->remove(triList);

            //-- add border adjacents to queue
            addBorderTri(adj0, queue);
            addBorderTri(adj1, queue);
            addBorderTri(adj2, queue);
        }
    }
}


/* private */
void
ConcaveHull::createBorderQueue(HullTriQueue& queue, TriList<HullTri>& triList)
{
    for (auto* tri : triList) {
        addBorderTri(tri, queue);
    }
    return;
}


/* private */
void
ConcaveHull::addBorderTri(HullTri* tri, HullTriQueue& queue)
{
    if (tri == nullptr) return;
    if (tri->numAdjacent() != 2) return;
    setSize(tri);
    queue.push(tri);
}


/* private */
void
ConcaveHull::setSize(HullTri* tri)
{
    if (criteriaType == PARAM_EDGE_LENGTH)
        tri->setSizeToBoundary();
    else
        tri->setSizeToCircumradius();
}


/* private */
void
ConcaveHull::computeHullHoles(TriList<HullTri>& triList)
{
    std::vector<HullTri*> candidateHoles = findCandidateHoles(triList, maxSizeInHull);
    // remove tris in order of decreasing size (edge length)
    for (auto* tri : candidateHoles) {
        if (tri->isRemoved() ||
            tri->isBorder() ||
            tri->hasBoundaryTouch()) {
            continue;
        }
        removeHole(triList, tri);
    }
}

/* private static */
std::vector<HullTri*>
ConcaveHull::findCandidateHoles(TriList<HullTri>& triList, double maxSizeInHull)
{
    std::vector<HullTri*> candidates;
    for (auto* tri : triList) {
        //-- tris below the size threshold are in the hull, so NOT in a hole
        if (tri->getSize() < maxSizeInHull) continue;

        bool isTouchingBoundary = tri->isBorder() || tri->hasBoundaryTouch();
        if (! isTouchingBoundary) {
            candidates.push_back(tri);
        }
    }
    // sort by HullTri comparator - larger sizes first
    std::sort(candidates.begin(), candidates.end(), HullTri::HullTriCompare());
    return candidates;
}

/**
* Erodes a hole starting at a given triangle,
* and eroding all adjacent triangles with boundary edge length above target.
* @param triList the triangulation
* @param triHole triangle which is a hole
*/
/* private */
void
ConcaveHull::removeHole(TriList<HullTri>& triList, HullTri* triHole)
{
    HullTriQueue queue;
    queue.push(triHole);

    while (! queue.empty()) {

        HullTri* tri = queue.top();
        queue.pop();

        if (tri != triHole && isInHull(tri)) {
            break;
        }

        if (tri == triHole || isRemovableHole(tri)) {
            //-- the non-null adjacents are now on the border
            HullTri* adj0 = static_cast<HullTri*>(tri->getAdjacent(0));
            HullTri* adj1 = static_cast<HullTri*>(tri->getAdjacent(1));
            HullTri* adj2 = static_cast<HullTri*>(tri->getAdjacent(2));

            tri->remove(triList);

            //-- add border adjacents to queue
            addBorderTri(adj0, queue);
            addBorderTri(adj1, queue);
            addBorderTri(adj2, queue);
        }
    }
}

/* private */
bool
ConcaveHull::isRemovableBorder(const HullTri* tri) const
{
    /**
     * Tri must have exactly 2 adjacent tris (i.e. a single boundary edge).
     * If it it has only 0 or 1 adjacent then removal would remove a vertex.
     * If it has 3 adjacent then it is not on border.
     */
    if (tri->numAdjacent() != 2) return false;
    /**
     * The tri cannot be removed if it is connecting, because
     * this would create more than one result polygon.
     */
    return ! tri->isConnecting();
}

/* private */
bool
ConcaveHull::isRemovableHole(const HullTri* tri) const
{
    /**
     * Tri must have exactly 2 adjacent tris (i.e. a single boundary edge).
     * If it it has only 0 or 1 adjacent then removal would remove a vertex.
     * If it has 3 adjacent then it is not connected to hole.
     */
    if (tri->numAdjacent() != 2) return false;
    /**
     * Ensure removal does not disconnect hull area.
     * This is a fast check which ensure holes and boundary
     * do not touch at single points.
     * (But it is slightly over-strict, since it prevents
     * any touching holes.)
     */
    return ! tri->hasBoundaryTouch();
}

/* private */
std::unique_ptr<Geometry>
ConcaveHull::toGeometry(TriList<HullTri>& triList, const GeometryFactory* factory)
{
    if (! isHolesAllowed) {
        return HullTriangulation::traceBoundaryPolygon(triList, factory);
    }
    //-- in case holes are present use union (slower but handles holes)
    return HullTriangulation::geomunion(triList, factory);
}







} // namespace geos.algorithm.hull
} // namespace geos.algorithm
} // namespace geos

