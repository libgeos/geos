/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/BoundaryNodeRule.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Location.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/util/GeometryLister.h>
#include <geos/noding/MCIndexSegmentSetMutualIntersector.h>
#include <geos/noding/SegmentString.h>
#include <geos/operation/relateng/DimensionLocation.h>
#include <geos/operation/relateng/EdgeSegmentIntersector.h>
#include <geos/operation/relateng/EdgeSetIntersector.h>
#include <geos/operation/relateng/RelateGeometry.h>
#include <geos/operation/relateng/RelateMatrixPredicate.h>
#include <geos/operation/relateng/RelateNG.h>
#include <geos/operation/relateng/RelatePredicate.h>
#include <geos/operation/relateng/RelateSegmentString.h>
#include <geos/operation/relateng/TopologyComputer.h>
#include <geos/operation/relateng/TopologyPredicate.h>

#include <sstream>


using namespace geos::geom;
using geos::algorithm::BoundaryNodeRule;
using geos::noding::MCIndexSegmentSetMutualIntersector;
using geos::noding::SegmentString;
using geos::geom::util::GeometryLister;

namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


#define GEOM_A RelateGeometry::GEOM_A
#define GEOM_B RelateGeometry::GEOM_B


/************************************************************************/

/* public static */
bool
RelateNG::intersects(const Geometry* a, const Geometry* b)
{
    RelatePredicate::IntersectsPredicate pred;
    return RelateNG::relate(a, b, pred);
}

/* public static */
bool
RelateNG::crosses(const Geometry* a, const Geometry* b)
{
    RelatePredicate::CrossesPredicate pred;
    return RelateNG::relate(a, b, pred);
}

/* public static */
bool
RelateNG::disjoint(const Geometry* a, const Geometry* b)
{
    RelatePredicate::DisjointPredicate pred;
    return RelateNG::relate(a, b, pred);
}

/* public static */
bool
RelateNG::touches(const Geometry* a, const Geometry* b)
{
    RelatePredicate::TouchesPredicate pred;
    return RelateNG::relate(a, b, pred);
}

/* public static */
bool
RelateNG::within(const Geometry* a, const Geometry* b)
{
    RelatePredicate::WithinPredicate pred;
    return RelateNG::relate(a, b, pred);
}

/* public static */
bool
RelateNG::contains(const Geometry* a, const Geometry* b)
{
    RelatePredicate::ContainsPredicate pred;
    return RelateNG::relate(a, b, pred);
}

/* public static */
bool
RelateNG::overlaps(const Geometry* a, const Geometry* b)
{
    RelatePredicate::OverlapsPredicate pred;
    return RelateNG::relate(a, b, pred);
}

/* public static */
bool
RelateNG::covers(const Geometry* a, const Geometry* b)
{
    RelatePredicate::CoversPredicate pred;
    return RelateNG::relate(a, b, pred);
}

/* public static */
bool
RelateNG::coveredBy(const Geometry* a, const Geometry* b)
{
    RelatePredicate::CoveredByPredicate pred;
    return RelateNG::relate(a, b, pred);
}

/* public static */
bool
RelateNG::equalsTopo(const Geometry* a, const Geometry* b)
{
    RelatePredicate::EqualsTopoPredicate pred;
    return RelateNG::relate(a, b, pred);
}

/* public static */
bool
RelateNG::relate(const Geometry* a, const Geometry* b, TopologyPredicate& pred)
{
    RelateNG rng(a, false);
    return rng.evaluate(b, pred);
}

/* public static */
bool
RelateNG::relate(const Geometry* a, const Geometry* b, TopologyPredicate& pred, const BoundaryNodeRule& bnRule)
{
    RelateNG rng(a, false, bnRule);
    return rng.evaluate(b, pred);
}

/* public static */
bool
RelateNG::relate(const Geometry* a, const Geometry* b, const std::string& imPattern)
{
    RelateNG rng(a, false);
    return rng.evaluate(b, imPattern);
}

/* public static */
std::unique_ptr<IntersectionMatrix>
RelateNG::relate(const Geometry* a, const Geometry* b)
{
    RelateNG rng(a, false);
    return rng.evaluate(b);
}

/* public static */
std::unique_ptr<IntersectionMatrix>
RelateNG::relate(const Geometry* a, const Geometry* b, const BoundaryNodeRule& bnRule)
{
    RelateNG rng(a, false, bnRule);
    return rng.evaluate(b);
}

/************************************************************************/

/* public */
bool
RelateNG::intersects(const Geometry* b)
{
    RelatePredicate::IntersectsPredicate pred;
    return evaluate(b, pred);
}

/* public */
bool
RelateNG::crosses(const Geometry* b)
{
    RelatePredicate::CrossesPredicate pred;
    return evaluate(b, pred);
}

/* public */
bool
RelateNG::disjoint(const Geometry* b)
{
    RelatePredicate::DisjointPredicate pred;
    return evaluate(b, pred);
}

/* public */
bool
RelateNG::touches(const Geometry* b)
{
    RelatePredicate::TouchesPredicate pred;
    return evaluate(b, pred);
}

/* public */
bool
RelateNG::within(const Geometry* a)
{
    RelatePredicate::WithinPredicate pred;
    return evaluate(a, pred);
}

/* public */
bool
RelateNG::contains(const Geometry* b)
{
    RelatePredicate::ContainsPredicate pred;
    return evaluate(b, pred);
}

/* public */
bool
RelateNG::overlaps(const Geometry* b)
{
    RelatePredicate::OverlapsPredicate pred;
    return evaluate(b, pred);
}

/* public */
bool
RelateNG::covers(const Geometry* b)
{
    RelatePredicate::CoversPredicate pred;
    return evaluate(b, pred);
}

/* public */
bool
RelateNG::coveredBy(const Geometry* b)
{
    RelatePredicate::CoveredByPredicate pred;
    return evaluate(b, pred);
}

/* public */
bool
RelateNG::equalsTopo(const Geometry* b)
{
    RelatePredicate::EqualsTopoPredicate pred;
    return evaluate(b, pred);
}

/* public */
bool
RelateNG::relate(const Geometry* b, const std::string& imPattern)
{
    return evaluate(b, imPattern);
}

/* public */
std::unique_ptr<IntersectionMatrix>
RelateNG::relate(const Geometry* b)
{
    return evaluate(b);
}

/************************************************************************/

/* public static */
std::unique_ptr<RelateNG>
RelateNG::prepare(const Geometry* a)
{
    return std::unique_ptr<RelateNG>(new RelateNG(a, true));
}


/* public static */
std::unique_ptr<RelateNG>
RelateNG::prepare(const Geometry* a, const BoundaryNodeRule& bnRule)
{
    return std::unique_ptr<RelateNG>(new RelateNG(a, true, bnRule));
}

/************************************************************************/

/* public */
std::unique_ptr<IntersectionMatrix>
RelateNG::evaluate(const Geometry* b)
{
    RelateMatrixPredicate rel;
    evaluate(b, rel);
    return rel.getIM();
}


/* public */
bool
RelateNG::evaluate(const Geometry* b, const std::string& imPattern)
{
    auto predicate = RelatePredicate::matches(imPattern);
    return evaluate(b, *predicate);
}


