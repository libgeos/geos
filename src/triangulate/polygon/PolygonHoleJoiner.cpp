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


#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/PolygonNodeTopology.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/noding/BasicSegmentString.h>
#include <geos/noding/MCIndexSegmentSetMutualIntersector.h>
#include <geos/noding/SegmentSetMutualIntersector.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/triangulate/polygon/PolygonNoder.h>
#include <geos/util/IllegalStateException.h>

#include <geos/triangulate/polygon/PolygonHoleJoiner.h>


using geos::algorithm::LineIntersector;
using geos::algorithm::Orientation;
using geos::algorithm::PolygonNodeTopology;
using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::noding::BasicSegmentString;
using geos::noding::SegmentString;
using geos::noding::NodedSegmentString;
using geos::noding::SegmentSetMutualIntersector;
using geos::noding::MCIndexSegmentSetMutualIntersector;
using geos::util::IllegalStateException;


namespace geos {
namespace triangulate {
namespace polygon {


/**
* Detects if a segment has an interior intersection with another segment.
*/
class PolygonHoleJoiner::InteriorIntersectionDetector : public noding::SegmentIntersector {

private:

    LineIntersector li;
    bool m_hasIntersection = false;

public:

    bool hasIntersection() const
    {
        return m_hasIntersection;
    }

    void processIntersections(
        SegmentString* ss0, std::size_t segIndex0,
        SegmentString* ss1, std::size_t segIndex1) override
    {
        const Coordinate& p00 = ss0->getCoordinate(segIndex0);
        const Coordinate& p01 = ss0->getCoordinate(segIndex0 + 1);
        const Coordinate& p10 = ss1->getCoordinate(segIndex1);
        const Coordinate& p11 = ss1->getCoordinate(segIndex1 + 1);

        li.computeIntersection(p00, p01, p10, p11);
        if (li.getIntersectionNum() == 0) {
            return;
        }
        else if (li.getIntersectionNum() == 1) {
            if (li.isInteriorIntersection())
                m_hasIntersection = true;
        }
        else { // li.getIntersectionNum() >= 2 - must be collinearf
            m_hasIntersection = true;
        }
    }

