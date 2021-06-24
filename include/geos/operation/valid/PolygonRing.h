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
#include <map>

// Forward declarations
namespace geos {
namespace geom {
class Coordinate;
class LinearRing;
}
}

namespace geos {      // geos.
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

using geos::geom::Coordinate;
using geos::geom::LinearRing;


class GEOS_DLL PolygonRing {

private:

    const LinearRing* ring = nullptr;
    int id = -1;
    const PolygonRing* shell = nullptr;

    /**
    * The root of the touch graph tree containing this ring.
    * Serves as the id for the graph partition induced by the touch relation.
    */
    const PolygonRing* touchSetRoot = nullptr;

    /**
    * The parent of this ring in the touch tree graph.
    */
    const PolygonRing* touchTreeParent = nullptr;

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

    bool isInTouchSet() const;

    void setTouchSetRoot(const PolygonRing* ring);

    const PolygonRing* getTouchSetRoot() const;

    void setParent(const PolygonRing* ring);

    bool isChildOf(const PolygonRing* ring) const;

    bool hasTouches() const;

    void addTouch(PolygonRing* ring, const Coordinate& pt);

    /**
    * Tests if this ring touches a given ring at
    * the single point specified.
    *
    * @param ring the other PolygonRing
    * @param pt the touch point
    * @return true if the rings touch only at the given point
    */
    bool isOnlyTouch(const PolygonRing* ring, const Coordinate& pt) const;

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
        PolygonRing* root,
        PolygonRing* ring,
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

    static bool isShell(const PolygonRing* polyRing);

    static bool addTouch(PolygonRing* ring0, PolygonRing* ring1,
        const Coordinate& pt);

    static const Coordinate* findTouchCycleLocation(
        std::vector<PolygonRing*>& polyRings);

    static const Coordinate* findInteriorSelfNode(
        std::vector<PolygonRing*>& polyRings);

    void addSelfTouch(const Coordinate& origin,
        const Coordinate* e00, const Coordinate* e01,
        const Coordinate* e10, const Coordinate* e11);

    bool isSamePolygon(const PolygonRing* ring) const;

    bool isShell() const;
    /**
    * Finds the location of an invalid interior self-touch in this ring,
    * if one exists.
    *
    * @return the location of an interior self-touch node, or null if there are none
    */
    const Coordinate* findInteriorSelfNode() const;


    // std::string toString();


};



} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

