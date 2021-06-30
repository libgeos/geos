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

    int id = -1;
    PolygonRing* shell = nullptr;
    const LinearRing* ring = nullptr;

    /**
    * The root of the touch graph tree containing this ring.
    * Serves as the id for the graph partition induced by the touch relation.
    */
    PolygonRing* touchSetRoot = nullptr;

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

    /* METHODS */

    /**
    * Tests if this ring touches a given ring at
    * the single point specified.
    *
    * @param ring the other PolygonRing
    * @param pt the touch point
    * @return true if the rings touch only at the given point
    */
    bool isOnlyTouch(const PolygonRing* polyRing, const Coordinate& pt) const;

    /**
    * Detects whether the subgraph of holes linked by touch to this ring
    * contains a hole cycle.
    * If no cycles are detected, the set of touching rings is a tree.
    * The set is marked using this ring as the root.
    *
    * @return a vertex in a hole cycle, or null if no cycle found
    */
    const Coordinate* findHoleCycleLocation();

    void init(PolygonRing* root, std::stack<PolygonRingTouch*>& touchStack);

    /**
    * Scans for a hole cycle starting at a given touch.
    *
    * @param currentTouch the touch to investigate
    * @param root the root of the touch subgraph
    * @param touchStack the stack of touches to scan
    * @return a vertex in a hole cycle if found, or null
    */
    const Coordinate* scanForHoleCycle(PolygonRingTouch* currentTouch,
        PolygonRing* root,
        std::stack<PolygonRingTouch*>& touchStack);


    bool isInTouchSet() const
    {
        return touchSetRoot != nullptr;
    };

    void setTouchSetRoot(PolygonRing* polyRing)
    {
        touchSetRoot = polyRing;
    };

    PolygonRing* getTouchSetRoot() const
    {
        return touchSetRoot;
    };

    bool hasTouches() const
    {
        return ! touches.empty();
    };

    std::vector<PolygonRingTouch*> getTouches() const;

    void addTouch(PolygonRing* polyRing, const Coordinate& pt);


public:

    /**
    * Creates a ring for a polygon hole.
    * @param p_ring the ring geometry
    * @param p_index the index of the hole
    * @param p_shell the parent polygon shell
    */
    PolygonRing(const LinearRing* p_ring, int p_index, PolygonRing* p_shell)
        : id(p_index)
        , shell(p_shell)
        , ring(p_ring)
        {};

    /**
    * Creates a ring for a polygon shell.
    * @param p_ring
    */
    PolygonRing(const LinearRing* p_ring)
        : PolygonRing(p_ring, -1, this)
        {};

    /**
    * Tests if a polygon ring represents a shell.
    *
    * @param polyRing the ring to test (may be null)
    * @return true if the ring represents a shell
    */
    static bool isShell(const PolygonRing* polyRing);

    /**
    * Records a touch location between two rings,
    * and checks if the rings already touch in a different location.
    *
    * @param ring0 a polygon ring
    * @param ring1 a polygon ring
    * @param pt the location where they touch
    * @return true if the polygons already touch
    */
    static bool addTouch(PolygonRing* ring0, PolygonRing* ring1, const Coordinate& pt);

    /**
    * Finds a location (if any) where a chain of holes forms a cycle
    * in the ring touch graph.
    * The shell may form part of the chain as well.
    * This indicates that a set of holes disconnects the interior of a polygon.
    *
    * @param polyRings the list of rings to check
    * @return a vertex contained in a ring cycle, or null if none is found
    */
    static const Coordinate* findHoleCycleLocation(std::vector<PolygonRing*> polyRings);

    /**
    * Finds a location of an interior self-touch in a list of rings,
    * if one exists.
    * This indicates that a self-touch disconnects the interior of a polygon,
    * which is invalid.
    *
    * @param polyRings the list of rings to check
    * @return the location of an interior self-touch node, or null if there are none
    */
    static const Coordinate* findInteriorSelfNode(std::vector<PolygonRing*> polyRings);

    bool isSamePolygon(const PolygonRing* polyRing) const
    {
        return shell == polyRing->shell;
    };

    bool isShell() const
    {
        return shell == this;
    };

    void addSelfTouch(const Coordinate& origin,
        const Coordinate* e00, const Coordinate* e01,
        const Coordinate* e10, const Coordinate* e11);

    /**
    * Finds the location of an invalid interior self-touch in this ring,
    * if one exists.
    *
    * @return the location of an interior self-touch node, or null if there are none
    */
    const Coordinate* findInteriorSelfNode();


};



} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos

