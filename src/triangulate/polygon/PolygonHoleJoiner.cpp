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
#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateSequenceFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>
#include <geos/noding/BasicSegmentString.h>
#include <geos/noding/MCIndexSegmentSetMutualIntersector.h>
#include <geos/noding/SegmentIntersectionDetector.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/SegmentStringUtil.h>
#include <geos/util/IllegalStateException.h>
#include <geos/util/IllegalArgumentException.h>

#include <geos/triangulate/polygon/PolygonHoleJoiner.h>

#include <limits>


using geos::geom::GeometryFactory;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateSequenceFactory;
using geos::noding::SegmentString;
using geos::noding::SegmentSetMutualIntersector;

namespace geos {
namespace triangulate {
namespace polygon {


PolygonHoleJoiner::PolygonHoleJoiner(const Polygon* p_inputPolygon)
    : polygonIntersector(nullptr)
    , inputPolygon(p_inputPolygon)
{
    polygonIntersector = createPolygonIntersector(p_inputPolygon);
    if(p_inputPolygon->getNumPoints() < 4)
        throw util::IllegalArgumentException("Input polygon has too few points");
}


/* public static */
std::unique_ptr<Polygon>
PolygonHoleJoiner::joinAsPolygon(const Polygon* inputPolygon)
{
    std::vector<Coordinate> coords = join(inputPolygon);
    const geom::GeometryFactory* gf = inputPolygon->getFactory();
    return gf->createPolygon(std::move(coords));
}


/* public static */
std::vector<Coordinate>
PolygonHoleJoiner::join(const Polygon* inputPolygon)
{
    PolygonHoleJoiner joiner(inputPolygon);
    return joiner.compute();
}


/* public */
std::vector<Coordinate>
PolygonHoleJoiner::compute()
{
    //--- copy the input polygon shell coords
    shellCoords = ringCoordinates(inputPolygon->getExteriorRing());
    if (inputPolygon->getNumInteriorRing() != 0) {
        joinHoles();
    }
    return shellCoords;
}


/* private static */
std::vector<Coordinate>
PolygonHoleJoiner::ringCoordinates(const LinearRing* ring)
{
    const CoordinateSequence* cs = ring->getCoordinatesRO();
    std::vector<Coordinate> coords(cs->size());
    for (std::size_t i = 0; i < cs->size(); i++) {
        coords[i] = cs->getAt(i);
    }
    return coords;
}


/* private */
void
PolygonHoleJoiner::joinHoles()
{
    shellCoordsSorted.insert(shellCoords.begin(), shellCoords.end());
    std::vector<const LinearRing*> orderedHoles = sortHoles(inputPolygon);
    for (std::size_t i = 0; i < orderedHoles.size(); i++) {
        joinHole(orderedHoles.at(i));
    }
}


/* private */
void
PolygonHoleJoiner::joinHole(const LinearRing* hole)
{
    /**
     * 1) Get a list of HoleVertex Index.
     * 2) Get a list of ShellVertex.
     * 3) Get the pair that has the shortest distance between them.
     * This pair is the endpoints of the cut
     * 4) The selected ShellVertex may occurs multiple times in
     * shellCoords[], so find the proper one and add the hole after it.
     */
    const CoordinateSequence* holeCoordSeq = hole->getCoordinatesRO();
    std::vector<std::size_t> holeLeftVerticesIndex = findLeftVertices(hole);
    const Coordinate& holeCoord = holeCoordSeq->getAt(holeLeftVerticesIndex[0]);
    std::vector<Coordinate> shellCoordsList = findLeftShellVertices(holeCoord);
    Coordinate shellCoord = shellCoordsList.at(0);
    std::size_t shortestHoleVertexIndex = 0;
    //--- pick the shell-hole vertex pair that gives the shortest distance
    if (std::abs(shellCoord.x - holeCoord.x) < EPS) {
        double shortest = DoubleInfinity;
        for (std::size_t i = 0; i < holeLeftVerticesIndex.size(); i++) {
            for (std::size_t j = 0; j < shellCoordsList.size(); j++) {
                double shellCoordY = shellCoordsList[j].y;
                double holeLeftY = (holeCoordSeq->getAt(holeLeftVerticesIndex[i])).y;
                double currLength = std::abs(shellCoordY - holeLeftY);
                if (currLength < shortest) {
                    shortest = currLength;
                    shortestHoleVertexIndex = i;
                    shellCoord = shellCoordsList[j];
                }
            }
        }
    }
    std::size_t shellVertexIndex = getShellCoordIndex(shellCoord,
        holeCoordSeq->getAt(holeLeftVerticesIndex[shortestHoleVertexIndex]));
    addHoleToShell(shellVertexIndex, holeCoordSeq, holeLeftVerticesIndex[shortestHoleVertexIndex]);
}


/* private */
std::size_t
PolygonHoleJoiner::getShellCoordIndex(const Coordinate& shellVertex, const Coordinate& holeVertex)
{
    std::size_t numSkip = 0;
    std::vector<Coordinate> newValueList;
    newValueList.emplace_back(holeVertex);
    auto it = cutMap.find(shellVertex);
    if (it != cutMap.end()) { // found
        std::vector<Coordinate>& cutMapCoords = it->second; // value
        for (const Coordinate& coord : cutMapCoords) {
            if ( coord.y < holeVertex.y ) {
                numSkip++;
            }
        }
        cutMapCoords.emplace_back(holeVertex);
    } else {
        cutMap[shellVertex] = newValueList;
    }
    it = cutMap.find(holeVertex);
    if (it == cutMap.end()) { // not found
        cutMap[holeVertex] = newValueList;
    }
    return getShellCoordIndexSkip(shellVertex, numSkip);
}


/* private */
std::size_t
PolygonHoleJoiner::getShellCoordIndexSkip(const Coordinate& coord, std::size_t numSkip)
{
    for (std::size_t i = 0; i < shellCoords.size(); i++) {
        if (shellCoords[i].equals2D(coord, EPS)) {
            if (numSkip == 0)
                return i;
            numSkip--;
        }
    }
    throw util::IllegalStateException("Vertex is not in shellcoords");
}

/* private */
std::vector<Coordinate>
PolygonHoleJoiner::findLeftShellVertices(const Coordinate& holeCoord)
{
    std::vector<Coordinate> list;
    auto it = shellCoordsSorted.upper_bound(holeCoord);
    // scroll forward until x changes
    while ((*it).x == holeCoord.x) {
        it++;
    }
    do {
        it--;
    } while (!isJoinable(holeCoord, *it) && it != shellCoordsSorted.begin());
    // we now have the closest coordinate
    list.emplace_back(*it);
    if ( it->x != holeCoord.x )
        return list;

    const double chosenX = it->x;
    list.clear();
    while (chosenX == it->x) {
        list.emplace_back(*it);
        // reached the start of the list
        if (it == shellCoordsSorted.begin())
            return list;
        // paper over difference between Java TreeSet.lower()
        // and std::set::lower_bound()
        it--;
    }
    return list;
}

/* private */
bool
PolygonHoleJoiner::isJoinable(const Coordinate& holeCoord, const Coordinate& shellCoord) const
{
    /**
     * Since the line runs between a hole and the shell,
     * it is inside the polygon if it does not cross the polygon boundary.
     */
    bool bIsJoinable = ! crossesPolygon(holeCoord, shellCoord);
    /*
    //--- slow code for testing only
    LineString join = geomFact.createLineString(new Coordinate[] { holeCoord, shellCoord });
    bool isJoinableSlow = inputPolygon.covers(join)
    if (isJoinableSlow != isJoinable) {
      System.out.println(WKTWriter.toLineString(holeCoord, shellCoord));
    }
    //Assert.isTrue(isJoinableSlow == isJoinable);
    */
    return bIsJoinable;
}


/* private */
bool
PolygonHoleJoiner::crossesPolygon(const Coordinate& p0, const Coordinate& p1) const
{
    // Build up a two-point SegmentString to pass to
    // the SegmentIntersectionDetector
    std::vector<Coordinate> coords;
    coords.emplace_back(p0);
    coords.emplace_back(p1);
    auto* csf = inputPolygon->getFactory()->getCoordinateSequenceFactory();
    auto cs = csf->create(std::move(coords));
    noding::BasicSegmentString segString(cs.get(), nullptr);

    std::vector<const SegmentString*> segStrings;
    segStrings.push_back(&segString);

    algorithm::LineIntersector li;
    noding::SegmentIntersectionDetector segInt(&li);
    segInt.setFindProper(true);
    polygonIntersector->setSegmentIntersector(&segInt);
    polygonIntersector->process(&segStrings);
    return segInt.hasProperIntersection();
}


/* private */
void
PolygonHoleJoiner::addHoleToShell(std::size_t shellJoinIndex,
    const CoordinateSequence* holeCoords, std::size_t holeJoinIndex)
{
    Coordinate shellJoinPt = shellCoords[shellJoinIndex];
    Coordinate holeJoinPt = holeCoords->getAt(holeJoinIndex);
    //-- check for touching (zero-length) join to avoid inserting duplicate vertices
    bool isJoinTouching = shellJoinPt.equals2D(holeJoinPt);

    //-- create new section of vertices to insert in shell
    std::vector<Coordinate> newSection;
    if (! isJoinTouching) {
        newSection.emplace_back(shellJoinPt);
    }

    std::size_t nPts = holeCoords->size() - 1;
    std::size_t i = holeJoinIndex;
    do {
        newSection.emplace_back(holeCoords->getAt(i));
        i = (i + 1) % nPts;
    } while (i != holeJoinIndex);
    if (! isJoinTouching) {
        newSection.emplace_back(holeCoords->getAt(holeJoinIndex));
    }

    // Insert newCoords into shellCoords, starting at shellVertextIndex
    shellCoords.insert(
        shellCoords.begin() + static_cast<long>(shellJoinIndex),
        newSection.begin(),
        newSection.end());
    // Insert all newCoords into orderedCoords
    shellCoordsSorted.insert(newSection.begin(), newSection.end());
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
std::vector<std::size_t>
PolygonHoleJoiner::findLeftVertices(const LinearRing* ring)
{
    const CoordinateSequence* cs = ring->getCoordinatesRO();
    std::vector<std::size_t> leftmostIndex;
    double leftX = ring->getEnvelopeInternal()->getMinX();
    for (std::size_t i = 0; i < cs->size(); i++) {
        if (std::abs(cs->getAt(i).x - leftX) < EPS) {
            leftmostIndex.push_back(i);
        }
    }
    return leftmostIndex;
}

/* private */
std::unique_ptr<SegmentSetMutualIntersector>
PolygonHoleJoiner::createPolygonIntersector(const Polygon* polygon)
{
    std::vector<const SegmentString*> polySegStrings;
    noding::SegmentStringUtil::extractSegmentStrings(polygon, polySegStrings);

    // Put the ownership on the PolygonHoleJoiner, so that
    // when the joining is done, the SegmentStrings go away.
    // TODO: xxxxxx Make extractSegmentStrings() return non-const pointers
    // so that this cast and others go away
    for (const SegmentString* ss: polySegStrings)
        polySegStringStore.emplace_back(const_cast<SegmentString*>(ss));

    std::unique_ptr<SegmentSetMutualIntersector> ssmi(new noding::MCIndexSegmentSetMutualIntersector());
    ssmi->setBaseSegments(&polySegStrings);
    return ssmi;
}


} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos
