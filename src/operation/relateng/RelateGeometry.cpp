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

#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Location.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/util/ComponentCoordinateExtracter.h>
#include <geos/geom/util/PointExtracter.h>
#include <geos/geom/util/GeometryLister.h>
#include <geos/operation/relateng/RelateGeometry.h>
#include <geos/operation/relateng/RelateSegmentString.h>
#include <geos/operation/relateng/DimensionLocation.h>
#include <geos/operation/valid/RepeatedPointRemover.h>

#include <sstream>


using geos::algorithm::BoundaryNodeRule;
using geos::algorithm::Orientation;
using namespace geos::geom;
using geos::geom::util::ComponentCoordinateExtracter;
using geos::geom::util::GeometryLister;
using geos::geom::util::PointExtracter;
using geos::operation::valid::RepeatedPointRemover;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


RelateGeometry::RelateGeometry(const Geometry* input, bool isPrepared, const BoundaryNodeRule& bnRule)
    : geom(input)
    , m_isPrepared(isPrepared)
    , geomEnv(input->getEnvelopeInternal())
    , boundaryNodeRule(bnRule)
    , geomDim(input->getDimension())
    , isLineZeroLen(isZeroLengthLine(input))
    , isGeomEmpty(input->isEmpty())
{
    analyzeDimensions();
}


/* public static */
std::string
RelateGeometry::name(bool isA)
{
    return isA ? "A" : "B";
}


/* private */
void
RelateGeometry::analyzeDimensions()
{
    if (isGeomEmpty) {
        return;
    }
    GeometryTypeId typeId = geom->getGeometryTypeId();
    if (typeId == GEOS_POINT || typeId == GEOS_MULTIPOINT) {
        hasPoints = true;
        geomDim = Dimension::P;
        return;
    }
    if (typeId == GEOS_LINESTRING || typeId == GEOS_LINEARRING || typeId == GEOS_MULTILINESTRING) {
        hasLines = true;
        geomDim = Dimension::L;
        return;
    }
    if (typeId == GEOS_POLYGON || typeId == GEOS_MULTIPOLYGON) {
        hasAreas = true;
        geomDim = Dimension::A;
        return;
    }
    //-- analyze a (possibly mixed type) collection
    std::vector<const Geometry*> elems;
    GeometryLister::list(geom, elems);
    for (const Geometry* elem : elems)
    {
        if (elem->isEmpty())
            continue;
        if (elem->getGeometryTypeId() == GEOS_POINT) {
            hasPoints = true;
            if (geomDim < Dimension::P) geomDim = Dimension::P;
        }
        if (elem->getGeometryTypeId() == GEOS_LINESTRING ||
            elem->getGeometryTypeId() == GEOS_LINEARRING) {
            hasLines = true;
            if (geomDim < Dimension::L) geomDim = Dimension::L;
        }
        if (elem->getGeometryTypeId() == GEOS_POLYGON) {
            hasAreas = true;
            if (geomDim < Dimension::A) geomDim = Dimension::A;
        }
    }
}


/* private static */
bool
RelateGeometry::isZeroLength(const Geometry* geom)
{
    std::vector<const Geometry*> elems;
    GeometryLister::list(geom, elems);
    for (const Geometry* elem : elems) {
        if (elem->getGeometryTypeId() == GEOS_LINESTRING ||
            elem->getGeometryTypeId() == GEOS_LINEARRING ) {
            if (! isZeroLength(static_cast<const LineString*>(elem))) {
                return false;
            }
        }
    }
    return true;
}

/* private static */
bool
RelateGeometry::isZeroLength(const LineString* line) {
    if (line->getNumPoints() >= 2) {
        const CoordinateXY& p0 = line->getCoordinateN(0);
        for (std::size_t i = 1; i < line->getNumPoints(); i++) {
            // NOTE !!! CHANGE FROM JTS, original below
            // const CoordinateXY& pi = line.getCoordinateN(1);
            const CoordinateXY& pi = line->getCoordinateN(i);
            //-- most non-zero-len lines will trigger this right away
            if (! p0.equals2D(pi)) {
                return false;
            }
        }
    }
    return true;
}


/* public */
int
RelateGeometry::getDimensionReal() const
{
    if (isGeomEmpty)
        return Dimension::False;
    if (getDimension() == Dimension::L && isLineZeroLen)
        return Dimension::P;
    if (hasAreas)
        return Dimension::A;
    if (hasLines)
        return Dimension::L;
    return Dimension::P;
}


/* public */
bool
RelateGeometry::hasEdges() const
{
    return hasLines || hasAreas;
}

/* private */
RelatePointLocator*
RelateGeometry::getLocator()
{
    if (locator == nullptr)
        locator.reset(new RelatePointLocator(geom, m_isPrepared, boundaryNodeRule));
    return locator.get();
}