/* public */
bool
RelateNG::evaluate(const Geometry* b, TopologyPredicate& predicate)
{
    //-- fast envelope checks
    if (! hasRequiredEnvelopeInteraction(b, predicate)) {
        return false;
    }

    geos::util::ensureNoCurvedComponents(geomA.getGeometry());
    geos::util::ensureNoCurvedComponents(b);
    
    RelateGeometry geomB(b, boundaryNodeRule);

    int dimA = geomA.getDimensionReal();
    int dimB = geomB.getDimensionReal();

    //-- check if predicate is determined by dimension or envelope
    predicate.init(dimA, dimB);
    if (predicate.isKnown())
        return finishValue(predicate);

    predicate.init(*(geomA.getEnvelope()), *(geomB.getEnvelope()));
    if (predicate.isKnown())
        return finishValue(predicate);

    TopologyComputer topoComputer(predicate, geomA, geomB);

    //-- optimized P/P evaluation
    if (dimA == Dimension::P && dimB == Dimension::P) {
        computePP(geomB, topoComputer);
        topoComputer.finish();
        return topoComputer.getResult();
    }

    //-- test points against (potentially) indexed geometry first
    computeAtPoints(geomB, GEOM_B, geomA, topoComputer);
    if (topoComputer.isResultKnown()) {
        return topoComputer.getResult();
    }
    computeAtPoints(geomA, GEOM_A, geomB, topoComputer);
    if (topoComputer.isResultKnown()) {
        return topoComputer.getResult();
    }

    if (geomA.hasEdges() && geomB.hasEdges()) {
        computeAtEdges(geomB, topoComputer);
    }

    //-- after all processing, set remaining unknown values in IM
    topoComputer.finish();
    return topoComputer.getResult();
}


/* private */
bool
RelateNG::hasRequiredEnvelopeInteraction(const Geometry* b, TopologyPredicate& predicate)
{
    const Envelope* envB = b->getEnvelopeInternal();
    bool isInteracts = false;
    if (predicate.requireCovers(GEOM_A)) {
        if (! geomA.getEnvelope()->covers(envB)) {
            return false;
        }
        isInteracts = true;
    }
    else if (predicate.requireCovers(GEOM_B)) {
        if (! envB->covers(geomA.getEnvelope())) {
            return false;
        }
        isInteracts = true;
    }
    if (! isInteracts
        && predicate.requireInteraction()
        && ! geomA.getEnvelope()->intersects(envB)) {
        return false;
    }
    return true;
}

/* private */
bool
RelateNG::finishValue(TopologyPredicate& predicate)
{
    predicate.finish();
    return predicate.value();
}


/* private */
void
RelateNG::computePP(RelateGeometry& geomB, TopologyComputer& topoComputer)
{
    Coordinate::ConstXYSet& ptsA = geomA.getUniquePoints();
    //TODO: only query points in interaction extent?
    Coordinate::ConstXYSet& ptsB = geomB.getUniquePoints();

    uint32_t numBinA = 0;
    for (const CoordinateXY* ptB : ptsB) {
        auto it = ptsA.find(ptB);
        bool found = (it != ptsA.end());
        if (found) {
            numBinA++;
            topoComputer.addPointOnPointInterior(ptB);
        }
        else {
            topoComputer.addPointOnPointExterior(GEOM_B, ptB);
        }
        if (topoComputer.isResultKnown()) {
            return;
        }
    }
    /**
     * If number of matched B points is less than size of A,
     * there must be at least one A point in the exterior of B
     */
    if (numBinA < ptsA.size()) {
        //TODO: determine actual exterior point?
        topoComputer.addPointOnPointExterior(GEOM_A, nullptr);
    }
}


/* private */
void
RelateNG::computeAtPoints(
    RelateGeometry& geom, bool isA,
    RelateGeometry& geomTarget, TopologyComputer& topoComputer)
{
    bool isResultKnown = false;
    isResultKnown = computePoints(geom, isA, geomTarget, topoComputer);
    if (isResultKnown)
        return;

    /**
     * Performance optimization: only check points against target
     * if it has areas OR if the predicate requires checking for
     * exterior interaction.
     * In particular, this avoids testing line ends against lines
     * for the intersects predicate (since these are checked
     * during segment/segment intersection checking anyway).
     * Checking points against areas is necessary, since the input
     * linework is disjoint if one input lies wholly inside an area,
     * so segment intersection checking is not sufficient.
     */
    bool checkDisjointPoints = geomTarget.hasDimension(Dimension::A)
                                || topoComputer.isExteriorCheckRequired(isA);
    if (! checkDisjointPoints)
        return;

    isResultKnown = computeLineEnds(geom, isA, geomTarget, topoComputer);
    if (isResultKnown)
        return;

    computeAreaVertex(geom, isA, geomTarget, topoComputer);
}


