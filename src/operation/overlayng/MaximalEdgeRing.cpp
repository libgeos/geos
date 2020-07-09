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

#include <geos/operation/overlayng/MaximalEdgeRing.h>
#include <geos/operation/overlayng/OverlayEdge.h>
#include <geos/operation/overlayng/OverlayEdgeRing.h>
#include <geos/geom/Location.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/util/TopologyException.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>


#include <cassert>

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

using namespace geos::geom;


/*public static*/
void
MaximalEdgeRing::linkResultAreaMaxRingAtNode(OverlayEdge* nodeEdge)
{
    // assertion is only valid if building a polygonal geometry (ie not a coverage)
    assert(nodeEdge->isInResultArea());

    /**
     * Since the node edge is an out-edge,
     * make it the last edge to be linked
     * by starting at the next edge.
     * The node edge cannot be an in-edge as well,
     * but the next one may be the first in-edge.
     */
    OverlayEdge* endOut = nodeEdge->oNextOE();
    OverlayEdge* currOut = endOut;

    int state = STATE_FIND_INCOMING;
    OverlayEdge* currResultIn = nullptr;
    do {
        /**
        * If an edge is linked this node has already been processed
        * so can skip further processing
        */
        if (currResultIn != nullptr && currResultIn->isResultMaxLinked())
            return;

        switch (state) {
            case STATE_FIND_INCOMING: {
                OverlayEdge* currIn = currOut->symOE();
                if (! currIn->isInResultArea())
                    break;
                currResultIn = currIn;
                state = STATE_LINK_OUTGOING;
                break;
            }
            case STATE_LINK_OUTGOING: {
                if (! currOut->isInResultArea())
                    break;
                // link the in edge to the out edge
                currResultIn->setNextResultMax(currOut);
                state = STATE_FIND_INCOMING;
                break;
            }
        }
        currOut = currOut->oNextOE();
    }
    while (currOut != endOut);

    if (state == STATE_LINK_OUTGOING) {
        throw util::TopologyException("no outgoing edge found", nodeEdge->getCoordinate());
    }
}


/*private*/
void
MaximalEdgeRing::attachEdges(OverlayEdge* startEdge)
{
    OverlayEdge* edge = startEdge;
    do {
        if (edge == nullptr)
            throw util::TopologyException("Found null edge in ring");
        if (edge->getEdgeRingMax() == this)
            throw util::TopologyException("Edge visited twice during ring-building", edge->getCoordinate());
        if (edge->nextResultMax() == nullptr) {
            throw util::TopologyException("Found null edge in ring", edge->dest());
        }
        edge->setEdgeRingMax(this);
        edge = edge->nextResultMax();
    }
    while (edge != startEdge);
}

/*public*/
std::vector<std::unique_ptr<OverlayEdgeRing>>
MaximalEdgeRing::buildMinimalRings(const GeometryFactory* geometryFactory)
{
    linkMinimalRings();
    std::vector<std::unique_ptr<OverlayEdgeRing>> outOERs;
    OverlayEdge* e = startEdge;
    do {
        if (e->getEdgeRing() == nullptr) {
            outOERs.emplace_back(new OverlayEdgeRing(e, geometryFactory));
        }
        e = e->nextResultMax();
    }
    while (e != startEdge);
    return outOERs;
}

/*private*/
void
MaximalEdgeRing::linkMinimalRings()
{
    OverlayEdge* e = startEdge;
    do {
        linkMinRingEdgesAtNode(e, this);
        e = e->nextResultMax();
    }
    while (e != startEdge);
}

/*private*/
void
MaximalEdgeRing::linkMinRingEdgesAtNode(OverlayEdge* nodeEdge, MaximalEdgeRing* maxRing)
{

    /**
     * The node edge is an out-edge,
     * so it is the first edge linked
     * with the next CCW in-edge
     */
    OverlayEdge* endOut = nodeEdge;
    OverlayEdge* currMaxRingOut = endOut;
    OverlayEdge* currOut = endOut->oNextOE();

    do {
        if (isAlreadyLinked(currOut->symOE(), maxRing))
            return;

        if (currMaxRingOut == nullptr) {
            currMaxRingOut = selectMaxOutEdge(currOut, maxRing);
        }
        else {
            currMaxRingOut = linkMaxInEdge(currOut, currMaxRingOut, maxRing);
        }
        currOut = currOut->oNextOE();
    }
    while (currOut != endOut);

    if (currMaxRingOut != nullptr) {
        throw util::TopologyException("Unmatched edge found during min-ring linking", nodeEdge->getCoordinate());
    }
}

/*private*/
bool
MaximalEdgeRing::isAlreadyLinked(OverlayEdge* edge, MaximalEdgeRing* maxRing) const
{
    bool isLinked = (edge->getEdgeRingMax() == maxRing) &&
                    (edge->isResultLinked());
    return isLinked;
}

/*private*/
OverlayEdge*
MaximalEdgeRing::selectMaxOutEdge(OverlayEdge* currOut, MaximalEdgeRing* maxEdgeRing) const
{
    // select if currOut edge is part of this max ring
    if (currOut->getEdgeRingMax() ==  maxEdgeRing)
        return currOut;
    // otherwise skip this edge
    return nullptr;
}

/*private*/
OverlayEdge*
MaximalEdgeRing::linkMaxInEdge(OverlayEdge* currOut, OverlayEdge* currMaxRingOut, MaximalEdgeRing* maxEdgeRing) const
{
    OverlayEdge* currIn = currOut->symOE();
    // currIn is not in this max-edgering, so keep looking
    if (currIn->getEdgeRingMax() !=  maxEdgeRing)
        return currMaxRingOut;

    currIn->setNextResult(currMaxRingOut);
    return nullptr;
}



// /*public*/
// String toString()
// {
//     Coordinate[] pts = getCoordinates();
//     return WKTWriter.toLineString(pts);
// }

// /*private*/
// Coordinate[]
// getCoordinates()
// {
//     CoordinateList coords = new CoordinateList();
//     OverlayEdge* edge = startEdge;
//     do {
//       coords.add(edge->orig());
//       if (edge == nullptr)
//         break;
//       if (edge->nextResultMax() == nullptr) {
//         break;
//       }
//       edge = edge->nextResultMax();
//     } while (edge != startEdge);
//     // add last coordinate
//     coords.add(edge->dest());
//     return coords.toCoordinateArray();
// }





} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
