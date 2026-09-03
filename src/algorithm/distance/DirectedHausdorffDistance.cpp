/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 Martin Davis
 * Copyright (C) 2026 Jeroen Bloemscheer
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/distance/DirectedHausdorffDistance.java
 * (locationtech/jts#1182)
 *
 **********************************************************************/

#include <geos/algorithm/distance/DirectedHausdorffDistance.h>

#include <geos/algorithm/construct/IndexedPointInPolygonsLocator.h>
#include <geos/algorithm/construct/LargestEmptyCircle.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Location.h>
#include <geos/geom/Point.h>
#include <geos/operation/distance/IndexedFacetDistance.h>
#include <geos/util/IllegalArgumentException.h>

#include <cmath>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

using geos::algorithm::construct::IndexedPointInPolygonsLocator;
using geos::algorithm::construct::LargestEmptyCircle;
using geos::geom::CoordinateXY;
using geos::geom::Dimension;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::geom::GeometryComponentFilter;
using geos::geom::LineString;
using geos::geom::Location;
using geos::geom::Point;
using geos::operation::distance::IndexedFacetDistance;

namespace geos {
namespace algorithm {
namespace distance {

namespace {

constexpr double EMPTY_DISTANCE = std::numeric_limits<double>::quiet_NaN();
constexpr double AUTO_TOLERANCE_FACTOR = 1.0e4;
constexpr double AREA_INTERIOR_TOLERANCE_FACTOR = 20;
constexpr double FULLY_WITHIN_TOLERANCE_FACTOR = 10 * AUTO_TOLERANCE_FACTOR;

} // namespace

class DirectedHausdorffDistance::TargetDistance {
public:
    explicit TargetDistance(const Geometry& geom)
        : distanceToFacets(&geom)
        , isArea(geom.getDimension() >= Dimension::A)
    {
        if (isArea) {
            ptInArea = std::make_unique<IndexedPointInPolygonsLocator>(geom);
        }
    }

    CoordinateXY nearestFacetPoint(const CoordinateXY& p)
    {
        CoordinateXY np = distanceToFacets.nearestPoint(p);
        return np;
    }

    CoordinateXY nearestPoint(const CoordinateXY& p)
    {
        if (ptInArea) {
            if (ptInArea->locate(&p) != Location::EXTERIOR) {
                return p;
            }
        }
        return distanceToFacets.nearestPoint(p);
    }

    bool isInterior(const CoordinateXY& p)
    {
        if (!isArea) {
            return false;
        }
        return ptInArea->locate(&p) == Location::INTERIOR;
    }

    bool isInterior(const CoordinateXY& p0, const CoordinateXY& p1)
    {
        if (!isArea) {
            return false;
        }
        double segDist = distanceToFacets.distance(p0, p1);
        if (segDist == 0.0) {
            return false;
        }
        return isInterior(p0);
    }

    /// JTS isSameOrCollinear: both endpoints project onto the same target segment.
    bool isSameOrCollinear(const CoordinateXY& p0, const CoordinateXY& p1)
    {
        auto f0 = distanceToFacets.nearestLocation(p0);
        auto f1 = distanceToFacets.nearestLocation(p1);
        return f0.isSameSegment(f1);
    }

private:
    IndexedFacetDistance distanceToFacets;
    bool isArea;
    std::unique_ptr<IndexedPointInPolygonsLocator> ptInArea;
};

class DHDSegment {
public:
    static DHDSegment create(const CoordinateXY& p0, const CoordinateXY& p1,
                             DirectedHausdorffDistance::TargetDistance& dist)
    {
        DHDSegment seg(p0, p1);
        seg.init(dist);
        return seg;
    }

    static DHDSegment create(const DHDSegment& prevSeg, const CoordinateXY& p1,
                             DirectedHausdorffDistance::TargetDistance& dist)
    {
        DHDSegment seg(prevSeg.p1, p1);
        seg.init(prevSeg.nearPt1, dist);
        return seg;
    }

    CoordinateXY getEndpoint(int index) const
    {
        return index == 0 ? p0 : p1;
    }

    double getLength() const
    {
        return p0.distance(p1);
    }

    double getMaxDistance() const
    {
        return maxDistance;
    }

    double getMaxDistanceBound() const
    {
        return maxDistanceBound;
    }

    DirectedHausdorffDistance::PointPair getMaxDistPts() const
    {
        double dist0 = p0.distance(nearPt0);
        double dist1 = p1.distance(nearPt1);
        if (dist0 > dist1) {
            return DirectedHausdorffDistance::PointPair{p0, nearPt0};
        }
        return DirectedHausdorffDistance::PointPair{p1, nearPt1};
    }

    std::array<DHDSegment, 2> bisect(DirectedHausdorffDistance::TargetDistance& dist) const
    {
        CoordinateXY mid((p0.x + p1.x) / 2.0, (p0.y + p1.y) / 2.0);
        CoordinateXY nearPtMid = dist.nearestPoint(mid);
        return {
            DHDSegment(p0, nearPt0, mid, nearPtMid),
            DHDSegment(mid, nearPtMid, p1, nearPt1)
        };
    }

    bool operator<(const DHDSegment& other) const
    {
        // priority_queue is a max-heap: larger bound first
        return maxDistanceBound < other.maxDistanceBound;
    }

    CoordinateXY p0;
    CoordinateXY nearPt0;
    CoordinateXY p1;
    CoordinateXY nearPt1;

private:
    DHDSegment(const CoordinateXY& p0_, const CoordinateXY& p1_)
        : p0(p0_)
        , p1(p1_)
        , maxDistanceBound(-std::numeric_limits<double>::infinity())
        , maxDistance(0.0)
    {}

    DHDSegment(const CoordinateXY& p0_, const CoordinateXY& nearPt0_,
               const CoordinateXY& p1_, const CoordinateXY& nearPt1_)
        : p0(p0_)
        , nearPt0(nearPt0_)
        , p1(p1_)
        , nearPt1(nearPt1_)
        , maxDistanceBound(-std::numeric_limits<double>::infinity())
        , maxDistance(0.0)
    {
        computeMaxDistances();
    }

    void init(DirectedHausdorffDistance::TargetDistance& dist)
    {
        nearPt0 = dist.nearestPoint(p0);
        nearPt1 = dist.nearestPoint(p1);
        computeMaxDistances();
    }

    void init(const CoordinateXY& nearest0, DirectedHausdorffDistance::TargetDistance& dist)
    {
        nearPt0 = nearest0;
        nearPt1 = dist.nearestPoint(p1);
        computeMaxDistances();
    }

    void computeMaxDistances()
    {
        double dist0 = p0.distance(nearPt0);
        double dist1 = p1.distance(nearPt1);
        maxDistance = std::max(dist0, dist1);
        maxDistanceBound = maxDistance + getLength() / 2.0;
    }

    double maxDistanceBound;
    double maxDistance;
};

/* static */
double
DirectedHausdorffDistance::pairDistance(const std::optional<PointPair>& pts)
{
    if (!pts) {
        return EMPTY_DISTANCE;
    }
    return (*pts)[0].distance((*pts)[1]);
}

/* static */
DirectedHausdorffDistance::PointPair
DirectedHausdorffDistance::pair(const CoordinateXY& p0, const CoordinateXY& p1)
{
    return PointPair{p0, p1};
}

/* static */
double
DirectedHausdorffDistance::computeTolerance(const Geometry& geom)
{
    return geom.getEnvelopeInternal()->getDiameter() / AUTO_TOLERANCE_FACTOR;
}

/* static */
bool
DirectedHausdorffDistance::isBeyond(
    const Envelope& envA, const Envelope& envB, double maxDistance)
{
    if (envA.isNull() || envB.isNull()) {
        return false;
    }
    return envA.getMinX() < envB.getMinX() - maxDistance
        || envA.getMinY() < envB.getMinY() - maxDistance
        || envA.getMaxX() > envB.getMaxX() + maxDistance
        || envA.getMaxY() > envB.getMaxY() + maxDistance;
}

/* static */
bool
DirectedHausdorffDistance::isValidLimit(double limit)
{
    return limit >= 0.0;
}

/* static */
bool
DirectedHausdorffDistance::isBeyondLimit(double maxDist, double maxDistanceLimit)
{
    return maxDistanceLimit >= 0 && maxDist > maxDistanceLimit;
}

/* static */
bool
DirectedHausdorffDistance::isWithinLimit(double maxDist, double maxDistanceLimit)
{
    return maxDistanceLimit >= 0 && maxDist <= maxDistanceLimit;
}

/* static */
double
DirectedHausdorffDistance::distance(const Geometry& a, const Geometry& b)
{
    DirectedHausdorffDistance hd(b);
    return pairDistance(hd.farthestPoints(a));
}

/* static */
double
DirectedHausdorffDistance::distance(const Geometry& a, const Geometry& b, double tolerance)
{
    DirectedHausdorffDistance hd(b);
    return pairDistance(hd.farthestPoints(a, tolerance));
}

/* static */
std::optional<DirectedHausdorffDistance::PointPair>
DirectedHausdorffDistance::distancePoints(const Geometry& a, const Geometry& b)
{
    DirectedHausdorffDistance dhd(b);
    return dhd.farthestPoints(a);
}

/* static */
std::optional<DirectedHausdorffDistance::PointPair>
DirectedHausdorffDistance::distancePoints(
    const Geometry& a, const Geometry& b, double tolerance)
{
    DirectedHausdorffDistance dhd(b);
    return dhd.farthestPoints(a, tolerance);
}

/* static */
std::optional<DirectedHausdorffDistance::PointPair>
DirectedHausdorffDistance::hausdorffDistancePoints(const Geometry& a, const Geometry& b)
{
    DirectedHausdorffDistance hdAB(b);
    auto ptsAB = hdAB.farthestPoints(a);
    DirectedHausdorffDistance hdBA(a);
    auto ptsBA = hdBA.farthestPoints(b);

    if (!ptsAB) {
        return ptsBA ? std::optional<PointPair>(pair((*ptsBA)[1], (*ptsBA)[0])) : std::nullopt;
    }
    if (!ptsBA) {
        return ptsAB;
    }
    if (pairDistance(ptsBA) > pairDistance(ptsAB)) {
        return pair((*ptsBA)[1], (*ptsBA)[0]);
    }
    return ptsAB;
}

/* static */
double
DirectedHausdorffDistance::hausdorffDistance(const Geometry& a, const Geometry& b)
{
    return pairDistance(hausdorffDistancePoints(a, b));
}

/* static */
bool
DirectedHausdorffDistance::isFullyWithinDistance(
    const Geometry& a, const Geometry& b, double maxDistance)
{
    DirectedHausdorffDistance hd(b);
    return hd.isFullyWithinDistance(a, maxDistance);
}

/* static */
bool
DirectedHausdorffDistance::isFullyWithinDistance(
    const Geometry& a, const Geometry& b, double maxDistance, double tolerance)
{
    DirectedHausdorffDistance hd(b);
    return hd.isFullyWithinDistance(a, maxDistance, tolerance);
}

DirectedHausdorffDistance::DirectedHausdorffDistance(const Geometry& geom)
    : target(geom)
    , targetDistance(std::make_unique<TargetDistance>(geom))
{}

DirectedHausdorffDistance::~DirectedHausdorffDistance() = default;

bool
DirectedHausdorffDistance::isFullyWithinDistance(
    const Geometry& geom, double maxDistance)
{
    double tolerance = maxDistance / FULLY_WITHIN_TOLERANCE_FACTOR;
    return isFullyWithinDistance(geom, maxDistance, tolerance);
}

bool
DirectedHausdorffDistance::isFullyWithinDistance(
    const Geometry& geom, double maxDistance, double tolerance)
{
    if (geom.isEmpty() || target.isEmpty()) {
        return false;
    }
    if (isBeyond(*geom.getEnvelopeInternal(), *target.getEnvelopeInternal(), maxDistance)) {
        return false;
    }
    auto maxDistCoords = computeDistancePoints(geom, tolerance, maxDistance);
    if (!maxDistCoords) {
        return false;
    }
    return pairDistance(maxDistCoords) <= maxDistance;
}

std::optional<DirectedHausdorffDistance::PointPair>
DirectedHausdorffDistance::farthestPoints(const Geometry& geom)
{
    return farthestPoints(geom, computeTolerance(geom));
}

std::optional<DirectedHausdorffDistance::PointPair>
DirectedHausdorffDistance::farthestPoints(const Geometry& geom, double tolerance)
{
    return computeDistancePoints(geom, tolerance, -1.0);
}

std::optional<DirectedHausdorffDistance::PointPair>
DirectedHausdorffDistance::computeDistancePoints(
    const Geometry& geom, double tolerance, double maxDistanceLimit)
{
    if (tolerance < 0.0) {
        throw util::IllegalArgumentException("Tolerance must be non-negative");
    }
    if (geom.isEmpty() || target.isEmpty()) {
        return std::nullopt;
    }
    if (geom.getDimension() == Dimension::P) {
        return computeForPoints(geom, maxDistanceLimit);
    }

    auto maxDistPtsEdge = computeForEdges(geom, tolerance, maxDistanceLimit);
    if (isBeyondLimit(pairDistance(maxDistPtsEdge), maxDistanceLimit)) {
        return maxDistPtsEdge;
    }
    if (geom.getDimension() == Dimension::A) {
        auto maxDistPtsInterior = computeForAreaInterior(geom, tolerance);
        if (maxDistPtsInterior
            && pairDistance(maxDistPtsInterior) > pairDistance(maxDistPtsEdge)) {
            return maxDistPtsInterior;
        }
    }
    return maxDistPtsEdge;
}

std::optional<DirectedHausdorffDistance::PointPair>
DirectedHausdorffDistance::computeForPoints(
    const Geometry& geom, double maxDistanceLimit)
{
    double maxDist = -1.0;
    std::optional<PointPair> maxDistPtsAB;

    struct PointFilter : public GeometryComponentFilter {
        PointFilter(TargetDistance& td, double& md, std::optional<PointPair>& pts,
                    double limit)
            : targetDistance(td), maxDist(md), maxDistPtsAB(pts), maxDistanceLimit(limit), done(false)
        {}

        void filter_ro(const Geometry* geomElem) override
        {
            if (done) {
                return;
            }
            const Point* pt = dynamic_cast<const Point*>(geomElem);
            if (!pt || pt->isEmpty()) {
                return;
            }
            const CoordinateXY* pA = pt->getCoordinate();
            if (!pA) {
                return;
            }
            CoordinateXY pB = targetDistance.nearestPoint(*pA);
            double dist = pA->distance(pB);
            bool interior = dist > 0 && targetDistance.isInterior(*pA);
            if (interior) {
                dist = 0;
                pB = *pA;
            }
            if (dist > maxDist) {
                maxDist = dist;
                maxDistPtsAB = DirectedHausdorffDistance::pair(*pA, pB);
            }
            if (DirectedHausdorffDistance::isValidLimit(maxDistanceLimit)
                && DirectedHausdorffDistance::isBeyondLimit(maxDist, maxDistanceLimit)) {
                done = true;
            }
        }

        TargetDistance& targetDistance;
        double& maxDist;
        std::optional<PointPair>& maxDistPtsAB;
        double maxDistanceLimit;
        bool done;
    };

    PointFilter filter(*targetDistance, maxDist, maxDistPtsAB, maxDistanceLimit);
    geom.apply_ro(&filter);
    return maxDistPtsAB;
}

std::optional<DirectedHausdorffDistance::PointPair>
DirectedHausdorffDistance::computeForEdges(
    const Geometry& geom, double tolerance, double maxDistanceLimit)
{
    std::priority_queue<DHDSegment> segQueue;

    struct LineFilter : public GeometryComponentFilter {
        LineFilter(std::priority_queue<DHDSegment>& q, TargetDistance& td)
            : segQueue(q), targetDistance(td)
        {}

        void filter_ro(const Geometry* g) override
        {
            const LineString* ls = dynamic_cast<const LineString*>(g);
            if (!ls || ls->isEmpty()) {
                return;
            }
            const auto* pts = ls->getCoordinatesRO();
            if (!pts || pts->size() < 2) {
                return;
            }
            DHDSegment prevSeg = DHDSegment::create(pts->getAt<CoordinateXY>(0),
                                                    pts->getAt<CoordinateXY>(1),
                                                    targetDistance);
            consider(prevSeg);
            for (std::size_t i = 1; i < pts->size() - 1; i++) {
                DHDSegment seg = DHDSegment::create(prevSeg, pts->getAt<CoordinateXY>(i + 1), targetDistance);
                consider(seg);
                prevSeg = seg;
            }
        }

        void addNonInterior(const DHDSegment& segment)
        {
            if (segment.getMaxDistance() > 0.0) {
                segQueue.push(segment);
                return;
            }
            if (targetDistance.isInterior(segment.getEndpoint(0), segment.getEndpoint(1))) {
                return;
            }
            segQueue.push(segment);
        }

        void consider(const DHDSegment& seg)
        {
            if (!segMaxDist || seg.getMaxDistanceBound() > segMaxDist->getMaxDistance()) {
                addNonInterior(seg);
            }
            if (!segMaxDist || seg.getMaxDistance() > segMaxDist->getMaxDistance()) {
                segMaxDist = seg;
            }
        }

        std::priority_queue<DHDSegment>& segQueue;
        TargetDistance& targetDistance;
        std::optional<DHDSegment> segMaxDist;
    };

    LineFilter filter(segQueue, *targetDistance);
    geom.apply_ro(&filter);

    std::optional<DHDSegment> segMaxDist;
    while (!segQueue.empty()) {
        DHDSegment segMaxBound = segQueue.top();
        segQueue.pop();

        if (!segMaxDist || segMaxBound.getMaxDistance() > segMaxDist->getMaxDistance()) {
            segMaxDist = segMaxBound;
        }
        if (segMaxBound.getMaxDistanceBound() <= segMaxDist->getMaxDistance()) {
            break;
        }
        if (isValidLimit(maxDistanceLimit)) {
            if (isWithinLimit(segMaxBound.getMaxDistanceBound(), maxDistanceLimit)
                || isBeyondLimit(segMaxBound.getMaxDistance(), maxDistanceLimit)) {
                break;
            }
        }
        if (segMaxBound.getMaxDistance() == 0.0
            && targetDistance->isSameOrCollinear(
                segMaxBound.getEndpoint(0), segMaxBound.getEndpoint(1))) {
            continue;
        }
        if (tolerance > 0 && segMaxBound.getLength() > tolerance) {
            auto bisects = segMaxBound.bisect(*targetDistance);
            filter.addNonInterior(bisects[0]);
            filter.addNonInterior(bisects[1]);
        }
    }

    if (segMaxDist) {
        return segMaxDist->getMaxDistPts();
    }
    const CoordinateXY* maxPt = geom.getCoordinate();
    if (!maxPt) {
        return std::nullopt;
    }
    return pair(*maxPt, *maxPt);
}

std::optional<DirectedHausdorffDistance::PointPair>
DirectedHausdorffDistance::computeForAreaInterior(
    const Geometry& geom, double tolerance)
{
    if (tolerance <= 0.0) {
        return std::nullopt;
    }
    const Geometry& polygonal = geom;
    if (polygonal.getEnvelopeInternal()->disjoint(target.getEnvelopeInternal())) {
        return std::nullopt;
    }

    LargestEmptyCircle lec(&target, &polygonal, tolerance * AREA_INTERIOR_TOLERANCE_FACTOR);
    auto centerPt = lec.getCenter();
    const CoordinateXY* ptA = centerPt->getCoordinate();
    if (!ptA) {
        return std::nullopt;
    }
    if (targetDistance->isInterior(*ptA)) {
        return std::nullopt;
    }
    CoordinateXY ptB = targetDistance->nearestFacetPoint(*ptA);
    return pair(*ptA, ptB);
}

} // namespace distance
} // namespace algorithm
} // namespace geos
