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

#include <geos/operation/overlayng/OverlayEdgeRing.h>
#include <geos/operation/overlayng/OverlayEdge.h>
#include <geos/geom/Location.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CircularString.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/CurvePolygon.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/util/CurveBuilder.h>
#include <geos/util/TopologyException.h>
#include <geos/algorithm/PointLocation.h>
#include <geos/algorithm/locate/PointOnGeometryLocator.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/algorithm/locate/SimplePointInAreaLocator.h>
#include <geos/algorithm/Orientation.h>
#include <geos/operation/polygonize/EdgeRing.h>

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

using namespace geos::geom;
using geos::operation::polygonize::EdgeRing;
using geos::algorithm::locate::PointOnGeometryLocator;
using geos::algorithm::locate::SimplePointInAreaLocator;

OverlayEdgeRing::OverlayEdgeRing(OverlayEdge* start, const GeometryFactory* geometryFactory)
    : startEdge(start)
    , ring(nullptr)
    , m_isHole(false)
    , locator(nullptr)
    , shell(nullptr)
{
    computeRing(start, geometryFactory);
}

/*public*/
std::unique_ptr<Curve>
OverlayEdgeRing::getRing()
{
    return std::move(ring);
}

const Curve*
OverlayEdgeRing::getRingPtr() const
{
    return ring.get();
}

const Envelope&
OverlayEdgeRing::getEnvelope() const
{
    return *ring->getEnvelopeInternal();
}

/**
* Tests whether this ring is a hole.
* @return <code>true</code> if this ring is a hole
*/
/*public*/
bool
OverlayEdgeRing::isHole() const
{
    return m_isHole;
}

/**
* Sets the containing shell ring of a ring that has been determined to be a hole.
*
* @param shell the shell ring
*/
/*public*/
void
OverlayEdgeRing::setShell(OverlayEdgeRing* p_shell)
{
    shell = p_shell;
    if (shell != nullptr)
        shell->addHole(this);
}

/**
* Tests whether this ring has a shell assigned to it.
*
* @return true if the ring has a shell
*/
/*public*/
bool
OverlayEdgeRing::hasShell() const
{
    return shell != nullptr;
}

/**
* Gets the shell for this ring.  The shell is the ring itself if it is not a hole, otherwise its parent shell.
*
* @return the shell for this ring
*/
/*public*/
const OverlayEdgeRing*
OverlayEdgeRing::getShell() const
{
    if (isHole())
        return shell;
    return this;
}

/*public*/
void
OverlayEdgeRing::addHole(OverlayEdgeRing* p_ring)
{
    holes.push_back(p_ring);
}

void
OverlayEdgeRing::closeRing(CoordinateSequence& pts)
{
    if(pts.size() > 0) {
        pts.closeRing(false);
    }
}

/*private*/
std::unique_ptr<Curve>
OverlayEdgeRing::computeRingGeometry(OverlayEdge* start, const GeometryFactory* gfact) const
{
    OverlayEdge* edge = start;

    const bool hasZ = start->getCoordinatesRO()->hasZ();
    const bool hasM = start->getCoordinatesRO()->hasM();

    geom::util::CurveBuilder cb(*gfact, hasZ, hasM);

    do {
        if (edge->getEdgeRing() == this)
            throw geos::util::TopologyException("Edge visited twice during ring-building", edge->getCoordinate());
            // only valid for polygonal output

        CoordinateSequence& pts = cb.getSeq(edge->isCurved());
        edge->addCoordinates(&pts);
        edge->setEdgeRing(this);

        if (edge->nextResult() == nullptr)
            throw geos::util::TopologyException("Found null edge in ring", edge->dest());

        edge = edge->nextResult();
    }
    while (edge != start);

    cb.closeRing();
    return cb.getGeometry();
}

/*private*/
void
OverlayEdgeRing::computeRing(OverlayEdge* start, const GeometryFactory* geometryFactory)
{
    if (ring != nullptr) return;   // don't compute more than once
    ring = computeRingGeometry(start, geometryFactory);

    if (ring->getGeometryTypeId() == GEOS_COMPOUNDCURVE) {
        const auto seq = ring->getCoordinates();
        m_isHole = algorithm::Orientation::isCCW(seq.get());
    } else {
        m_isHole = algorithm::Orientation::isCCW(static_cast<const SimpleCurve*>(ring.get())->getCoordinatesRO());
    }
}

/**
* Finds the innermost enclosing shell OverlayEdgeRing
* containing this OverlayEdgeRing, if any.
* The innermost enclosing ring is the smallest enclosing ring.
* The algorithm used depends on the fact that:
*  ring A contains ring B iff envelope(ring A) contains envelope(ring B)
*
* This routine is only safe to use if the chosen point of the hole
* is known to be properly contained in a shell
* (which is guaranteed to be the case if the hole does not touch its shell)
*
* To improve performance of this function the caller should
* make the passed shellList as small as possible (e.g.
* by using a spatial index filter beforehand).
*
* @return containing EdgeRing, if there is one
* or null if no containing EdgeRing is found
*/
/*public*/
OverlayEdgeRing*
OverlayEdgeRing::findEdgeRingContaining(const std::vector<OverlayEdgeRing*>& erList) const
{
    OverlayEdgeRing* minContainingRing = nullptr;
    for (OverlayEdgeRing* edgeRing : erList) {
        if (edgeRing->contains(*this)) {
            if (minContainingRing == nullptr
                || minContainingRing->getEnvelope().contains(edgeRing->getEnvelope())) {
                minContainingRing = edgeRing;
            }
        }
    }
    return minContainingRing;
}

// Adapter class to check whether a point lies within a ring.
// Unlike IndexedPointInAreaLocator, SimplePointInAreaLocator does not treat
// closed rings as areas.
class PointInCurvedRingLocator : public algorithm::locate::PointOnGeometryLocator {
public:
    explicit PointInCurvedRingLocator(const Curve& ring) : m_ring(ring) {}

    geom::Location locate(const geom::CoordinateXY* p) override {
        return algorithm::PointLocation::locateInRing(*p, m_ring);
    }

private:
    const Curve& m_ring;
};

/*private*/
PointOnGeometryLocator*
OverlayEdgeRing::getLocator() const
{
    if (locator == nullptr) {
        const Curve* p_ring = getRingPtr();
        if (p_ring->hasCurvedComponents()) {
            locator = detail::make_unique<PointInCurvedRingLocator>(*p_ring);
        } else {
            locator = detail::make_unique<IndexedPointInAreaLocator>(*p_ring);
        }

    }
    return locator.get();
}

/*public*/
geom::Location
OverlayEdgeRing::locate(const CoordinateXY& pt) const
{
    /**
    * Use an indexed point-in-polygon for performance
    */
    return getLocator()->locate(&pt);
}

/**
 * Tests if an edgeRing is properly contained in this ring.
 * Relies on property that edgeRings never overlap (although they may
 * touch at single vertices).
 * 
 * @param ring ring to test
 * @return true if ring is properly contained
 */
bool
OverlayEdgeRing::contains(const OverlayEdgeRing& otherRing) const {
    // the test envelope must be properly contained
    // (guards against testing rings against themselves)
    const Envelope& env = getEnvelope();
    const Envelope& testEnv = otherRing.getEnvelope();
    if (! env.containsProperly(testEnv)) {
        return false;
    }
    return isPointInOrOut(otherRing);
}

bool
OverlayEdgeRing::isPointInOrOut(const OverlayEdgeRing& otherRing) const {
    // in most cases only one or two points will be checked

    struct PointTester : public geom::CoordinateFilter {

    public:
        explicit PointTester(const OverlayEdgeRing* p_ring) : m_er(p_ring), m_result(false) {}

        void filter_ro(const CoordinateXY* pt) override {
            Location loc = m_er->locate(*pt);

            if (loc == geom::Location::INTERIOR) {
                m_done = true;
                m_result = true;
            } else if (loc == geom::Location::EXTERIOR) {
                m_done = true;
                m_result = false;
            }

            // pt is on BOUNDARY, so keep checking for a determining location
        }

        bool isDone() const override {
            return m_done;
        }

        bool getResult() const {
            return m_result;
        }

    private:
        const OverlayEdgeRing* m_er;
        bool m_done{false};
        bool m_result;
    };

    PointTester tester(this);
    otherRing.getRingPtr()->apply_ro(&tester);

    return tester.getResult();
}

/*public*/
const Coordinate&
OverlayEdgeRing::getCoordinate() const
{
    return *detail::down_cast<const Coordinate*>(ring->getCoordinate());
}

/**
* Computes the {@link Polygon} formed by this ring and any contained holes.
* @return the {@link Polygon} formed by this ring and its holes.
*/
/*public*/
std::unique_ptr<Surface>
OverlayEdgeRing::toSurface(const GeometryFactory* factory)
{
    if (holes.empty()) {
        return factory->createSurface(std::move(ring));
    }

    std::vector<std::unique_ptr<Curve>> holeCurves(holes.size());
    for (std::size_t i = 0; i < holes.size(); i++) {
        holeCurves[i] = holes[i]->getRing();
    }

    return factory->createSurface(std::move(ring), std::move(holeCurves));
}

/*public*/
OverlayEdge*
OverlayEdgeRing::getEdge()
{
    return startEdge;
}




} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
