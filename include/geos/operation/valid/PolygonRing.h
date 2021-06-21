/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 * Copyright (C) 2021 Martin Davis
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#pragma once

#include <geos/operation/valid/PolygonRingTouch.h>
#include <geos/operation/valid/PolygonRingSelfNode.h>

#include <geos/export.h>


#include <memory>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class LinearRing;
}
}

using geos::geom::Coordinate;
using geos::geom::LinearRing;

namespace geos {      // geos.
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid


class GEOS_DLL PolygonRing {

private:

    const LinearRing* ring;
    int id;
    const PolygonRing* shell;

    /**
    * The root of the touch graph tree containing this ring.
    * Serves as the id for the graph partition induced by the touch relation.
    */
    const PolygonRing* touchSetRoot;

    /**
    * The parent of this ring in the touch tree graph.
    */
    const PolygonRing* touchTreeParent;

    /**
    * The set of PolygonRingTouch links
    * for this ring.
    * The set of all touches in the rings of a polygon
    * forms the polygon touch graph.
    * This supports detecting touch cycles, which
    * reveal the condition of a disconnected interior.
    *
    * Only a single touch is recorded between any two rings,
    * since more than one touch between two rings
    * indicates interior disconnection as well.
    */
    std::map<int, PolygonRingTouch> touches;

    /**
    * The set of self-nodes in this ring.
    * This supports checking valid ring self-touch topology.
    */
    std::vector<PolygonRingSelfNode> selfNodes;

    bool isInTouchSet();

    void setTouchSetRoot(const PolygonRing* ring);

    const PolygonRing* getTouchSetRoot();

    void setParent(const PolygonRing* ring);

    bool isChildOf(const PolygonRing* ring);

    bool hasTouches();

    const std::vector<PolygonRingTouch>& getTouches();

    void addTouch(PolygonRing* ring, const Coordinate* pt);

    /**
    * Tests if this ring touches a given ring at
    * the single point specified.
    *
    * @param ring the other PolygonRing
    * @param pt the touch point
    * @return true if the rings touch only at the given point
    */
    bool isOnlyTouch(const PolygonRing* ring, const Coordinate* pt) const;

    /**
    * Detects whether the subgraph of rings linked by touch to this ring
    * contains a touch cycle.
    * If no cycles are detected, the subgraph of touching rings is a tree.
    * The subgraph is marked using this ring as the root.
    *
    * @return a vertex in a ring cycle, or null if no cycle found
    */
    const Coordinate* findTouchCycleLocation();

    /**
    * Scans the rings touching a given ring,
    * and checks if they are already part of its ring subgraph set.
    * If so, a ring cycle has been detected.
    * Otherwise, each touched ring is added to the current subgraph set,
    * and queued to be scanned in turn.
    *
    * @param root the root of the touch subgraph
    * @param ring the ring being processed
    * @param ringStack the stack of rings to scan
    * @return a vertex in a ring cycle if found, or null
    */
    const Coordinate* scanForTouchCycle(
        const PolygonRing* root,
        const PolygonRing* ring,
        std::deque<PolygonRing*>& ringStack);


public:

    /**
    * Creates a ring for a polygon hole.
    * @param p_ring the ring geometry
    * @param p_index the index of the hole
    * @param p_shell the parent polygon shell
    */
    PolygonRing(const LinearRing* p_ring, int p_index, const PolygonRing* p_shell)
        : ring(p_ring)
        , id(p_index)
        , shell(p_shell)
        {};

    /**
    * Creates a ring for a polygon shell.
    * @param p_ring
    */
    PolygonRing(const LinearRing* p_ring)
        : PolygonRing(p_ring, -1, this)
        {};

    bool isSamePolygon(const PolygonRing* ring);

    bool isShell();

    void addSelfTouch(const Coordinate* origin,
        const Coordinate* e00, const Coordinate* e01,
        const Coordinate* e10, const Coordinate* e11);

    /**
    * Finds the location of an invalid interior self-touch in this ring,
    * if one exists.
    *
    * @return the location of an interior self-touch node, or null if there are none
    */
    const Coordinate* findInteriorSelfNode() const;

    String toString();


};



} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