/* public */
bool
RelateGeometry::isNodeInArea(const CoordinateXY* nodePt, const Geometry* parentPolygonal)
{
    int dimLoc = getLocator()->locateNodeWithDim(nodePt, parentPolygonal);
    return dimLoc == DimensionLocation::AREA_INTERIOR;
}


/* public */
Location
RelateGeometry::locateLineEnd(const CoordinateXY* p)
{
    return getLocator()->locateLineEnd(p);
}


/* public */
Location
RelateGeometry::locateAreaVertex(const CoordinateXY* pt)
{
    /**
     * Can pass a null polygon, because the point is an exact vertex,
     * which will be detected as being on the boundary of its polygon
     */
    return locateNode(pt, nullptr);
}


/* public */
Location
RelateGeometry::locateNode(const CoordinateXY* pt, const Geometry* parentPolygonal)
{
    return getLocator()->locateNode(pt, parentPolygonal);
}


/* public */
int
RelateGeometry::locateWithDim(const CoordinateXY* pt)
{
    int loc = getLocator()->locateWithDim(pt);
    return loc;
}


/* public */
bool
RelateGeometry::isSelfNodingRequired() const
{
    GeometryTypeId typeId = geom->getGeometryTypeId();
    if (typeId == GEOS_POINT
        || typeId == GEOS_MULTIPOINT
        || typeId == GEOS_POLYGON
        || typeId == GEOS_MULTIPOLYGON)
    {
         return false;
     }
    //-- GC with a single polygon does not need noding
    if (hasAreas && geom->getNumGeometries() == 1)
        return false;
    return true;
}


/* public */
bool
RelateGeometry::isPolygonal() const
{
    //TODO: also true for a GC containing one polygonal element (and possibly some lower-dimension elements)
    GeometryTypeId typeId = geom->getGeometryTypeId();
    return typeId == GEOS_POLYGON
        || typeId == GEOS_MULTIPOLYGON;
}


/* public */
bool
RelateGeometry::isEmpty() const
{
    return isGeomEmpty;
}


/* public */
bool
RelateGeometry::hasBoundary()
{
    return getLocator()->hasBoundary();
}


/* public */
Coordinate::ConstXYSet&
RelateGeometry::getUniquePoints()
{
    if (uniquePoints.empty()) {
        uniquePoints = createUniquePoints();
    }
    return uniquePoints;
}


/* private */
Coordinate::ConstXYSet
RelateGeometry::createUniquePoints()
{
    //-- only called on P geometries
    std::vector<const CoordinateXY*> pts;
    ComponentCoordinateExtracter::getCoordinates(*geom, pts);
    Coordinate::ConstXYSet set(pts.begin(), pts.end());
    return set;
}


/* public */
std::vector<const Point*>
RelateGeometry::getEffectivePoints()
{
    std::vector<const Point*> ptListAll;
    geom::util::PointExtracter::getPoints(*geom, ptListAll);

    if (getDimensionReal() <= Dimension::P)
        return ptListAll;

    //-- only return Points not covered by another element
    std::vector<const Point*> ptList;
    for (const Point* p : ptListAll) {
        if (p->isEmpty())
            continue;
        int locDim = locateWithDim(p->getCoordinate());
        if (DimensionLocation::dimension(locDim) == Dimension::P) {
            ptList.push_back(p);
        }
    }
    return ptList;
}


/* public */
std::vector<const SegmentString*>
RelateGeometry::extractSegmentStrings(bool isA, const Envelope* env)
{
    std::vector<const SegmentString*> segStrings;

    // When we get called in the context of a prepared geometry
    // geomA might already have segments extracted and stored,
    // so check and reuse them if possible
    if (isA && isPrepared() && env == nullptr) {
        if (segStringPermStore.empty()) {
            extractSegmentStrings(isA, env, geom, segStrings, segStringPermStore);
        }
        else {
            for (auto& ss : segStringPermStore) {
                segStrings.push_back(ss.get());
            }
        }
    }
    // In the context of geomB we always extract for each call,
    // and same goes for geomA when not in prepared mode, or when
    // using an envelope filter.
    else {
        segStringTempStore.clear();
        extractSegmentStrings(isA, env, geom, segStrings, segStringTempStore);
    }
    return segStrings;
}


/* private */
void
RelateGeometry::extractSegmentStrings(bool isA,
    const Envelope* env, const Geometry* p_geom,
    std::vector<const SegmentString*>& segStrings,
    std::vector<std::unique_ptr<const RelateSegmentString>>& segStore)
{
    //-- record if parent is MultiPolygon
    const MultiPolygon* parentPolygonal = nullptr;
    if (p_geom->getGeometryTypeId() == GEOS_MULTIPOLYGON) {
        parentPolygonal = static_cast<const MultiPolygon*>(p_geom);
    }

    for (std::size_t i = 0; i < p_geom->getNumGeometries(); i++) {
        const Geometry* g = p_geom->getGeometryN(i);
        // if (g->getGeometryTypeId() == GEOS_GEOMETRYCOLLECTION) {
        if (g->isCollection()) {
            extractSegmentStrings(isA, env, g, segStrings, segStore);
        }
        else {
            extractSegmentStringsFromAtomic(isA, g, parentPolygonal, env, segStrings, segStore);
        }
    }
}


/* private */
void
RelateGeometry::extractSegmentStringsFromAtomic(bool isA,
    const Geometry* p_geom, const MultiPolygon* parentPolygonal,
    const Envelope* env,
    std::vector<const SegmentString*>& segStrings,
    std::vector<std::unique_ptr<const RelateSegmentString>>& segStore)
{
    if (p_geom->isEmpty())
        return;

    bool doExtract = (env == nullptr) || env->intersects(p_geom->getEnvelopeInternal());
    if (! doExtract)
        return;

    elementId++;
    if (p_geom->getGeometryTypeId() == GEOS_LINESTRING ||
        p_geom->getGeometryTypeId() == GEOS_LINEARRING) {
        const LineString* line = static_cast<const LineString*>(p_geom);
        /*
         * Condition the input Coordinate sequence so that it has no repeated points.
         * This requires taking a copy which removeRepeated does behind the scenes and stores in csStore.
         */
        const CoordinateSequence* cs = removeRepeated(line->getCoordinatesRO());
        auto ss = RelateSegmentString::createLine(cs, isA, elementId, this);
        segStore.emplace_back(ss);
        segStrings.push_back(ss);
    }
    else if (p_geom->getGeometryTypeId() == GEOS_POLYGON) {
        const Polygon* poly = static_cast<const Polygon*>(p_geom);
        const Geometry* parentPoly;
        if (parentPolygonal != nullptr)
            parentPoly = static_cast<const Geometry*>(parentPolygonal);
        else
            parentPoly = static_cast<const Geometry*>(poly);
        extractRingToSegmentString(isA, poly->getExteriorRing(), 0, env, parentPoly, segStrings, segStore);
        for (uint32_t i = 0; i < poly->getNumInteriorRing(); i++) {
            extractRingToSegmentString(isA, poly->getInteriorRingN(i), static_cast<int>(i+1), env, parentPoly, segStrings, segStore);
        }
    }
}


/* private */
void
RelateGeometry::extractRingToSegmentString(bool isA,
    const LinearRing* ring, int ringId, const Envelope* env,
    const Geometry* parentPoly,
    std::vector<const SegmentString*>& segStrings,
    std::vector<std::unique_ptr<const RelateSegmentString>>& segStore)
{
    if (ring->isEmpty())
        return;
    if (env != nullptr && ! env->intersects(ring->getEnvelopeInternal()))
        return;

    /*
     * Condition the input Coordinate sequence so that it has no repeated points
     * and is oriented in a deterministic way. This requires taking a copy which
     * orientAndRemoveRepeated does behind the scenes and stores in csStore.
     */
    bool requireCW = (ringId == 0);
    const CoordinateSequence* cs = orientAndRemoveRepeated(ring->getCoordinatesRO(), requireCW);
    auto ss = RelateSegmentString::createRing(cs, isA, elementId, ringId, parentPoly, this);
    segStore.emplace_back(ss);
    segStrings.push_back(ss);
}


/* public */
std::string
RelateGeometry::toString() const
{
    return geom->toString();
}


/* public friend */
std::ostream&
operator<<(std::ostream& os, const RelateGeometry& rg)
{
    os << rg.toString();
    return os;
}



/* private */
const CoordinateSequence *
RelateGeometry::orientAndRemoveRepeated(const CoordinateSequence *seq, bool orientCW)
{
    bool isFlipped = (orientCW == Orientation::isCCW(seq));
    bool hasRepeated = seq->hasRepeatedPoints();
    /* Already conditioned */
    if (!isFlipped && !hasRepeated) {
        return seq;
    }

    if (hasRepeated) {
        auto deduped = RepeatedPointRemover::removeRepeatedPoints(seq);
        if (isFlipped)
            deduped->reverse();
        CoordinateSequence* cs = deduped.release();
        csStore.emplace_back(cs);
        return cs;
    }

    if (isFlipped) {
        auto reversed = seq->clone();
        reversed->reverse();
        CoordinateSequence* cs = reversed.release();
        csStore.emplace_back(cs);
        return cs;
    }

    return seq;
}

/* private */
const CoordinateSequence *
RelateGeometry::removeRepeated(const CoordinateSequence *seq)
{
    bool hasRepeated = seq->hasRepeatedPoints();
    if (!hasRepeated)
        return seq;
    auto deduped = RepeatedPointRemover::removeRepeatedPoints(seq);
    CoordinateSequence* cs = deduped.release();
    csStore.emplace_back(cs);
    return cs;
}


} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos


