/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/planargraph/DirectedEdgeStar.h>
#include <geos/planargraph/DirectedEdge.h>

#include <vector>
#include <algorithm>


using namespace geos::geom;

namespace geos {
namespace planargraph {


/*
 * Adds a new member to this DirectedEdgeStar.
 */
void
DirectedEdgeStar::add(DirectedEdge* de)
{
    outEdges.push_back(de);
    sorted = false;
}

/*
 * Drops a member of this DirectedEdgeStar.
 */
void
DirectedEdgeStar::remove(DirectedEdge* de)
{
    for(unsigned int i = 0; i < outEdges.size(); ++i) {
        if(outEdges[i] == de) {
            outEdges.erase(std::next(outEdges.begin(), static_cast<int>(i)));
            --i;
        }
    }
}

std::vector<DirectedEdge*>::iterator
DirectedEdgeStar::begin()
{
    sortEdges();
    return outEdges.begin();
}

std::vector<DirectedEdge*>::iterator
DirectedEdgeStar::end()
{
    sortEdges();
    return outEdges.end();
}

std::vector<DirectedEdge*>::const_iterator
DirectedEdgeStar::begin() const
{
    sortEdges();
    return outEdges.begin();
}

std::vector<DirectedEdge*>::const_iterator
DirectedEdgeStar::end() const
{
    sortEdges();
    return outEdges.end();
}

/*
 * Returns the coordinate for the node at which this star is based
 */
Coordinate&
DirectedEdgeStar::getCoordinate() const
{
    if(outEdges.empty()) {
        static Coordinate nullCoord = Coordinate::getNull();
        return nullCoord;
    }
    DirectedEdge* e = outEdges[0];
    return e->getCoordinate();
}

/*
 * Returns the DirectedEdges, in ascending order by angle with
 * the positive x-axis.
 */
std::vector<DirectedEdge*>&
DirectedEdgeStar::getEdges()
{
    sortEdges();
    return outEdges;
}

bool
pdeLessThan(DirectedEdge* first, DirectedEdge* second)
{
    if(first->compareTo(second) < 0) {
        return true;
    }
    else {
        return false;
    }
}

/*private*/
void
DirectedEdgeStar::sortEdges() const
{
    if(!sorted) {
        sort(outEdges.begin(), outEdges.end(), pdeLessThan);
        sorted = true;
    }
}

/*
 * Returns the zero-based index of the given Edge, after sorting in
 * ascending order by angle with the positive x-axis.
 */
int
DirectedEdgeStar::getIndex(const Edge* edge)
{
    sortEdges();
    for(unsigned int i = 0; i < outEdges.size(); ++i) {
        DirectedEdge* de = outEdges[i];
        if(de->getEdge() == edge) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

/*
 * Returns the zero-based index of the given DirectedEdge, after sorting
 * in ascending order by angle with the positive x-axis.
 */
int
DirectedEdgeStar::getIndex(const DirectedEdge* dirEdge)
{
    sortEdges();
    for(unsigned int i = 0; i < outEdges.size(); ++i) {
        DirectedEdge* de = outEdges[i];
        if(de == dirEdge) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

/*
 * Returns the remainder when i is divided by the number of edges in this
 * DirectedEdgeStar.
 */
unsigned int
DirectedEdgeStar::getIndex(int i) const
{
    int modi = i % (int)outEdges.size();
    //I don't think modi can be 0 (assuming i is positive) [Jon Aquino 10/28/2003]
    if(modi < 0) {
        modi += (int)outEdges.size();
    }
    return static_cast<unsigned int>(modi);
}

/*
 * Returns the DirectedEdge on the left-hand side of the given
 * DirectedEdge (which must be a member of this DirectedEdgeStar).
 */
DirectedEdge*
DirectedEdgeStar::getNextEdge(DirectedEdge* dirEdge)
{
    int i = getIndex(dirEdge);
    return outEdges[getIndex(i + 1)];
}

} // namespace planargraph
} // namespace geos

