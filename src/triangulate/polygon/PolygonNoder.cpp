/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/algorithm/LineIntersector.h>
#include <geos/geom/Coordinate.h>
#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/SegmentIntersector.h>
#include <geos/noding/SegmentString.h>

#include <geos/triangulate/polygon/PolygonNoder.h>


using geos::algorithm::LineIntersector;
using geos::geom::Coordinate;
using geos::noding::SegmentString;
using geos::noding::NodedSegmentString;
using geos::noding::SegmentIntersector;
using geos::noding::MCIndexNoder;


namespace geos {
namespace triangulate {
namespace polygon {


PolygonNoder::PolygonNoder(
    std::unique_ptr<CoordinateSequence>& shellRing,
    std::vector<std::unique_ptr<CoordinateSequence>>& holeRings)
{
    isHoleTouching.resize(holeRings.size(), false);
    createNodedSegmentStrings(shellRing, holeRings);
}



/**
* A {@link SegmentIntersector} that added node vertices
* to {@link NodedSegmentStrings} where a segment touches another
* segment in its interior.
*/
/* private static */
class PolygonNoder::NodeAdder : public SegmentIntersector {

public:

    NodeAdder(std::vector<bool>& p_isHoleTouching,
              std::map<NodedSegmentString*, std::size_t>& p_nodedRingIndexes)
        : isHoleTouching(p_isHoleTouching)
        , nodedRingIndexes(p_nodedRingIndexes)
        {}

    void processIntersections(
        SegmentString* ss0, std::size_t segIndex0,
        SegmentString* ss1, std::size_t segIndex1) override
    {
        //-- input is assumed valid, so rings do not self-intersect
        if (ss0 == ss1)
            return;

        const Coordinate& p00 = ss0->getCoordinate(segIndex0);
        const Coordinate& p01 = ss0->getCoordinate(segIndex0 + 1);
        const Coordinate& p10 = ss1->getCoordinate(segIndex1);
        const Coordinate& p11 = ss1->getCoordinate(segIndex1 + 1);

        li.computeIntersection(p00, p01, p10, p11);
        /**
        * There should never be 2 intersection points, since
        * that would imply collinear segments, and an invalid polygon
        */
        if (li.getIntersectionNum() == 1) {
            addTouch(ss0);
            addTouch(ss1);
            const Coordinate& intPt = li.getIntersection(0);
            if (li.isInteriorIntersection(0)) {
                NodedSegmentString* nss0 = static_cast<NodedSegmentString*>(ss0);
                nss0->addIntersection(intPt, segIndex0);
            }
            else if (li.isInteriorIntersection(1)) {
                NodedSegmentString* nss1 = static_cast<NodedSegmentString*>(ss1);
                nss1->addIntersection(intPt, segIndex1);
            }
        }
    }

    /* public */
    bool isDone() const override {
        return false;
    }


private:

    LineIntersector li;
    std::vector<bool>& isHoleTouching;
    std::map<NodedSegmentString*, std::size_t>& nodedRingIndexes;

    void addTouch(SegmentString* ss) {
        NodedSegmentString* nssOrig = static_cast<NodedSegmentString*>(const_cast<void*>(ss->getData()));
        std::size_t holeIndex = nodedRingIndexes[nssOrig];
        if (holeIndex > 0) {
            isHoleTouching[holeIndex-1] = true;
        }
    }

}; // NodeAdder




/* public */
void
PolygonNoder::node()
{
    NodeAdder nodeAdder(isHoleTouching, nodedRingIndexes);
    MCIndexNoder noder(&nodeAdder);
    std::vector<SegmentString*> bareNodedRings;
    for (auto& nss : nodedRings) {
        bareNodedRings.push_back(nss.get());
    }
    noder.computeNodes(&bareNodedRings);
}

/* public */
bool
PolygonNoder::isShellNoded()
{
    return nodedRings[0]->hasNodes();
}

/* public */
bool
PolygonNoder::isHoleNoded(std::size_t i)
{
    return nodedRings[i+1]->hasNodes();
}

/* public */
std::unique_ptr<CoordinateSequence>
PolygonNoder::getNodedShell()
{
    return nodedRings[0]->getNodedCoordinates();
}

/* public */
std::unique_ptr<CoordinateSequence>
PolygonNoder::getNodedHole(std::size_t i)
{
    return nodedRings[i+1]->getNodedCoordinates();
}

/* public */
std::vector<bool>&
PolygonNoder::getHolesTouching()
{
    return isHoleTouching;
}


/* private */
void
PolygonNoder::createNodedSegmentStrings(
    std::unique_ptr<CoordinateSequence>& shellRing,
    std::vector<std::unique_ptr<CoordinateSequence>>& holeRings)
{
    nodedRings.emplace_back(createNodedSegString(shellRing, 0));
    for (std::size_t i = 1; i <= holeRings.size(); i++) {
        nodedRings.emplace_back(createNodedSegString(holeRings[i-1], i));
    }
    return;
}

/* private */
NodedSegmentString*
PolygonNoder::createNodedSegString(std::unique_ptr<CoordinateSequence>& ringPts, std::size_t i)
{
    // note: in PolygonHoleJoiner::nodeRings we will replace the contents
    // of the shellRing and holeRings with the results of the calculation
    // here, so it's OK to take ownership of the points from them here
    NodedSegmentString* nss = new NodedSegmentString(ringPts.release(), false, false, nullptr);
    nss->setData(nss);
    // need to map the identity of this nss to the index number of the
    // ring it represents. use an external map to avoid abusing the void*
    // data sidecar in the SegmentString too hard
    nodedRingIndexes[nss] = i;
    return nss;
}



} // namespace geos.triangulate.polygon
} // namespace geos.triangulate
} // namespace geos