    bool isDone() const override
    {
        return m_hasIntersection;
    }
}; // InteriorIntersectionDetector


/* public static */
std::unique_ptr<Polygon>
PolygonHoleJoiner::joinAsPolygon(const Polygon* polygon)
{
    auto factory = polygon->getFactory();
    auto lr = factory->createLinearRing(join(polygon));
    return factory->createPolygon(std::move(lr));
}


/* public static */
std::unique_ptr<CoordinateSequence>
PolygonHoleJoiner::join(const Polygon* polygon)
{
    PolygonHoleJoiner joiner(polygon);
    return joiner.compute();
}


/* public */
std::unique_ptr<CoordinateSequence>
PolygonHoleJoiner::compute()
{
    extractOrientedRings(inputPolygon);
    if (holeRings.size() > 0) {
        nodeRings();
    }
    joinedRing.clear();
    joinedRing.add(*shellRing);
    if (holeRings.size() > 0) {
        joinHoles();
    }
    return detail::make_unique<CoordinateSequence>(joinedRing);
}

/* private */
void
PolygonHoleJoiner::extractOrientedRings(const Polygon* polygon)
{
    shellRing = extractOrientedRing(polygon->getExteriorRing(), true);
    std::vector<const LinearRing*> holes = sortHoles(polygon);
    for (const LinearRing* hole : holes) {
        auto oHole = extractOrientedRing(hole, false);
        holeRings.emplace_back(oHole.release());
    }
}

/* private static */
std::unique_ptr<CoordinateSequence>
PolygonHoleJoiner::extractOrientedRing(const LinearRing* ring, bool isCW)
{
    std::unique_ptr<CoordinateSequence> pts = ring->getCoordinates();
    bool isRingCW = ! Orientation::isCCW(pts.get());
    if (isCW != isRingCW) {
        pts->reverse();
    }
    return pts;
}

/* private */
void
PolygonHoleJoiner::nodeRings()
{
    PolygonNoder noder(shellRing, holeRings);
    noder.node();
    shellRing = noder.getNodedShell();
    for (std::size_t i = 0; i < holeRings.size(); i++) {
        holeRings[i].reset(noder.getNodedHole(i).release());
    }
    isHoleTouchingHint = noder.getHolesTouching();
}


/* private */
void
PolygonHoleJoiner::joinHoles()
{
    boundaryIntersector = createBoundaryIntersector();

    joinedPts.clear();
    joinedPts.insert(joinedRing.items<Coordinate>().begin(), joinedRing.items<Coordinate>().end());

    for (std::size_t i = 0; i < holeRings.size(); i++) {
        joinHole(i, *(holeRings[i]));
    }
}

/* private */
void
PolygonHoleJoiner::joinHole(std::size_t index, const CoordinateSequence& holeCoords)
{
    //-- check if hole is touching
    if (isHoleTouchingHint[index]) {
        bool isTouching = joinTouchingHole(holeCoords);
        if (isTouching)
            return;
    }
    joinNonTouchingHole(holeCoords);
}


/* private */
bool
PolygonHoleJoiner::joinTouchingHole(const CoordinateSequence& holeCoords)
{
    std::size_t holeTouchIndex = findHoleTouchIndex(holeCoords);

    //-- hole does not touch
    if (holeTouchIndex == NO_COORD_INDEX)
        return false;

    /**
     * Find shell corner which contains the hole,
     * by finding corner which has a hole segment at the join pt in interior
     */
    const Coordinate& joinPt = holeCoords.getAt(holeTouchIndex);
    const Coordinate& holeSegPt = holeCoords.getAt(prev(holeTouchIndex, holeCoords.size()));

    std::size_t joinIndex = findJoinIndex(joinPt, holeSegPt);
    addJoinedHole(joinIndex, holeCoords, holeTouchIndex);
    return true;
}


/* private */
std::size_t
PolygonHoleJoiner::findHoleTouchIndex(const CoordinateSequence& holeCoords)
{
    std::size_t i = 0;
    for (auto& coord : holeCoords.items<Coordinate>()) {
        if (joinedPts.count(coord) > 0) {
            return i;
        }
        i++;
    }
    return NO_COORD_INDEX;
}


/* private */
void
PolygonHoleJoiner::joinNonTouchingHole(const CoordinateSequence& holeCoords)
{
    std::size_t holeJoinIndex = findLowestLeftVertexIndex(holeCoords);
    const Coordinate& holeJoinCoord = holeCoords.getAt(holeJoinIndex);
    const Coordinate& joinCoord = findJoinableVertex(holeJoinCoord);
    std::size_t joinIndex = findJoinIndex(joinCoord, holeJoinCoord);
    addJoinedHole(joinIndex, holeCoords, holeJoinIndex);
}


const Coordinate&
PolygonHoleJoiner::findJoinableVertex(const Coordinate& holeJoinCoord)
{

    auto it = joinedPts.upper_bound(holeJoinCoord);
    // find highest shell vertex in half-plane left of hole pt
    while ((*it).x == holeJoinCoord.x) {
        it++;
    }
    //-- find rightmost joinable shell vertex
    do {
        it--;
    }
    while (intersectsBoundary(holeJoinCoord, *it) && it != joinedPts.begin());
    return *it;
}


/* private */
std::size_t
PolygonHoleJoiner::findJoinIndex(const Coordinate& joinCoord, const Coordinate& holeJoinCoord)
{
    //-- linear scan is slow but only done once per hole
    for (std::size_t i = 0; i < joinedRing.size() - 1; i++) {
        if (joinCoord.equals2D(joinedRing.getAt(i))) {
            if (isLineInterior(joinedRing, i, holeJoinCoord)) {
                return i;
            }
        }
    }
    throw IllegalStateException("Unable to find shell join index with interior join line");
}


/* private static */
bool
PolygonHoleJoiner::isLineInterior(const CoordinateSequence& ring, std::size_t ringIndex, const Coordinate& linePt)
{
    const Coordinate& nodePt = ring.getAt(ringIndex);
    const Coordinate& shell0 = ring.getAt(prev(ringIndex, ring.size()));
    const Coordinate& shell1 = ring.getAt(next(ringIndex, ring.size()));
    return PolygonNodeTopology::isInteriorSegment(&nodePt, &shell0, &shell1, &linePt);
}

/* private static */
std::size_t
PolygonHoleJoiner::prev(std::size_t i, std::size_t size)
{
    if (i == 0)
        return size - 2;
    return i - 1;
}

/* private static */
std::size_t
PolygonHoleJoiner::next(std::size_t i, std::size_t size)
{
    std::size_t next = i + 1;
    if ((size < 2) || (next > size - 2))
        return 0;
    return next;
}


/* private */
void
PolygonHoleJoiner::addJoinedHole(std::size_t joinIndex, const CoordinateSequence& holeCoords, std::size_t holeJoinIndex)
{
    const Coordinate& joinPt = joinedRing.getAt(joinIndex);
    const Coordinate& holeJoinPt = holeCoords.getAt(holeJoinIndex);

    //-- check for touching (zero-length) join to avoid inserting duplicate vertices
    bool isVertexTouch = joinPt.equals2D(holeJoinPt);
    const Coordinate& addJoinPt = isVertexTouch ? Coordinate::getNull() : joinPt;

    //-- create new section of vertices to insert in shell
    std::vector<Coordinate> newSection = createHoleSection(holeCoords, holeJoinIndex, addJoinPt);

    //-- add section after shell join vertex
    std::size_t addIndex = joinIndex + 1;
    joinedRing.add(addIndex, newSection.begin(), newSection.end());
    joinedPts.insert(newSection.begin(), newSection.end());
}


/* private */
std::vector<Coordinate>
PolygonHoleJoiner::createHoleSection(
    const CoordinateSequence& holeCoords,
    std::size_t holeJoinIndex,
    const Coordinate& joinPt)
{
    std::vector<Coordinate> section;

    bool isNonTouchingHole = ! joinPt.isNull();
    /**
     * Add all hole vertices, including duplicate at hole join vertex
     * Except if hole DOES touch, join vertex is already in shell ring
     */
    if (isNonTouchingHole)
      section.push_back(holeCoords.getAt(holeJoinIndex));

    std::size_t holeSize = holeCoords.size() - 1;
    std::size_t index = holeJoinIndex;
    for (std::size_t i = 0; i < holeSize; i++) {
        index = (index + 1) % holeSize;
        section.push_back(holeCoords.getAt(index));
    }
    /**
     * Add duplicate shell vertex at end of the return join line.
     * Except if hole DOES touch, join line is zero-length so do not need dup vertex
     */
    if (isNonTouchingHole) {
        section.push_back(joinPt);
    }

    return section;
}


/* private static */
std::vector<const LinearRing*>
PolygonHoleJoiner::sortHoles(const Polygon* poly)
{
    std::vector<const LinearRing*> holes;
    for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        holes.push_back(poly->getInteriorRingN(i));
    }
    //  void std::sort( RandomIt first, RandomIt last, Compare comp );
    std::sort(holes.begin(), holes.end(), [](const LinearRing* a, const LinearRing* b) {
        return *(a->getEnvelopeInternal()) < *(b->getEnvelopeInternal());
    });

