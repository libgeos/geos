/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/


#include <geos/algorithm/Distance.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/util/GeometryMapper.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainSelectAction.h>
#include <geos/util/Assert.h>
#include <geos/operation/valid/RepeatedPointRemover.h>

#include <geos/operation/buffer/BufferOp.h>
#include <geos/operation/buffer/OffsetCurve.h>
#include <geos/operation/buffer/OffsetCurveBuilder.h>
#include <geos/operation/buffer/OffsetCurveSection.h>
#include <geos/operation/buffer/SegmentMCIndex.h>

using geos::algorithm::Distance;
using geos::geom::util::GeometryMapper;
using geos::index::chain::MonotoneChain;
using geos::index::chain::MonotoneChainSelectAction;
using geos::operation::valid::RepeatedPointRemover;

using namespace geos::geom;

namespace geos {
namespace operation {
namespace buffer {

static constexpr double NOT_IN_CURVE = -1.0;

/* public */
void
OffsetCurve::setJoined(bool pIsJoined)
{
    isJoined = pIsJoined;
}


/* public */
std::unique_ptr<Geometry>
OffsetCurve::getCurve()
{
    GeometryMapper::mapOp GetCurveMapOp = [this](const Geometry& geom)->std::unique_ptr<Geometry> {

        if (geom.getGeometryTypeId() == GEOS_POINT) return nullptr;
        if (geom.getGeometryTypeId() == GEOS_POLYGON) {
            auto boundary = geom.buffer(distance)->getBoundary();

            if (boundary->getGeometryTypeId() == GEOS_LINEARRING) {
                const LinearRing& ring = static_cast<const LinearRing&>(geom);
                auto ringCs = ring.getCoordinatesRO();
                std::unique_ptr<Geometry> ls(geom.getFactory()->createLineString(*ringCs));
                return ls;
            }
            return boundary;
        }
        return computeCurve(static_cast<const LineString&>(geom), distance);
    };

    return GeometryMapper::flatMap(inputGeom, 1, GetCurveMapOp);
}

/* public static */
std::unique_ptr<Geometry>
OffsetCurve::getCurve(const Geometry& geom, double distance)
{
    OffsetCurve oc(geom, distance);
    return oc.getCurve();
}


/* public static */
std::unique_ptr<Geometry>
OffsetCurve::getCurve(const Geometry& geom,
    double dist,
    int quadSegs,
    BufferParameters::JoinStyle joinStyle,
    double mitreLimit)
{
    BufferParameters bufParms;
    if (quadSegs >= 0) bufParms.setQuadrantSegments(quadSegs);
    if (joinStyle >= 0) bufParms.setJoinStyle(joinStyle);
    if (mitreLimit >= 0) bufParms.setMitreLimit(mitreLimit);
    OffsetCurve oc(geom, dist, bufParms);
    return oc.getCurve();
}


/* public static */
std::unique_ptr<Geometry>
OffsetCurve::getCurveJoined(const Geometry& geom, double dist)
{
    OffsetCurve oc(geom, dist);
    oc.setJoined(true);
    return oc.getCurve();
}


/* public static */
std::unique_ptr<CoordinateSequence>
OffsetCurve::rawOffsetCurve(
    const LineString& line,
    double dist,
    BufferParameters& bufParams)
{
    const CoordinateSequence* pts = line.getCoordinatesRO();
    std::unique_ptr<CoordinateSequence> cleanPts = RepeatedPointRemover::removeRepeatedAndInvalidPoints(pts);

    OffsetCurveBuilder ocb(line.getFactory()->getPrecisionModel(), bufParams);
    return ocb.getOffsetCurve(cleanPts.get(), dist);
}


/* public static */
std::unique_ptr<CoordinateSequence>
OffsetCurve::rawOffset(const LineString& line, double dist)
{
    BufferParameters bufParams;
    return rawOffsetCurve(line, dist, bufParams);
}

/* private */
std::unique_ptr<Geometry>
OffsetCurve::computeCurve(const LineString& lineGeom, double dist)
{
    //-- first handle simple cases
    //-- empty or single-point line
    if (lineGeom.getNumPoints() < 2 || lineGeom.getLength() == 0.0) {
        return geomFactory->createLineString();
    }
    //-- zero offset distance
    if (dist == 0) {
      return lineGeom.clone();
    }
    //-- two-point line
    if (lineGeom.getNumPoints() == 2) {
        return offsetSegment(lineGeom.getCoordinatesRO(), dist);
    }

    auto sections = computeSections(lineGeom, dist);

    if (isJoined) {
        return OffsetCurveSection::toLine(sections, geomFactory);
    }
    else {
        return OffsetCurveSection::toGeometry(sections, geomFactory);
    }
}

/* private */
std::vector<std::unique_ptr<OffsetCurveSection>>
OffsetCurve::computeSections(const LineString& lineGeom, double dist)
{
    std::unique_ptr<CoordinateSequence> rawCurve = rawOffsetCurve(lineGeom, dist, bufferParams);
    std::vector<std::unique_ptr<OffsetCurveSection>> sections;
    if (rawCurve->size() == 0) {
        return sections;
    }

    /**
     * Note: If the raw offset curve has no
     * narrow concave angles or self-intersections it could be returned as is.
     * However, this is likely to be a less frequent situation,
     * and testing indicates little performance advantage,
     * so not doing this.
     */
    std::unique_ptr<Polygon> bufferPoly = getBufferOriented(lineGeom, dist, bufferParams);

    //-- first extract offset curve sections from shell
    auto shell = bufferPoly->getExteriorRing()->getCoordinatesRO();
    computeCurveSections(shell, *rawCurve, sections);

    //-- extract offset curve sections from holes
    for (std::size_t i = 0; i < bufferPoly->getNumInteriorRing(); i++) {
        auto hole = bufferPoly->getInteriorRingN(i)->getCoordinatesRO();
        computeCurveSections(hole, *rawCurve, sections);
    }
    return sections;
}

/* private */
std::unique_ptr<LineString>
OffsetCurve::offsetSegment(const CoordinateSequence* pts, double dist)
{
    LineSegment offsetSeg(pts->getAt(0), pts->getAt(1));
    offsetSeg = offsetSeg.offset(dist);
    CoordinateSequence cs;
    cs.add(offsetSeg.p0);
    cs.add(offsetSeg.p1);
    return geomFactory->createLineString(std::move(cs));
}

/* private static */
std::unique_ptr<Polygon>
OffsetCurve::getBufferOriented(const LineString& geom, double dist, BufferParameters& bufParams)
{
    std::unique_ptr<Geometry> buffer = BufferOp::bufferOp(&geom, std::abs(dist), bufParams);
    const Polygon* bufferPoly = extractMaxAreaPolygon(buffer.get());
    //-- for negative distances (Right of input) reverse buffer direction to match offset curve
    return dist < 0
        ? bufferPoly->reverse()
        : bufferPoly->clone();
}


/* private static */
const Polygon*
OffsetCurve::extractMaxAreaPolygon(const Geometry* geom)
{
    if (geom->getGeometryTypeId() == GEOS_POLYGON)
        return static_cast<const Polygon*>(geom);

    double maxArea = 0.0;
    const Polygon* maxPoly = nullptr;
    for (std::size_t i = 0; i < geom->getNumGeometries(); i++) {
        const Geometry* subgeom = geom->getGeometryN(i);
        if (subgeom->getGeometryTypeId() != GEOS_POLYGON) continue;
        const Polygon* poly = static_cast<const Polygon*>(subgeom);
        double area = poly->getArea();
        if (maxPoly == nullptr || area > maxArea) {
            maxPoly = poly;
            maxArea = area;
        }
    }
    return maxPoly;
}


/* private */
void
OffsetCurve::computeCurveSections(
    const CoordinateSequence* bufferRingPts,
    const CoordinateSequence& rawCurve,
    std::vector<std::unique_ptr<OffsetCurveSection>>& sections)
{
    std::vector<double> rawPosition(bufferRingPts->size()-1, NOT_IN_CURVE);

    SegmentMCIndex bufferSegIndex(bufferRingPts);
    std::size_t bufferFirstIndex = NO_COORD_INDEX;
    double minRawPosition = -1;
    for (std::size_t i = 0; i < rawCurve.size() - 1; i++) {
        std::size_t minBufferIndexForSeg = matchSegments(rawCurve[i], rawCurve[i+1], i, bufferSegIndex, bufferRingPts, rawPosition);
        if (minBufferIndexForSeg != NO_COORD_INDEX) {
            double pos = rawPosition[minBufferIndexForSeg];
            if (bufferFirstIndex == NO_COORD_INDEX || pos < minRawPosition) {
                minRawPosition = pos;
                bufferFirstIndex = minBufferIndexForSeg;
            }
        }
    }
    //-- no matching sections found in this buffer ring
    if (bufferFirstIndex == NO_COORD_INDEX)
        return;

    extractSections(bufferRingPts, rawPosition, bufferFirstIndex, sections);
}


/* private */
std::size_t
OffsetCurve::matchSegments(
    const Coordinate& raw0, const Coordinate& raw1,
    std::size_t rawCurveIndex,
    SegmentMCIndex& bufferSegIndex,
    const CoordinateSequence* bufferPts,
    std::vector<double>& rawCurvePos)
{

    /**
    * An action to match a raw offset curve segment
    * to segments in a buffer ring
    * and record the matched segment locations(s) along the raw curve.
    *
    * @author Martin Davis
    */
    /* private static */
    class MatchCurveSegmentAction : public MonotoneChainSelectAction
    {

    public:

        const Coordinate& p0;
        const Coordinate& p1;
        std::size_t rawCurveIndex;
        double matchDistance;
        const CoordinateSequence* bufferRingPts;
        std::vector<double>& rawCurveLoc;
        double minRawLocation;
        std::size_t bufferRingMinIndex;

        MatchCurveSegmentAction(
            const Coordinate& p_p0,
            const Coordinate& p_p1,
            std::size_t p_rawCurveIndex,
            double p_matchDistance,
            const CoordinateSequence* p_bufferRingPts,
            std::vector<double>& p_rawCurveLoc)
            : p0(p_p0)
            , p1(p_p1)
            , rawCurveIndex(p_rawCurveIndex)
            , matchDistance(p_matchDistance)
            , bufferRingPts(p_bufferRingPts)
            , rawCurveLoc(p_rawCurveLoc)
            , minRawLocation(-1.0)
            , bufferRingMinIndex(NO_COORD_INDEX)
            {};

        std::size_t getBufferMinIndex() {
            return bufferRingMinIndex;
        }

        void select(const geom::LineSegment& seg) override {
            (void)seg; // quiet ununsed variable warning
            return;
        }

        void select(const MonotoneChain& mc, std::size_t segIndex) override
        {
            (void)mc; // quiet ununsed variable warning
            /**
            * A curveRingPt segment may match all or only a portion of a single raw segment.
            * There may be multiple curve ring segs that match along the raw segment.
            */
            double frac = segmentMatchFrac(
                bufferRingPts->getAt(segIndex),
                bufferRingPts->getAt(segIndex+1),
                p0, p1, matchDistance);

            //-- no match
            if (frac < 0) return;

            //-- location is used to sort segments along raw curve
            double location = static_cast<double>(rawCurveIndex) + frac;
            rawCurveLoc[segIndex] = location;
            //-- record lowest index
            if (minRawLocation < 0 || location < minRawLocation) {
                minRawLocation = location;
                bufferRingMinIndex = segIndex;
            }
        }
    };

    Envelope matchEnv(raw0, raw1);
    matchEnv.expandBy(matchDistance);
    MatchCurveSegmentAction matchAction(raw0, raw1, rawCurveIndex, matchDistance, bufferPts, rawCurvePos);
    bufferSegIndex.query(&matchEnv, matchAction);
    return matchAction.getBufferMinIndex();
}

/* private static */
double
OffsetCurve::segmentMatchFrac(
    const Coordinate& p0,   const Coordinate& p1,
    const Coordinate& seg0, const Coordinate& seg1,
    double matchDistance)
{
    if (matchDistance < Distance::pointToSegment(p0, seg0, seg1))
        return -1.0;
    if (matchDistance < Distance::pointToSegment(p1, seg0, seg1))
        return -1.0;
    //-- matched - determine position as fraction along segment
    LineSegment seg(seg0, seg1);
    return seg.segmentFraction(p0);
}


/* private */
void
OffsetCurve::extractSections(
    const CoordinateSequence* ringPts,
    std::vector<double>& rawCurveLoc,
    std::size_t startIndex,
    std::vector<std::unique_ptr<OffsetCurveSection>>& sections)
{
    std::size_t sectionStart = startIndex;
    std::size_t sectionCount = 0;
    std::size_t sectionEnd;
    do {
        sectionEnd = findSectionEnd(rawCurveLoc, sectionStart, startIndex);
        double location = rawCurveLoc[sectionStart];
        std::size_t lastIndex = prevIndex(sectionEnd, rawCurveLoc.size());
        double lastLoc = rawCurveLoc[lastIndex];
        std::unique_ptr<OffsetCurveSection> section = OffsetCurveSection::create(ringPts, sectionStart, sectionEnd, location, lastLoc);
        sections.emplace_back(section.release());
        sectionStart = findSectionStart(rawCurveLoc, sectionEnd);

        //-- check for an abnormal state
        if (sectionCount++ > ringPts->size()) {
            util::Assert::shouldNeverReachHere("Too many sections for ring - probable bug");
        }
    } while (sectionStart != startIndex && sectionEnd != startIndex);
}


/* private */
std::size_t
OffsetCurve::findSectionStart(
    const std::vector<double>& loc,
    std::size_t end)
{
    std::size_t start = end;
    do {
        std::size_t next = nextIndex(start, loc.size());
        //-- skip ahead if segment is not in raw curve
        if (loc[start] == NOT_IN_CURVE) {
            start = next;
            continue;
        }
        std::size_t prev = prevIndex(start, loc.size());
        //-- if prev segment is not in raw curve then have found a start
        if (loc[prev] == NOT_IN_CURVE) {
            return start;
        }
        if (isJoined) {
            /**
             *  Start section at next gap in raw curve.
             *  Only needed for joined curve, since otherwise
             *  contiguous buffer segments can be in same curve section.
             */
            double locDelta = std::abs(loc[start] - loc[prev]);
            if (locDelta > 1.0)
                return start;
        }
        start = next;
    } while (start != end);
    return start;
}


/* private */
std::size_t
OffsetCurve::findSectionEnd(
    const std::vector<double>& loc,
    std::size_t start,
    std::size_t firstStartIndex)
{
    // assert: pos[start] is IN CURVE
    std::size_t end = start;
    std::size_t next;
    do {
        next = nextIndex(end, loc.size());
        if (loc[next] == NOT_IN_CURVE)
            return next;
        if (isJoined) {
        /**
         *  End section at gap in raw curve.
         *  Only needed for joined curve, since otherwise
         *  contigous buffer segments can be in same section
         */
            double locDelta = std::abs(loc[next] - loc[end]);
            if (locDelta > 1)
                return next;
        }
      end = next;
    } while (end != start && end != firstStartIndex);
    return end;
}

/* private static */
std::size_t
OffsetCurve::nextIndex(std::size_t i, std::size_t size)
{
    return i >= size - 1 ? 0 : i + 1;
}

/* private static */
std::size_t
OffsetCurve::prevIndex(std::size_t i, std::size_t size)
{
    return i == 0 ? size - 1 : i - 1;
}





} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos
