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

#include <geos/algorithm/hull/HullTri.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/Triangle.h>

using geos::geom::Coordinate;
using geos::geom::Triangle;


namespace geos {
namespace algorithm { // geos.algorithm
namespace hull {      // geos.algorithm.hulll

/* public */
double
HullTri::getSize() const
{
    return m_size;
}

/* public */
void
HullTri::setSizeToBoundary()
{
    m_size = lengthOfBoundary();
}

/* public */
void
HullTri::setSizeToLongestEdge()
{
    m_size = lengthOfLongestEdge();
}

/* public */
void
HullTri::setSizeToCircumradius()
{
    m_size = Triangle::circumradius(p2, p1, p0);
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
HullTri::isRemoved()
{
    return ! hasAdjacent();
}

/* public */
TriIndex
HullTri::boundaryIndex() const
{
    if (isBoundary(0)) return 0;
    if (isBoundary(1)) return 1;
    if (isBoundary(2)) return 2;
    return -1;
}

/* public */
TriIndex
HullTri::boundaryIndexCCW() const
{
    TriIndex index = boundaryIndex();
    if (index < 0) return -1;
    TriIndex prevIndex = prev(index);
    if (isBoundary(prevIndex)) {
        return prevIndex;
    }
    return index;
}

/* public */
TriIndex
HullTri::boundaryIndexCW() const
{
    TriIndex index = boundaryIndex();
    if (index < 0) return -1;
    TriIndex nextIndex = next(index);
    if (isBoundary(nextIndex)) {
        return nextIndex;
    }
    return index;
}

/* public */
bool
HullTri::isConnecting() const
{
    TriIndex adj2Index = adjacent2VertexIndex();
    bool isInterior = isInteriorVertex(adj2Index);
    return ! isInterior;
}

/* public */
int
HullTri::adjacent2VertexIndex() const
{
    if (hasAdjacent(0) && hasAdjacent(1)) return 1;
    if (hasAdjacent(1) && hasAdjacent(2)) return 2;
    if (hasAdjacent(2) && hasAdjacent(0)) return 0;
    return -1;
}

/* public */
TriIndex
HullTri::isolatedVertexIndex(TriList<HullTri>& triList) const
{
    for (TriIndex i = 0; i < 3; i++) {
      if (triList.degree(this, i) <= 1)
        return i;
    }
    return -1;
}

/* public */
double
HullTri::lengthOfLongestEdge() const
{
    return Triangle::longestSideLength(p0, p1, p2);
}

/* public */
double
HullTri::lengthOfBoundary() const
{
    double len = 0.0;
    for (TriIndex i = 0; i < 3; i++) {
        if (! hasAdjacent(i)) {
            len += getCoordinate(i).distance(getCoordinate(Tri::next(i)));
        }
    }
    return len;
}

/* public */
bool
HullTri::hasBoundaryTouch() const
{
    for (TriIndex i = 0; i < 3; i++) {
        if (isBoundaryTouch(i))
            return true;
    }
    return false;
}

/* private */
bool
HullTri::isBoundaryTouch(TriIndex index) const
{
    //-- If vertex is in a boundary edge it is not a touch
    if (isBoundary(index)) return false;
    if (isBoundary(prev(index))) return false;
    //-- if vertex is not in interior it is on boundary
    return ! isInteriorVertex(index);
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
HullTri::clearMarks(TriList<HullTri>& triList)
{
    for (auto* tri : triList) {
        tri->setMarked(false);
    }
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

/*public friend*/
std::ostream&
operator<<(std::ostream& os, const HullTri& ht)
{
    os << "(" << ht.p0 << ",";
    os << ht.p1 << ",";
    os << ht.p2 << ") ";
    os << ht.tri0 << " ";
    os << ht.tri1 << " ";
    os << ht.tri2 << " ";
    os << ht.m_size;
    return os;
}

/* public */
void
HullTri::remove(TriList<HullTri>& triList)
{
    Tri::remove();
    triList.remove(this);
}



} // namespace geos.algorithm.hull
} // namespace geos.algorithm
} // namespace geos