    return holes;
}


/* private static */
std::size_t
PolygonHoleJoiner::findLowestLeftVertexIndex(const CoordinateSequence& holeCoords)
{
    Coordinate lowestLeftCoord;
    lowestLeftCoord.setNull();
    std::size_t lowestLeftIndex = NO_COORD_INDEX;
    for (std::size_t i = 0; i < holeCoords.size() - 1; i++) {
        if (lowestLeftCoord.isNull() || holeCoords.getAt(i).compareTo(lowestLeftCoord) < 0) {
            lowestLeftCoord = holeCoords.getAt(i);
            lowestLeftIndex = i;
        }
    }
    return lowestLeftIndex;
}


/* private */
bool
PolygonHoleJoiner::intersectsBoundary(const Coordinate& p0, const Coordinate& p1)
{
    CoordinateSequence cs { p0, p1 };
    BasicSegmentString bss(&cs, nullptr);
    std::vector<const SegmentString*> segStrings { &bss };

    InteriorIntersectionDetector segInt;
    boundaryIntersector->setSegmentIntersector(&segInt);
    boundaryIntersector->process(&segStrings);
    return segInt.hasIntersection();
}


/* private */
std::unique_ptr<SegmentSetMutualIntersector>
PolygonHoleJoiner::createBoundaryIntersector()
{
    std::vector<const SegmentString*> polySegStrings;
    polySegStringStore.clear();
    BasicSegmentString* bss = new BasicSegmentString(shellRing.get(), nullptr);
    polySegStringStore.emplace_back(bss);
    polySegStrings.push_back(bss);
    for (auto& hole : holeRings) {
        bss = new BasicSegmentString(hole.get(), nullptr);
        polySegStringStore.emplace_back(bss);
        polySegStrings.push_back(bss);
    }
    std::unique_ptr<MCIndexSegmentSetMutualIntersector> mssmi(new MCIndexSegmentSetMutualIntersector());
    mssmi->setBaseSegments(&polySegStrings);
    return mssmi;
}


} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos
