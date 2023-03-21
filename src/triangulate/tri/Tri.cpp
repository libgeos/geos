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

#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Triangle.h>
#include <geos/triangulate/tri/Tri.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/IllegalStateException.h>
#include <geos/util.h>


using geos::util::IllegalArgumentException;
using geos::algorithm::Orientation;
using geos::geom::Triangle;

namespace geos {        // geos
namespace triangulate { // geos.triangulate
namespace tri {         // geos.triangulate.tri


/* public */
void
Tri::setAdjacent(Tri* p_tri0, Tri* p_tri1, Tri* p_tri2)
{
    tri0 = p_tri0;
    tri1 = p_tri1;
    tri2 = p_tri2;
}

/* public */
void
Tri::setAdjacent(const Coordinate& pt, Tri* tri)
{
    TriIndex index = getIndex(pt);
    setTri(index, tri);
    // TODO: validate that tri is adjacent at the edge specified
}

/* public */
void
Tri::setTri(TriIndex edgeIndex, Tri* tri)
{
    switch (edgeIndex) {
        case 0: tri0 = tri; return;
        case 1: tri1 = tri; return;
        case 2: tri2 = tri; return;
    }
    throw util::IllegalArgumentException("Tri::setTri - invalid index");
}

/* private */
void
Tri::setCoordinates(const Coordinate& np0, const Coordinate& np1, const Coordinate& np2)
{
    p0 = np0;
    p1 = np1;
    p2 = np2;
}

/* public */
void
Tri::flip(TriIndex index)
{
    Tri* tri = getAdjacent(index);
    TriIndex index1 = tri->getIndex(this);

    Coordinate adj0 = getCoordinate(index);
    Coordinate adj1 = getCoordinate(next(index));
    Coordinate opp0 = getCoordinate(oppVertex(index));
    Coordinate opp1 = tri->getCoordinate(oppVertex(index1));

    flip(tri, index, index1, adj0, adj1, opp0, opp1);
}

/* private */
void
Tri::flip(Tri* tri, TriIndex index0, TriIndex index1,
    const Coordinate& adj0, const Coordinate& adj1,
    const Coordinate& opp0, const Coordinate& opp1)
{
    //System.out.println("Flipping: " + this + " -> " + tri);
    //validate();
    //tri.validate();

    setCoordinates(opp1, opp0, adj0);
    tri->setCoordinates(opp0, opp1, adj1);

    /**
     *  Order: 0: opp0-adj0 edge, 1: opp0-adj1 edge,
     *  2: opp1-adj0 edge, 3: opp1-adj1 edge
     */
    std::vector<Tri*> adjacent = getAdjacentTris(tri, index0, index1);
    setAdjacent(tri, adjacent[0], adjacent[2]);
    //--- update the adjacent triangles with new adjacency
    if ( adjacent[2] != nullptr ) {
        adjacent[2]->replace(tri, this);
    }
    tri->setAdjacent(this, adjacent[3], adjacent[1]);
    if ( adjacent[1] != nullptr ) {
        adjacent[1]->replace(this, tri);
    }
    //validate();
    //tri.validate();
}

/**
* Replace triOld with triNew
*
* @param triOld
* @param triNew
*/
/* private */
void
Tri::replace(Tri* triOld, Tri* triNew)
{
    if ( tri0 != nullptr && tri0 == triOld ) {
        tri0 = triNew;
    }
    else if ( tri1 != nullptr && tri1 == triOld ) {
        tri1 = triNew;
    }
    else if ( tri2 != nullptr && tri2 == triOld ) {
        tri2 = triNew;
    }
}


/* public */
void
Tri::remove(TriList<Tri>& triList)
{
    remove();
    triList.remove(this);
}

/* public */
void
Tri::remove()
{
    remove(0);
    remove(1);
    remove(2);
}

/* private */
void
Tri::remove(TriIndex index)
{
    Tri* adj = getAdjacent(index);
    if (adj == nullptr) return;
    adj->setTri(adj->getIndex(this), nullptr);
    setTri(index, nullptr);
}


/**
*
* Order: 0: opp0-adj0 edge, 1: opp0-adj1 edge,
*  2: opp1-adj0 edge, 3: opp1-adj1 edge
*
* @param tri
* @param index0
* @param index1
* @return
*/
/* private */
std::vector<Tri*>
Tri::getAdjacentTris(Tri* triAdj, TriIndex index, TriIndex indexAdj)
{
    std::vector<Tri*> adj(4);
    adj[0] = getAdjacent(prev(index));
    adj[1] = getAdjacent(next(index));
    adj[2] = triAdj->getAdjacent(next(indexAdj));
    adj[3] = triAdj->getAdjacent(prev(indexAdj));
    return adj;
}

/* public */
void
Tri::validate()
{
    if ( Orientation::CLOCKWISE != Orientation::index(p0, p1, p2) ) {
        throw IllegalArgumentException("Tri is not oriented correctly");
    }

    validateAdjacent(0);
    validateAdjacent(1);
    validateAdjacent(2);
}

/* public */
void
Tri::validateAdjacent(TriIndex index)
{
    Tri* tri = getAdjacent(index);
    if (tri == nullptr) return;

    assert(isAdjacent(tri));
    assert(tri->isAdjacent(this));

    // const Coordinate& e0 = getCoordinate(index);
    // const Coordinate& e1 = getCoordinate(next(index));
    // TriIndex indexNeighbor = tri->getIndex(this);
    // const Coordinate& n0 = tri->getCoordinate(indexNeighbor);
    // const Coordinate& n1 = tri->getCoordinate(next(indexNeighbor));
    // assert(e0.equals2D(n1)); // Edge coord not equal
    // assert(e1.equals2D(n0)); // Edge coord not equal

    //--- check that no edges cross
    algorithm::LineIntersector li;
    for (TriIndex i = 0; i < 3; i++) {
        for (TriIndex j = 0; j < 3; j++) {
            const Coordinate& p00 = getCoordinate(i);
            const Coordinate& p01 = getCoordinate(next(i));
            const Coordinate& p10 = tri->getCoordinate(j);
            const Coordinate& p11 = tri->getCoordinate(next(j));
            li.computeIntersection(p00,  p01,  p10, p11);
            assert(!li.isProper());
        }
    }
}

/* public */
std::pair<const Coordinate&, const Coordinate&>
Tri::getEdge(Tri* neighbor) const
{
    TriIndex index = getIndex(neighbor);
    TriIndex nextTri = next(index);

    // const Coordinate& e0 = getCoordinate(index);
    // const Coordinate& e1 = getCoordinate(nextTri);
    // assert (neighbor->hasCoordinate(e0));
    // assert (neighbor->hasCoordinate(e1));
    // TriIndex iN = neighbor->getIndex(e0);
    // TriIndex iNPrev = prev(iN);
    // assert (neighbor->getIndex(e1) == iNPrev);

    std::pair<const Coordinate&, const Coordinate&> edge(getCoordinate(index), getCoordinate(nextTri));
    return edge;
}

/* public */
const Coordinate&
Tri::getEdgeStart(TriIndex i) const
{
    return getCoordinate(i);
}

/* public */
const Coordinate&
Tri::getEdgeEnd(TriIndex i) const
{
    return getCoordinate(next(i));
}

/* public */
bool
Tri::hasCoordinate(const Coordinate& v) const
{
    if ( p0.equals(v) || p1.equals(v) || p2.equals(v) ) {
        return true;
    }
    return false;
}

/* public */
const Coordinate&
Tri::getCoordinate(TriIndex i) const
{
    switch(i) {
    case 0: return p0;
    case 1: return p1;
    case 2: return p2;
    }
    throw util::IllegalArgumentException("Tri::getCoordinate - invalid index");
}

/* public */
TriIndex
Tri::getIndex(const Coordinate& p) const
{
    if ( p0.equals2D(p) )
        return 0;
    if ( p1.equals2D(p) )
        return 1;
    if ( p2.equals2D(p) )
        return 2;
    return -1;
}

/* public */
TriIndex
Tri::getIndex(const Tri* tri) const
{
    if ( tri0 == tri )
        return 0;
    if ( tri1 == tri )
        return 1;
    if ( tri2 == tri )
        return 2;
    return -1;
}

/* public */
Tri*
Tri::getAdjacent(TriIndex i) const
{
    switch(i) {
    case 0: return tri0;
    case 1: return tri1;
    case 2: return tri2;
    }
    throw util::IllegalArgumentException("Tri::getAdjacent - invalid index");
}

/* public */
bool
Tri::hasAdjacent() const
{
    return hasAdjacent(0) || hasAdjacent(1) || hasAdjacent(2);
}

/* public */
bool
Tri::hasAdjacent(TriIndex i) const
{
    return nullptr != getAdjacent(i);
}


/* public */
bool
Tri::isAdjacent(Tri* tri) const
{
    return getIndex(tri) >= 0;
}

/* public */
int
Tri::numAdjacent() const
{
    int num = 0;
    if ( tri0 != nullptr )
      num++;
    if ( tri1 != nullptr )
      num++;
    if ( tri2 != nullptr )
      num++;
    return num;
}

/* public */
bool
Tri::isInteriorVertex(TriIndex index) const
{
    const Tri* curr = this;
    TriIndex currIndex = index;
    do {
        const Tri* adj = curr->getAdjacent(currIndex);
        if (adj == nullptr) return false;
        TriIndex adjIndex = adj->getIndex(curr);
        if (adjIndex < 0) {
            throw util::IllegalStateException("Inconsistent adjacency - invalid triangulation");
        }
        curr = adj;
        currIndex = Tri::next(adjIndex);
    }
    while (curr != this);
    return true;
}

/* public */
bool
Tri::isBorder() const
{
    return isBoundary(0) || isBoundary(1) || isBoundary(2);
}

/* public */
bool
Tri::isBoundary(TriIndex index) const
{
    return ! hasAdjacent(index);
}

/* public static */
TriIndex
Tri::next(TriIndex i)
{
    switch (i) {
        case 0: return 1;
        case 1: return 2;
        case 2: return 0;
    }
    return -1;
}

/* public static */
TriIndex
Tri::prev(TriIndex i)
{
    switch (i) {
        case 0: return 2;
        case 1: return 0;
        case 2: return 1;
    }
    return -1;
}

/* public static */
TriIndex
Tri::oppVertex(TriIndex edgeIndex)
{
    return prev(edgeIndex);
}

/* public static */
TriIndex
Tri::oppEdge(TriIndex vertexIndex)
{
    return next(vertexIndex);
}


/* public */
Coordinate
Tri::midpoint(TriIndex edgeIndex) const
{
    const Coordinate& np0 = getCoordinate(edgeIndex);
    const Coordinate& np1 = getCoordinate(next(edgeIndex));
    double midX = (np0.x + np1.x) / 2;
    double midY = (np0.y + np1.y) / 2;
    return Coordinate(midX, midY);
}

/* public */
double
Tri::getArea() const
{
    return Triangle::area(p0, p1, p2);
}

/* public */
double
Tri::getLength() const
{
    return Triangle::length(p0, p1, p2);
}

/* public */
double
Tri::getLength(TriIndex i) const
{
    return getCoordinate(i).distance(getCoordinate(next(i)));
}

/* public */
std::unique_ptr<geom::Polygon>
Tri::toPolygon(const geom::GeometryFactory* gf) const
{
    auto coords = detail::make_unique<geom::CoordinateSequence>(4u);
    (*coords)[0] = p0;
    (*coords)[1] = p1;
    (*coords)[2] = p2;
    (*coords)[3] = p0;

    return gf->createPolygon(gf->createLinearRing(std::move(coords)));
}

/* public static */
std::unique_ptr<geom::Geometry>
Tri::toGeometry(std::set<Tri*>& tris, const geom::GeometryFactory* gf)
{
    std::vector<std::unique_ptr<geom::Polygon>> polys;
    for (Tri* tri: tris) {
        std::unique_ptr<geom::Polygon> poly = tri->toPolygon(gf);
        polys.emplace_back(poly.release());
    }
    return gf->createGeometryCollection(std::move(polys));
}


std::ostream&
operator<<(std::ostream& os, const Tri& tri)
{
    os << "POLYGON ((";
    os << tri.p0 << ", ";
    os << tri.p1 << ", ";
    os << tri.p2 << ", ";
    os << tri.p0 << "))";
    return os;
}


} // namespace geos.triangulate.tri
} // namespace geos.triangulate
} // namespace geos
