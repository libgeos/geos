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
#include <geos/algorithm/PolygonNodeTopology.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/LinearRing.h>
#include <geos/operation/valid/PolygonRing.h>

#include <stack>

using namespace geos::geom;

namespace geos {      // geos
namespace operation { // geos.operation
namespace valid {     // geos.operation.valid


/* public static */
bool
PolygonRing::isShell(const PolygonRing* polyRing)
{
    if (polyRing == nullptr) return true;
    return polyRing->isShell();
}


/* public static */
bool
PolygonRing::addTouch(PolygonRing* ring0, PolygonRing* ring1, const CoordinateXY& pt)
{
    //--- skip if either polygon does not have holes
    if (ring0 == nullptr || ring1 == nullptr)
      return false;

    //--- only record touches within a polygon
    if (! ring0->isSamePolygon(ring1)) return false;

    if (! ring0->isOnlyTouch(ring1, pt)) return true;
    if (! ring1->isOnlyTouch(ring0, pt)) return true;

    ring0->addTouch(ring1, pt);
    ring1->addTouch(ring0, pt);
    return false;
}


/* public static */
const CoordinateXY*
PolygonRing::findHoleCycleLocation(std::vector<PolygonRing*> polyRings)
{
    for (PolygonRing* polyRing : polyRings) {
        if (! polyRing->isInTouchSet()) {
            const CoordinateXY* holeCycleLoc = polyRing->findHoleCycleLocation();
            if (holeCycleLoc != nullptr) return holeCycleLoc;
        }
    }
    return nullptr;
}


/* public static */
const CoordinateXY*
PolygonRing::findInteriorSelfNode(std::vector<PolygonRing*> polyRings)
{
    for (PolygonRing* polyRing : polyRings) {
        const CoordinateXY* interiorSelfNode = polyRing->findInteriorSelfNode();
        if (interiorSelfNode != nullptr) {
            return interiorSelfNode;
        }
    }
    return nullptr;
}


/* private */
std::vector<PolygonRingTouch*>
PolygonRing::getTouches() const
{
    std::vector<PolygonRingTouch*> touchesVect;
    for (auto& mapEntry: touches) {
        PolygonRingTouch* touch = const_cast<PolygonRingTouch*>(&mapEntry.second);
        touchesVect.push_back(touch);
    }
    return touchesVect;
}


/* private */
void
PolygonRing::addTouch(PolygonRing* polyRing, const CoordinateXY& pt)
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
PolygonRing::addSelfTouch(const CoordinateXY& origin,
    const CoordinateXY* e00, const CoordinateXY* e01,
    const CoordinateXY* e10, const CoordinateXY* e11)
{
    selfNodes.emplace_back(origin, e00, e01, e10, e11);
}


/* private */
bool
PolygonRing::isOnlyTouch(const PolygonRing* polyRing, const CoordinateXY& pt) const
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
const CoordinateXY*
PolygonRing::findHoleCycleLocation()
{
    //--- the touch set including this ring is already processed
    if (isInTouchSet()) return nullptr;

    //--- scan the touch set tree rooted at this ring
    // Assert: this.touchSetRoot is null
    PolygonRing* root = this;
    root->setTouchSetRoot(root);

    if (! hasTouches())
        return nullptr;

    std::stack<PolygonRingTouch*> touchStack;
    init(root, touchStack);

    while (! touchStack.empty()) {
        PolygonRingTouch* touch = touchStack.top();
        touchStack.pop();
        const CoordinateXY* holeCyclePt = scanForHoleCycle(touch, root, touchStack);
        if (holeCyclePt != nullptr) {
            return holeCyclePt;
        }
    }
    return nullptr;
}


/* private static */
void
PolygonRing::init(PolygonRing* root, std::stack<PolygonRingTouch*>& touchStack)
{
    for (PolygonRingTouch* touch : root->getTouches()) {
        touch->getRing()->setTouchSetRoot(root);
        touchStack.push(touch);
    }
}


/* private */
const CoordinateXY*
PolygonRing::scanForHoleCycle(PolygonRingTouch* currentTouch,
    PolygonRing* root,
    std::stack<PolygonRingTouch*>& touchStack)
{
    PolygonRing* polyRing = currentTouch->getRing();
    const CoordinateXY* currentPt = currentTouch->getCoordinate();

    /**
     * Scan the touched rings
     * Either they form a hole cycle, or they are added to the touch set
     * and pushed on the stack for scanning
     */
    for (PolygonRingTouch* touch : polyRing->getTouches()) {
        /**
        * Don't check touches at the entry point
        * to avoid trivial cycles.
        * They will already be processed or on the stack
        * from the previous ring (which touched
        * all the rings at that point as well)
        */
        if (currentPt->equals2D(*touch->getCoordinate()))
            continue;

        /**
        * Test if the touched ring has already been
        * reached via a different touch path.
        * This is indicated by it already being marked as
        * part of the touch set.
        * This indicates a hole cycle has been found.
        */
        PolygonRing* touchRing = touch->getRing();
        if (touchRing->getTouchSetRoot() == root)
            return touch->getCoordinate();

        touchRing->setTouchSetRoot(root);

        touchStack.push(touch);
    }
    return nullptr;
}


/* public */
const CoordinateXY*
PolygonRing::findInteriorSelfNode()
{
    if (selfNodes.empty()) return nullptr;

    /**
     * Determine if the ring interior is on the Right.
     * This is the case if the ring is a shell and is CW,
     * or is a hole and is CCW.
     */
    bool isCCW = algorithm::Orientation::isCCW(ring->getCoordinatesRO());
    bool isInteriorOnRight = isShell() ^ isCCW;

    for (const PolygonRingSelfNode& selfNode : selfNodes) {
        if (!selfNode.isExterior(isInteriorOnRight)) {
            return selfNode.getCoordinate();
        }
    }
    return nullptr;
}



} // namespace geos.operation.valid
} // namespace geos.operation
} // namespace geos