/* private */
bool
RelateNG::computePoints(
    RelateGeometry& geom, bool isA,
    RelateGeometry& geomTarget, TopologyComputer& topoComputer)
{
    if (! geom.hasDimension(Dimension::P)) {
        return false;
    }

    std::vector<const Point*> points = geom.getEffectivePoints();
    for (const Point* point : points) {
        //TODO: exit when all possible target locations (E,I,B) have been found?
        if (point->isEmpty())
            continue;

        const CoordinateXY* pt = point->getCoordinate();
        computePoint(isA, pt, geomTarget, topoComputer);
        if (topoComputer.isResultKnown()) {
            return true;
        }
    }
    return false;
}


/* private */
void
RelateNG::computePoint(bool isA, const CoordinateXY* pt, RelateGeometry& geomTarget, TopologyComputer& topoComputer)
{
      int locDimTarget = geomTarget.locateWithDim(pt);
      Location locTarget = DimensionLocation::location(locDimTarget);
      int dimTarget = DimensionLocation::dimension(locDimTarget, topoComputer.getDimension(! isA));
      topoComputer.addPointOnGeometry(isA, locTarget, dimTarget, pt);
}


/* private */
bool
RelateNG::computeLineEnds(
    RelateGeometry& geom, bool isA,
    RelateGeometry& geomTarget, TopologyComputer& topoComputer)
{
    if (! geom.hasDimension(Dimension::L)) {
        return false;
    }

    bool hasExteriorIntersection = false;
    std::vector<const Geometry*> elems;
    GeometryLister::list(geom.getGeometry(), elems);
    for (const Geometry* elem : elems) {
        if (elem->isEmpty())
            continue;

        if (elem->getGeometryTypeId() == GEOS_LINESTRING ||
            elem->getGeometryTypeId() == GEOS_LINEARRING) {
            //-- once an intersection with target exterior is recorded, skip further known-exterior points
            if (hasExteriorIntersection
                && elem->getEnvelopeInternal()->disjoint(geomTarget.getEnvelope()))
                continue;

            const LineString* line = static_cast<const LineString*>(elem);
            //TODO: add optimzation to skip disjoint elements once exterior point found
            const CoordinateXY& e0 = line->getCoordinatesRO()->getAt(0);
            hasExteriorIntersection |= computeLineEnd(geom, isA, &e0, geomTarget, topoComputer);
            if (topoComputer.isResultKnown()) {
                return true;
            }

            if (! line->isClosed()) {
                const CoordinateXY& e1 = line->getCoordinatesRO()->getAt(line->getNumPoints() - 1);
                hasExteriorIntersection |= computeLineEnd(geom, isA, &e1, geomTarget, topoComputer);
                if (topoComputer.isResultKnown()) {
                    return true;
                }
            }
        //TODO: break when all possible locations have been found?
        }
    }
    return false;
}


/* private */
bool
RelateNG::computeLineEnd(RelateGeometry& geom, bool isA, const CoordinateXY* pt,
      RelateGeometry& geomTarget, TopologyComputer& topoComputer)
{
    int locDimLineEnd = geom.locateLineEndWithDim(pt);
    int dimLineEnd = DimensionLocation::dimension(locDimLineEnd, topoComputer.getDimension(isA));
    //-- skip line ends which are in a GC area
    if (dimLineEnd != Dimension::L)
        return false;
    Location locLineEnd = DimensionLocation::location(locDimLineEnd);
    int locDimTarget = geomTarget.locateWithDim(pt);
    Location locTarget = DimensionLocation::location(locDimTarget);
    int dimTarget = DimensionLocation::dimension(locDimTarget, topoComputer.getDimension(! isA));
    topoComputer.addLineEndOnGeometry(isA, locLineEnd, locTarget, dimTarget, pt);
    return locTarget == Location::EXTERIOR;
}

/* private */
bool
RelateNG::computeAreaVertex(RelateGeometry& geom, bool isA, RelateGeometry& geomTarget, TopologyComputer& topoComputer)
{
    if (! geom.hasDimension(Dimension::A)) {
        return false;
    }
    //-- evaluate for line and area targets only, since points are handled in the reverse direction
    if (geomTarget.getDimension() < Dimension::L)
        return false;

    bool hasExteriorIntersection = false;

    std::vector<const Geometry*> elems;
    GeometryLister::list(geom.getGeometry(), elems);
    for (const Geometry* elem : elems) {
        if (elem->isEmpty())
            continue;

        if (elem->getGeometryTypeId() == GEOS_POLYGON) {
            //-- once an intersection with target exterior is recorded, skip further known-exterior points
            if (hasExteriorIntersection && elem->getEnvelopeInternal()->disjoint(geomTarget.getEnvelope()))
                continue;

            const Polygon* poly = static_cast<const Polygon*>(elem);
            hasExteriorIntersection |= computeAreaVertex(geom, isA, poly->getExteriorRing(), geomTarget, topoComputer);
            if (topoComputer.isResultKnown()) {
                return true;
            }
            for (std::size_t j = 0; j < poly->getNumInteriorRing(); j++) {
                hasExteriorIntersection |= computeAreaVertex(geom, isA, poly->getInteriorRingN(j), geomTarget, topoComputer);
                if (topoComputer.isResultKnown()) {
                    return true;
                }
            }
        }
    }
    return false;
}


/* private */
bool
RelateNG::computeAreaVertex(RelateGeometry& geom, bool isA, const LinearRing* ring, RelateGeometry& geomTarget, TopologyComputer& topoComputer)
{
    //TODO: use extremal (highest) point to ensure one is on boundary of polygon cluster
    const CoordinateXY* pt = ring->getCoordinate();

    Location locArea = geom.locateAreaVertex(pt);
    int locDimTarget = geomTarget.locateWithDim(pt);
    Location locTarget = DimensionLocation::location(locDimTarget);
    int dimTarget = DimensionLocation::dimension(locDimTarget, topoComputer.getDimension(! isA));
    topoComputer.addAreaVertex(isA, locArea, locTarget, dimTarget, pt);
    return locTarget == Location::EXTERIOR;
}


/* private */
void
RelateNG::computeAtEdges(RelateGeometry& geomB, TopologyComputer& topoComputer)
{
    Envelope envInt;
    geomA.getEnvelope()->intersection(*(geomB.getEnvelope()), envInt);
    if (envInt.isNull())
        return;

    std::vector<const SegmentString*> edgesB = geomB.extractSegmentStrings(GEOM_B, &envInt);
    EdgeSegmentIntersector intersector(topoComputer);

    if (topoComputer.isSelfNodingRequired()) {
        computeEdgesAll(edgesB, &envInt, intersector);
    }
    else {
        computeEdgesMutual(edgesB, &envInt, intersector);
    }
    if (topoComputer.isResultKnown()) {
        return;
    }

    topoComputer.evaluateNodes();
}


/* private */
void
RelateNG::computeEdgesAll(std::vector<const SegmentString*>& edgesB, const Envelope* envInt, EdgeSegmentIntersector& intersector)
{
    //TODO: find a way to reuse prepared index?
    std::vector<const SegmentString*> edgesA = geomA.extractSegmentStrings(GEOM_A, envInt);

    EdgeSetIntersector edgeInt(edgesA, edgesB, envInt);
    edgeInt.process(intersector);
}


/* private */
void
RelateNG::computeEdgesMutual(std::vector<const SegmentString*>& edgesB, const Envelope* envInt, EdgeSegmentIntersector& intersector)
{
    //-- in prepared mode the A edge index is reused
    if (edgeMutualInt == nullptr) {
        const Envelope* envExtract = geomA.isPrepared() ? nullptr : envInt;
        std::vector<const SegmentString*> edgesA = geomA.extractSegmentStrings(GEOM_A, envExtract);
        edgeMutualInt.reset(new MCIndexSegmentSetMutualIntersector(envExtract));
        edgeMutualInt->setBaseSegments(&edgesA);

    }

    edgeMutualInt->setSegmentIntersector(&intersector);
    edgeMutualInt->process(&edgesB);
}












} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos


