/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2012 Excensus LLC.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: triangulate/IncrementalDelaunayTriangulator.java rev. r524
 *
 **********************************************************************/

#include <geos/triangulate/IncrementalDelaunayTriangulator.h>

#include <geos/triangulate/quadedge/QuadEdge.h>
#include <geos/triangulate/quadedge/QuadEdgeSubdivision.h>
#include <geos/triangulate/quadedge/LocateFailureException.h>
#include <geos/algorithm/Orientation.h>

namespace geos {
namespace triangulate { //geos.triangulate

using namespace algorithm;
using namespace quadedge;

IncrementalDelaunayTriangulator::IncrementalDelaunayTriangulator(
    QuadEdgeSubdivision* p_subdiv) :
    subdiv(p_subdiv), isUsingTolerance(p_subdiv->getTolerance() > 0.0),
    m_isForceConvex(true)
{
}

void 
IncrementalDelaunayTriangulator::forceConvex(bool isForceConvex) 
{
    m_isForceConvex = isForceConvex;
}

void
IncrementalDelaunayTriangulator::insertSites(const VertexList& vertices)
{
    for(const auto& vertex : vertices) {
        insertSite(vertex);
    }
}

QuadEdge&
IncrementalDelaunayTriangulator::insertSite(const Vertex& v)
{
    /*
     * This code is based on Guibas and Stolfi (1985), with minor modifications
     * and a bug fix from Dani Lischinski (Graphic Gems 1993). (The modification
     * I believe is the test for the inserted site falling exactly on an
     * existing edge. Without this test zero-width triangles have been observed
     * to be created)
     */
    QuadEdge* e = subdiv->locate(v);

    if(!e) {
        throw LocateFailureException("Could not locate vertex.");
    }

    if(subdiv->isVertexOfEdge(*e, v)) {
        // point is already in subdivision.
        return *e;
    }
    else if(subdiv->isOnEdge(*e, v.getCoordinate())) {
        // the point lies exactly on an edge, so delete the edge
        // (it will be replaced by a pair of edges which have the point as a vertex)
        e = &e->oPrev();
        subdiv->remove(e->oNext());
    }

    /*
     * Connect the new point to the vertices of the containing triangle
     * (or quadrilateral, if the new point fell on an existing edge.)
     */
    QuadEdge* base = &subdiv->makeEdge(e->orig(), v);

    QuadEdge::splice(*base, *e);
    QuadEdge* startEdge = base;
    do {
        base = &subdiv->connect(*e, base->sym());
        e = &base->oPrev();
    }
    while(&e->lNext() != startEdge);

    /**
     * Examine suspect edges to ensure that the Delaunay condition is satisfied.
     * If it is not, flip the edge and continue scanning.
     * 
     * Since the frame is not infinitely far away,
     * edges which touch the frame or are adjacent to it require special logic
     * to ensure the inner triangulation maintains a convex boundary.
     */
    for(;;) {
        //-- general case - flip if vertex is in circumcircle
        QuadEdge* t = &e->oPrev();
        bool doFlip = t->dest().rightOf(*e) &&
                v.isInCircle(e->orig(), t->dest(), e->dest());
        
        if (m_isForceConvex) {
            //-- special cases to ensure triangulation boundary is convex
            if (isConcaveBoundary(*e)) {
            //-- flip if the triangulation boundary is concave
                doFlip = true;
            }
            else if (isBetweenFrameAndInserted(*e, v)) {
            //-- don't flip if edge lies between the inserted vertex and a frame vertex
                doFlip = false;
            }
        }

        if (doFlip) {
            QuadEdge::swap(*e);
            e = &e->oPrev();
            continue;
        }
        if (&e->oNext() == startEdge) {
            return *base; // no more suspect edges.
        }
        //-- check next edge
        e = &e->oNext().lPrev();
    }
}

bool 
IncrementalDelaunayTriangulator::isConcaveBoundary(const QuadEdge& e) const
{
    if (subdiv->isFrameVertex(e.dest())) {
        return isConcaveAtOrigin(e);
    }
    if (subdiv->isFrameVertex(e.orig())) {
        return isConcaveAtOrigin(e.sym());
    }
    return false;
}

bool 
IncrementalDelaunayTriangulator::isConcaveAtOrigin(const QuadEdge& e) const 
{
    const Coordinate& p = e.orig().getCoordinate();
    const Coordinate& pp = e.oPrev().dest().getCoordinate();
    const Coordinate& pn = e.oNext().dest().getCoordinate();
    bool isConcave = Orientation::COUNTERCLOCKWISE == Orientation::index(pp, pn, p);
    return isConcave;
}

bool 
IncrementalDelaunayTriangulator::isBetweenFrameAndInserted(const QuadEdge& e, const Vertex& vInsert) const
{
    const Vertex& v1 = e.oNext().dest();
    const Vertex& v2 = e.oPrev().dest();
    return (v1.getCoordinate() == vInsert.getCoordinate() && subdiv->isFrameVertex(v2))
        || (v2.getCoordinate() == vInsert.getCoordinate() && subdiv->isFrameVertex(v1));
}

} //namespace geos.triangulate
} //namespace geos

