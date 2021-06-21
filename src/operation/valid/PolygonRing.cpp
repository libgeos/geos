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

#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/operation/valid/PolygonRing.h>

using namespace geos::geom;

namespace geos {      // geos
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid

/* public static */
bool
PolygonRing::isShell(const PolygonRing* polyRing)
{
    if (polyRing == nullptr) return true;
    return polyRing.isShell();
}

/* public static */
bool
PolygonRing::addTouch(PolygonRing* ring0, PolygonRing* ring1, const Coordinate* pt)
{
    //--- skip if either polygon does not have holes
    if (ring0 == nullptr || ring1 == nullptr)
        return false;

    //--- only record touches within a polygon
    if (! ring0.isSamePolygon(ring1)) return false;

    if (! ring0.isOnlyTouch(ring1, pt)) return true;
    if (! ring1.isOnlyTouch(ring0, pt)) return true;

    ring0.addTouch(ring1, pt);
    ring1.addTouch(ring0, pt);
    return false;
}

/* public static */
const Coordinate*
PolygonRing::findTouchCycleLocation(std::vector<PolygonRing*>& polyRings)
{
    for (const PolygonRing* polyRing : polyRings)
    {
        if (! polyRing->isInTouchSet()) {
            const Coordinate* touchCycleLoc = polyRing->findTouchCycleLocation();
            if (touchCycleLoc != nullptr)
                return touchCycleLoc;
        }
    }
    return nullptr;
}

/* public static */
const Coordinate*
PolygonRing::findInteriorSelfNode(std::vector<PolygonRing*>& polyRings)
{
    for (const PolygonRing* polyRing : polyRings) {
        const Coordinate* interiorSelfNode = polyRing->findInteriorSelfNode();
        if (interiorSelfNode != nullptr) {
            return interiorSelfNode;
        }
    }
    return nullptr;
}

/* public */
bool
PolygonRing::isSamePolygon(const PolygonRing* polyRing)
{
    return shell == polyRing->shell;
}

/* public */
bool
PolygonRing::isShell()
{
    return shell == this;
}

/* private */
bool
PolygonRing::isInTouchSet()
{
    return touchSetRoot != nullptr;
}

/* private */
void
PolygonRing::setTouchSetRoot(const PolygonRing* polyRing)
{
    touchSetRoot = polyRing;
}

/* private */
const PolygonRing*
PolygonRing::getTouchSetRoot()
{
    return touchSetRoot;
}

/* private */
void
PolygonRing::setParent(const PolygonRing* polyRing)
{
    touchTreeParent = polyRing;
}

/* private */
bool
PolygonRing::isChildOf(const PolygonRing* polyRing)
{
    return touchTreeParent == polyRing;
}


/* private */
bool
PolygonRing::hasTouches()
{
    return touches != nullptr && ! touches->isEmpty();
}

/* private */
const std::vector<PolygonRingTouch>&
PolygonRing::getTouches()
{
    return touches;
}


/* private */
void
PolygonRing::addTouch(PolygonRing* polyRing, const Coordinate* pt)
{
    std::size_t nTouches = touches.count(polyRing->id);
    if (nTouches == 0) {
        // uses pair's piecewise constructor to emplace into
        // std::map<int, PolygonRingTouch> touches;
        touches.emplace(std::piecewise_construct,
              std::forward_as_tuple(polyRing->id),
              std::forward_as_tuple(polyRing, pt));
    };
}


/* public */
void
PolygonRing::addSelfTouch(const Coordinate* origin,
    const Coordinate* e00, const Coordinate* e01,
    const Coordinate* e10, const Coordinate* e11)
{
    selfNodes.emplace_back(origin, e00, e01, e10, e11);
}


/* private */
bool
PolygonRing::isOnlyTouch(const PolygonRing* polyRing, const Coordinate* pt) const
{
    //--- no touches for this ring
    if (touches.empty()) return true;
    //--- no touches for other ring

    // std::map.find() returns std::pair
    auto search = touches.find(polyRing->id);
    // Not found
    if (search == touches.end()) return true;
    // Found
    const PolygonRingTouch& touch = search->second;
    //--- the rings touch - check if point is the same
    return touch.isAtLocation(pt);
}


/* private */
const Coordinate*
PolygonRing::findTouchCycleLocation()
{
    //--- the touch set including this ring is already processed
    if (isInTouchSet()) return nullptr;

    //--- scan the touch set tree rooted at this ring
    // Assert: this.touchSetRoot is null
    PolygonRing* root = this;
    root->setParent(root);
    root->setTouchSetRoot(root);

    std::deque<PolygonRing*> ringStack;
    ringStack.push_back(root);

    while (! ringStack.empty()) {
        PolygonRing* polyRing = ringStack.front();
        ringStack.pop_front();
        const Coordinate* touchCyclePt = scanForTouchCycle(root, polyRing, ringStack);
        if (touchCyclePt != nullptr)
            return touchCyclePt;
    }
    return nullptr;
}


/* private */
const Coordinate*
PolygonRing::scanForTouchCycle(const PolygonRing* root,
    const PolygonRing* polyRing,
    std::dequeue<PolygonRing*>& ringStack)
{
    if (! polyRing->hasTouches())
        return nullptr;

    //-- check the touched rings
    //--- either they form a touch cycle, or they are pushed on stack for processing
    for (const PolygonRingTouch& touch : polyRing->getTouches()) {
        PolygonRing* touchRing = touch.getRing();
        /**
        * There is always a link back to the touch-tree parent of the ring,
        * so don't include it.
        * (I.e. the ring touches the parent ring which originally
        * added this ring to the stack)
        */
        if (polyRing->isChildOf(touchRing))
            continue;

        /**
        * Test if the touched ring has already been
        * reached via a different path in the tree.
        * This indicates a touching ring cycle has been found.
        * This is invalid.
        */
        if (touchRing->getTouchSetRoot() == root) {
            return touch.getCoordinate();
        }

        touchRing->setParent(polyRing);
        touchRing->setTouchSetRoot(root);
        ringStack.push_back(touchRing);
    }
    return nullptr;
}


/* public */
const Coordinate*
PolygonRing::findInteriorSelfNode() const
{
    if (selfNodes.empty()) return nullptr;

    /**
     * Determine if the ring interior is on the Right.
     * This is the case if the ring is a shell and is CW,
     * or is a hole and is CCW.
     */
    bool isCCW = Orientation::isCCW(ring->getCoordinatesRO());
    bool isInteriorOnRight = isShell() ^ isCCW;

    for (const PolygonRingSelfNode& selfNode : selfNodes) {
        if (!selfNode.isExterior(isInteriorOnRight)) {
            return selfNode.getCoordinate();
        }
    }
    return nullptr;
}


/* public */
String
PolygonRing::toString()
{
    return ring->toString();
}


} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos
