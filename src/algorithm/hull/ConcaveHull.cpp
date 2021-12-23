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

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateList.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/operation/overlayng/CoverageUnion.h>
#include <geos/triangulate/DelaunayTriangulationBuilder.h>
#include <geos/triangulate/quadedge/QuadEdge.h>
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
// #include <geos/triangulate/quadedge/TriangleVisitor.h>
// #include <geos/triangulate/tri/Tri.h>
#include <geos/triangulate/tri/TriangulationBuilder.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/IllegalStateException.h>
#include <geos/util/Assert.h>


using geos::geom::Coordinate;
using geos::geom::CoordinateList;
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
    std::size_t numPts = geom->getNumPoints();
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
ConcaveHull::concaveHullByLength(const Geometry* geom, double maxLength, bool isHolesAllowed)
{
    ConcaveHull hull(geom);
    hull.setMaximumEdgeLength(maxLength);
    hull.setHolesAllowed(isHolesAllowed);
    return hull.getHull();
}


/* public static */
std::unique_ptr<Geometry>
ConcaveHull::concaveHullByArea(const Geometry* geom, double areaRatio)
{
    ConcaveHull hull(geom);
    hull.setMaximumAreaRatio(areaRatio);
    return hull.getHull();
}


/* public */
void
ConcaveHull::setMaximumEdgeLength(double edgeLength)
{
    if (edgeLength < 0)
        throw util::IllegalArgumentException("Edge length must be non-negative");
    maxEdgeLength = edgeLength;
}


/* public */
void
ConcaveHull::setMaximumAreaRatio(double areaRatio)
{
    if (areaRatio < 0 || areaRatio > 1)
        throw util::IllegalArgumentException("Area ratio must be in range [0,1]");
    maxAreaRatio = areaRatio;
}


/* public */
void
ConcaveHull::setHolesAllowed(bool p_isHolesAllowed)
{
    isHolesAllowed = p_isHolesAllowed;
}


/* public */
std::unique_ptr<Geometry>
ConcaveHull::getHull()
{
    TriList<HullTri> triList;
    createDelaunayTriangulation(inputGeometry, triList);
    computeHull(triList);
    std::unique_ptr<Geometry> hull = toPolygon(triList, geomFactory);
    return hull;
}


/* private */
void
ConcaveHull::computeHull(TriList<HullTri>& triList)
{
    //-- used if area is the threshold criteria
    double areaConvex = triList.area();
    double areaConcave = areaConvex;

    // Sort the HullTri from largest (at head) to smallest
    HullTriQueue queue;
    initQueue(queue, triList);

    // remove tris in order of decreasing size (edge length)
    while (! queue.empty()) {
        if (isBelowAreaThreshold(areaConcave, areaConvex))
            break;

        HullTri* tri = queue.top();
        queue.pop();

        if (isBelowLengthThreshold(tri))
            break;

        if (isRemovable(tri, triList)) {
            //-- the non-null adjacents are now on the border
            HullTri* adj0 = static_cast<HullTri*>(tri->getAdjacent(0));
            HullTri* adj1 = static_cast<HullTri*>(tri->getAdjacent(1));
            HullTri* adj2 = static_cast<HullTri*>(tri->getAdjacent(2));

            //-- remove tri
            tri->remove();
            triList.remove(tri);
            areaConcave -= tri->getArea();

            //-- if holes not allowed, add new border adjacents to queue
            if (! isHolesAllowed) {
                addBorderTri(adj0, queue);
                addBorderTri(adj1, queue);
                addBorderTri(adj2, queue);
            }
        }
    }
}


/* private */
void
ConcaveHull::initQueue(HullTriQueue& queue, TriList<HullTri>& triList)
{
    for (auto* tri : triList) {
        if (! isHolesAllowed) {
            //-- add only border triangles which could be eroded
            // (if tri has only 1 adjacent it can't be removed because that would isolate a vertex)
            if (tri->numAdjacent() != 2)
                continue;
            tri->setSizeToBorder();
        }
        queue.push(tri);
    }
    return;
}


/* private */
void
ConcaveHull::addBorderTri(HullTri* tri, HullTriQueue& queue)
{
    if (tri == nullptr) return;
    if (tri->numAdjacent() != 2) return;
    tri->setSizeToBorder();
    queue.push(tri);
}


/* private */
bool
ConcaveHull::isBelowAreaThreshold(double areaConcave, double areaConvex) const
{
    return areaConcave / areaConvex <= maxAreaRatio;
}


/* private */
bool
ConcaveHull::isBelowLengthThreshold(const HullTri* tri) const
{
    double len = 0;
    if (isHolesAllowed) {
        len = tri->lengthOfLongestEdge();
    }
    else {
        len = tri->lengthOfBorder();
    }
    return len < maxEdgeLength;
}


/* private */
bool
ConcaveHull::isRemovable(HullTri* tri, TriList<HullTri>& triList)
{
    if (isHolesAllowed) {
        /**
        * Don't remove if that would separate a single vertex
        */
        if (hasVertexSingleAdjacent(tri, triList))
            return false;
        return HullTri::isConnected(triList, tri);
    }

    //-- compute removable for no holes allowed
    int numAdj = tri->numAdjacent();
    /**
     * Tri must have exactly 2 adjacent tris.
     * If it it has only 0 or 1 adjacent then removal would remove a vertex.
     * If it has 3 adjacent then it is not on border.
     */
    if (numAdj != 2) return false;
    /**
     * The tri cannot be removed if it is connecting, because
     * this would create more than one result polygon.
     */
    return ! isConnecting(tri);
}


/* private static */
bool
ConcaveHull::hasVertexSingleAdjacent(const HullTri* tri, const TriList<HullTri>& triList)
{
    for (TriIndex i = 0; i < 3; i++) {
        if (degree(tri->getCoordinate(i), triList) <= 1)
            return true;
    }
    return false;
}


/* private static */
std::size_t
ConcaveHull::degree(const Coordinate& v, const TriList<HullTri>& triList)
{
    std::size_t degree = 0;
    for (const auto* tri : triList) {
        for (TriIndex i = 0; i < 3; i++) {
            if (v.equals2D(tri->getCoordinate(i)))
                degree++;
        }
    }
    return degree;
}


/* private static */
bool
ConcaveHull::isConnecting(const HullTri* tri)
{
    int adj2Index = adjacent2VertexIndex(tri);
    bool isInterior = isInteriorVertex(tri, adj2Index);
    return ! isInterior;
}


/* private static */
bool
ConcaveHull::isInteriorVertex(const HullTri* triStart, TriIndex index)
{
    const HullTri* curr = triStart;
    TriIndex currIndex = index;
    do {
        Tri* adj = curr->getAdjacent(currIndex);
        if (adj == nullptr) return false;
        TriIndex adjIndex = adj->getIndex(curr);
        curr = static_cast<HullTri*>(adj);
        currIndex = Tri::next(adjIndex);
    }
    while (curr != triStart);
    return true;
}


/* private static */
TriIndex
ConcaveHull::adjacent2VertexIndex(const HullTri* tri)
{
    if (tri->hasAdjacent(0) && tri->hasAdjacent(1)) return 1;
    if (tri->hasAdjacent(1) && tri->hasAdjacent(2)) return 2;
    if (tri->hasAdjacent(2) && tri->hasAdjacent(0)) return 0;
    return -1;
}


/* private static */
void
ConcaveHull::createDelaunayTriangulation(const Geometry* geom, TriList<HullTri>& triList)
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
ConcaveHull::toTris(QuadEdgeSubdivision& subdiv, TriList<HullTri>& triList)
{
    HullTriVisitor visitor(triList);
    subdiv.visitTriangles(&visitor, false);
    TriangulationBuilder tb;
    for (auto* tri : triList) {
        tb.add(static_cast<Tri*>(tri));
    }
    return;
}


/* private */
std::unique_ptr<Geometry>
ConcaveHull::toPolygon(TriList<HullTri>& triList, const GeometryFactory* factory)
{
    if (! isHolesAllowed) {
        return extractPolygon(triList, factory);
    }
    //-- in case holes are present use union (slower but handles holes)
    return geomunion(triList, factory);
}


/* private */
std::unique_ptr<Geometry>
ConcaveHull::extractPolygon(TriList<HullTri>& triList, const GeometryFactory* factory)
{
    if (triList.size() == 1) {
        HullTri* tri = triList[0];
        return tri->toPolygon(factory);
    }
    std::vector<Coordinate> pts = traceBorder(triList);
    return factory->createPolygon(std::move(pts));
}


/* private static */
std::unique_ptr<Geometry>
ConcaveHull::geomunion(TriList<HullTri>& triList, const GeometryFactory* factory)
{
    std::vector<std::unique_ptr<Polygon>> polys;
    // List<Polygon> polys = new ArrayList<Polygon>();
    for (auto* tri : triList) {
        std::unique_ptr<Polygon> poly = tri->toPolygon(factory);
        polys.emplace_back(poly.release());
    }
    std::unique_ptr<Geometry> geom = factory->buildGeometry(std::move(polys));
    return CoverageUnion::geomunion(geom.release());
}


/* private static */
std::vector<Coordinate>
ConcaveHull::traceBorder(TriList<HullTri>& triList)
{
    HullTri* triStart = findBorderTri(triList);
    // std::vector<Coordinate> coordList;
    CoordinateList coordList;
    HullTri* tri = triStart;
    do {
        TriIndex borderIndex = tri->borderIndexCCW();
        //-- add border vertex
        coordList.add(tri->getCoordinate(borderIndex), false);
        TriIndex nextIndex = Tri::next(borderIndex);
        //-- if next edge is also border, add it and move to next
        if (tri->isBorder(nextIndex)) {
            coordList.add(tri->getCoordinate(nextIndex), false);
            borderIndex = nextIndex;
        }
        //-- find next border tri CCW around non-border edge
        tri = tri->nextBorderTri();
    } while (tri != triStart);
    coordList.closeRing();
    return *(coordList.toCoordinateArray());
}


/* public static */
HullTri*
ConcaveHull::findBorderTri(TriList<HullTri>& triList)
{
    for (auto* tri : triList) {
        if (tri->isBorder()) return tri;
    }
    util::Assert::shouldNeverReachHere("No border triangles found");
    return nullptr;
}




/* HullTri ------------------------------------------------------------ */

/* public */
double
HullTri::getSize() const
{
    return m_size;
}

/* public */
void
HullTri::setSizeToBorder()
{
    m_size = lengthOfBorder();
}

/* public */
bool
HullTri::isMarked() const
{
    return m_isMarked;
}

/* public */
void
HullTri::setMarked(bool marked)
{
    m_isMarked = marked;
}

/* public */
bool
HullTri::isBorder() const
{
    return isBorder(0) || isBorder(1) || isBorder(2);
}

/* public */
bool
HullTri::isBorder(TriIndex index) const
{
    return ! hasAdjacent(index);
}

/* public */
TriIndex
HullTri::borderIndex() const
{
    if (isBorder(0)) return 0;
    if (isBorder(1)) return 1;
    if (isBorder(2)) return 2;
    return -1;
}

/**
* Gets the most CCW border edge index.
* This assumes there is at least one non-border edge.
*
* @return the CCW border edge index
*/
/* public */
TriIndex
HullTri::borderIndexCCW() const
{
    TriIndex index = borderIndex();
    TriIndex prevIndex = prev(index);
    if (isBorder(prevIndex)) {
        return prevIndex;
    }
    return index;
}

/**
* Gets the most CW border edge index.
* This assumes there is at least one non-border edge.
*
* @return the CW border edge index
*/
/* public */
TriIndex
HullTri::borderIndexCW() const
{
    TriIndex index = borderIndex();
    TriIndex nextIndex = next(index);
    if (isBorder(nextIndex)) {
        return nextIndex;
    }
    return index;
}


/* public */
double
HullTri::lengthOfLongestEdge() const
{
    return Triangle::longestSideLength(p0, p1, p2);
}


/* public */
HullTri*
HullTri::nextBorderTri()
{
    HullTri* tri = this;
    // start at first non-border edge CW
    TriIndex index = next(borderIndexCW());
    // scan CCW around vertex for next border tri
    do {
        HullTri* adjTri = static_cast<HullTri*>(tri->getAdjacent(index));
        if (adjTri == this) {
            throw util::IllegalStateException("No outgoing border edge found");
        }
        index = next(adjTri->getIndex(tri));
        tri = adjTri;
    }
    while (! tri->isBorder(index));
    return (tri);
}


/* public */
double
HullTri::lengthOfBorder() const
{
    double len = 0.0;
    for (TriIndex i = 0; i < 3; i++) {
        if (! hasAdjacent(i)) {
            len += getCoordinate(i).distance(getCoordinate(Tri::next(i)));
        }
    }
    return len;
}

/* public static */
bool
HullTri::isConnected(TriList<HullTri>& triList, HullTri* exceptTri)
{
      if (triList.size() == 0) return false;
      clearMarks(triList);
      HullTri* triStart = findTri(triList, exceptTri);
      if (triStart == nullptr) return false;
      markConnected(triStart, exceptTri);
      exceptTri->setMarked(true);
      return isAllMarked(triList);
}

/* public static */
void
HullTri::clearMarks(TriList<HullTri>& triList)
{
    for (auto* tri : triList) {
        tri->setMarked(false);
    }
}

/* public static */
HullTri*
HullTri::findTri(TriList<HullTri>& triList, Tri* exceptTri)
{
    for (auto* tri : triList) {
        if (tri != exceptTri) return tri;
    }
    return nullptr;
}

/* public static */
bool
HullTri::isAllMarked(TriList<HullTri>& triList)
{
    for (auto* tri : triList) {
        if (! tri->isMarked())
            return false;
    }
    return true;
}


/* public static */
void
HullTri::markConnected(HullTri* triStart, HullTri* exceptTri)
{
    std::deque<HullTri*> queue;
    queue.push_back(triStart);
    while (! queue.empty()) {
        HullTri* tri = queue.front();
        queue.pop_front();
        tri->setMarked(true);
        for (TriIndex i = 0; i < 3; i++) {
            HullTri* adj = static_cast<HullTri*>(tri->getAdjacent(i));
            //-- don't connect thru this tri
            if (adj == exceptTri) {
                continue;
            }
            if (adj != nullptr && ! adj->isMarked()) {
                queue.push_back(adj);
            }
        }
    }
}


/* HullTriVisitor ------------------------------------------------------------ */

void
HullTriVisitor::visit(std::array<QuadEdge*, 3>& triEdges)
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
};




} // namespace geos.algorithm.hull
} // namespace geos.algorithm
} // namespace geos

