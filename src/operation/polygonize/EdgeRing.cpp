/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2005-2006 Refractions Research Inc.
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/polygonize/EdgeRing.java 0b3c7e3eb0d3e
 *
 **********************************************************************/

#include <geos/operation/polygonize/EdgeRing.h>
#include <geos/operation/polygonize/PolygonizeEdge.h>
#include <geos/planargraph/DirectedEdge.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/util/CurveBuilder.h>
#include <geos/algorithm/PointLocation.h>
#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/algorithm/locate/SimplePointInAreaLocator.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h> // TODO: drop this, includes too much
#include <geos/geom/Location.h>

#include <vector>
#include <cassert>

//#define DEBUG_ALLOC 1
//#define GEOS_PARANOIA_LEVEL 2


using namespace geos::planargraph;
using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace polygonize { // geos.operation.polygonize

/*public*/
EdgeRing*
EdgeRing::findEdgeRingContaining(const std::vector<EdgeRing*> & erList) const
{
    EdgeRing* minContainingRing = nullptr;
    for (auto& edgeRing : erList) {
        if (edgeRing->contains(*this)) {
            if (minContainingRing == nullptr
                || minContainingRing->getEnvelope().contains(edgeRing->getEnvelope())) {
                minContainingRing = edgeRing;
            }
        }
    }
    return minContainingRing;
}

std::vector<PolygonizeDirectedEdge*>
EdgeRing::findDirEdgesInRing(PolygonizeDirectedEdge* startDE) {
    auto de = startDE;
    std::vector<decltype(de)> edges;

    do {
        edges.push_back(de);
        de = de->getNext();
    } while (de != startDE);

    return edges;
}

/*public static*/
const CoordinateXY&
EdgeRing::ptNotInList(const CoordinateSequence* testPts,
                      const CoordinateSequence* pts)
{
    const std::size_t npts = testPts->getSize();
    for(std::size_t i = 0; i < npts; ++i) {
        const CoordinateXY& testPt = testPts->getAt<CoordinateXY>(i);
        if(!isInList(testPt, pts)) {
            return testPt;
        }
    }
    return Coordinate::getNull();
}

/*public static*/
bool
EdgeRing::isInList(const CoordinateXY& pt,
                   const CoordinateSequence* pts)
{
    const std::size_t npts = pts->getSize();
    for(std::size_t i = 0; i < npts; ++i) {
        if(pt == pts->getAt<CoordinateXY>(i)) {
            return true;
        }
    }
    return false;
}

/*public*/
EdgeRing::EdgeRing(const GeometryFactory* newFactory)
    :
    factory(newFactory),
    ring(nullptr),
    is_hole(false)
{
#ifdef DEBUG_ALLOC
    std::cerr << "[" << this << "] EdgeRing(factory)" << std::endl;
#endif // DEBUG_ALLOC
}

void
EdgeRing::build(PolygonizeDirectedEdge* startDE) {
    auto de = startDE;
    do {
        add(de);
        de->setRing(this);
        de = de->getNext();
    } while (de != startDE);
}

/*public*/
void
EdgeRing::add(const PolygonizeDirectedEdge* de)
{
    deList.push_back(de);
}

/*public*/
void
EdgeRing::computeHole()
{
    getRingInternal();

    if (ring->getGeometryTypeId() == GEOS_COMPOUNDCURVE) {
        auto coords = ring->getCoordinates();
        is_hole = Orientation::isCCW(coords.get());
    } else {
        const SimpleCurve* sc = detail::down_cast<SimpleCurve*>(ring.get());
        is_hole = Orientation::isCCW(sc->getCoordinatesRO());
    }
}

/*public*/
void
EdgeRing::addHole(std::unique_ptr<Curve> hole)
{
    holes.push_back(std::move(hole));
}

void
EdgeRing::addHole(EdgeRing* holeER) {
    holeER->setShell(this);
    auto hole = holeER->getRingOwnership(); // TODO is this right method?
    addHole(std::move(hole));
}

/*public*/
std::unique_ptr<Surface>
EdgeRing::getPolygon()
{
    if (!holes.empty()) {
        return factory->createSurface(std::move(ring), std::move(holes));
    } else {
        return factory->createSurface(std::move(ring));
    }
}

// Adapter class to check whether a point lies within a ring.
// Unlike IndexedPointInAreaLocator, SimplePointInAreaLocator does not treat
// closed rings as areas.
class PointInCurvedRingLocator : public algorithm::locate::PointOnGeometryLocator {
    public:
        explicit PointInCurvedRingLocator(const Curve& ring) : m_ring(ring) {}

        geom::Location locate(const geom::CoordinateXY* p) override {
            return PointLocation::locateInRing(*p, m_ring);
        }

    private:
        const Curve& m_ring;
};

algorithm::locate::PointOnGeometryLocator*
EdgeRing::getLocator() const
{
    if (ringLocator == nullptr) {
        const auto* rng = getRingInternal();
        if (rng->hasCurvedComponents()) {
            ringLocator = std::make_unique<PointInCurvedRingLocator>(*rng);
        } else {
            ringLocator = std::make_unique<algorithm::locate::IndexedPointInAreaLocator>(*rng);
        }
    }
    return ringLocator.get();
}

/*public*/
bool
EdgeRing::isValid() const
{
    return is_valid;
}

void
EdgeRing::computeValid()
{
    const Curve* r = getRingInternal();

    if (r->getNumPoints() <= 3) {
        is_valid = false;
        return;
    }

    if (r->getGeometryTypeId() == GEOS_LINEARRING) {
        is_valid = getRingInternal()->isValid();
    } else {
        // TODO: Change once IsValidOp supports curves
        is_valid = true;
    }
}

bool
EdgeRing::contains(const EdgeRing& otherRing) const {
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
EdgeRing::isPointInOrOut(const EdgeRing& otherRing) const {
    // in most cases only one or two points will be checked

    struct PointTester : public CoordinateFilter {

    public:
        explicit PointTester(const EdgeRing* p_ring) : m_er(p_ring), m_result(false) {}

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
        const EdgeRing* m_er;
        bool m_done{false};
        bool m_result;
    };

    PointTester tester(this);
    otherRing.getRingInternal()->apply_ro(&tester);

    return tester.getResult();
}

/*private*/
const Curve*
EdgeRing::getRingInternal() const
{
    if (ring == nullptr) {
        bool hasZ = false;
        bool hasM = false;

        for(const auto& de : deList) {
            const auto edge = detail::down_cast<PolygonizeEdge*>(de->getEdge());
            hasZ |= edge->getLine()->hasZ();
            hasM |= edge->getLine()->hasM();
        }

        geom::util::CurveBuilder builder(*factory, hasZ, hasM);

        for(const auto& de : deList) {
            const auto edge = detail::down_cast<PolygonizeEdge*>(de->getEdge());
            const bool isCurved = edge->getLine()->getGeometryTypeId() == GEOS_CIRCULARSTRING;

            CoordinateSequence& ringPts = builder.getSeq(isCurved);

            addEdge(edge->getLine()->getCoordinatesRO(),
                    de->getEdgeDirection(), &ringPts);
        }

        ring = builder.getGeometry();
    }

    return ring.get();
}

/*public*/
std::unique_ptr<Curve>
EdgeRing::getRingOwnership()
{
    getRingInternal(); // active lazy generation
    return std::move(ring);
}

/*private*/
void
EdgeRing::addEdge(const CoordinateSequence* srcCoords, bool isForward,
                  CoordinateSequence* dstCoords)
{
    const std::size_t npts = srcCoords->getSize();

    if (!dstCoords->isEmpty() && !srcCoords->isEmpty()) {
        // Patch Z value in last coordinate, if needed
        if (srcCoords->hasZ() && std::isnan(dstCoords->getZ(dstCoords->size() - 1))) {
            dstCoords->setZ(dstCoords->size() - 1, isForward ? srcCoords->getZ(0) : srcCoords->getZ(srcCoords->getSize() - 1));
        }
        // Patch M value in last coordinate, if needed
        if (srcCoords->hasM() && std::isnan(dstCoords->getM(dstCoords->size() - 1))) {
            dstCoords->setM(dstCoords->size() - 1, isForward ? srcCoords->getM(0) : srcCoords->getM(srcCoords->getSize() - 1));
        }
    }

    if(isForward) {
        dstCoords->add(*srcCoords, 0, npts - 1, false);
    }
    else {
        for(std::size_t i = npts; i > 0; --i) {
            srcCoords->applyAt(i-1, [&dstCoords](const auto& coord) {
                dstCoords->add(coord, false);
            });
        }
    }
}

EdgeRing*
EdgeRing::getOuterHole() const {
    // Only shells can have outer holes
    if (isHole()) {
        return nullptr;
    }

    // A shell is an outer shell if any edge is also in an outer hole.
    // A hole is an outer shell if it is not contained by a shell.
    for (auto& de : deList) {
        auto adjRing = (dynamic_cast<PolygonizeDirectedEdge*>(de->getSym()))->getRing();
        if (adjRing->isOuterHole()) {
            return adjRing;
        }
    }

    return nullptr;
}

void
EdgeRing::updateIncludedRecursive() {
    visitedByUpdateIncludedRecursive = true;

    if (isHole()) {
        return;
    }

    for (const auto& de : deList) {
        auto adjShell = (dynamic_cast<const PolygonizeDirectedEdge*>(de->getSym()))->getRing()->getShell();

        if (adjShell != nullptr) {
            if (!adjShell->isIncludedSet() && !adjShell->visitedByUpdateIncludedRecursive) {
                adjShell->updateIncludedRecursive();
            }
        }
    }

    for (const auto& de : deList) {
        auto adjShell = (dynamic_cast<const PolygonizeDirectedEdge*>(de->getSym()))->getRing()->getShell();

        if (adjShell != nullptr) {
            if (adjShell->isIncludedSet()) {
                setIncluded(!adjShell->isIncluded());
                return;
            }
        }
    }

}

} // namespace geos.operation.polygonize
} // namespace geos.operation
} // namespace geos
