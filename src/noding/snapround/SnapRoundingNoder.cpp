/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/noding/MCIndexNoder.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/index/kdtree/KdTree.h>
#include <geos/index/kdtree/KdNode.h>
#include <geos/index/kdtree/KdNodeVisitor.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/snapround/SnapRoundingNoder.h>
#include <geos/noding/snapround/SnapRoundingIntersectionAdder.h>

#include <algorithm> // for std::min and std::max
#include <memory>

using namespace geos::geom;
using namespace geos::index::kdtree;

namespace geos {
namespace noding { // geos.noding
namespace snapround { // geos.noding.snapround


/*public*/
std::vector<SegmentString*>*
SnapRoundingNoder::getNodedSubstrings() const
{
    std::vector<SegmentString*>* nssResult = NodedSegmentString::getNodedSubstrings(snappedResult);

    // Intermediate SegmentStrings are no longer needed
    for (auto nss: snappedResult)
        delete nss;

    return nssResult;
}

/*public*/
void
SnapRoundingNoder::computeNodes(std::vector<SegmentString*>* inputSegStrings)
{
    snapRound(*inputSegStrings, snappedResult);
    return;
}

/*private*/
void
SnapRoundingNoder::snapRound(std::vector<SegmentString*>& inputSegStrings, std::vector<SegmentString*>& resultNodedSegments)
{
    /**
    * Determine hot pixels for intersections and vertices.
    * This is done BEFORE the input lines are rounded,
    * to avoid distorting the line arrangement
    * (rounding can cause vertices to move across edges).
    */
    addIntersectionPixels(inputSegStrings);
    addVertexPixels(inputSegStrings);

    computeSnaps(inputSegStrings, resultNodedSegments);
    return;
}

/*private*/
void
SnapRoundingNoder::addIntersectionPixels(std::vector<SegmentString*>& segStrings)
{
    double tolerance = 1.0 / pm->getScale() / INTERSECTION_NEARNESS_FACTOR;
    SnapRoundingIntersectionAdder intAdder(tolerance);
    MCIndexNoder noder(&intAdder, tolerance);
    noder.computeNodes(&segStrings);
    const auto& intPts = intAdder.getIntersections();
    pixelIndex.addNodes(&intPts);
}

/*private void*/
void
SnapRoundingNoder::addVertexPixels(std::vector<SegmentString*>& segStrings)
{
    for (SegmentString* nss : segStrings) {
        const CoordinateSequence* pts = nss->getCoordinates();
        pixelIndex.add(pts);
    }
}

/*private*/
std::unique_ptr<CoordinateSequence>
SnapRoundingNoder::round(const CoordinateSequence& pts) const
{
    auto roundPts = detail::make_unique<CoordinateSequence>(0u, pts.hasZ(), pts.hasM());
    roundPts->reserve(pts.size());
    pts.forEach([this, &roundPts](auto pt) {
        pm->makePrecise(pt);
        roundPts->add(pt, false);
    });
    return roundPts;
}

/*private*/
void
SnapRoundingNoder::computeSnaps(const std::vector<SegmentString*>& segStrings, std::vector<SegmentString*>& snapped)
{
    for (SegmentString* ss: segStrings) {
        NodedSegmentString* snappedSS = computeSegmentSnaps(detail::down_cast<NodedSegmentString*>(ss));
        if (snappedSS != nullptr) {
            /**
             * Some intersection hot pixels may have been marked as nodes in the previous
             * loop, so add nodes for them.
             */
            snapped.push_back(snappedSS);
        }
    }
    for (SegmentString* ss: snapped) {
        NodedSegmentString* nss = detail::down_cast<NodedSegmentString*>(ss);
        addVertexNodeSnaps(nss);
    }
    return;
}

/**
* Add snapped vertices to a segment string.
* If the segment string collapses completely due to rounding,
* null is returned.
*
* @param ss the segment string to snap
* @return the snapped segment string, or null if it collapses completely
*/
/*private*/
NodedSegmentString*
SnapRoundingNoder::computeSegmentSnaps(NodedSegmentString* ss)
{
    /**
    * Get edge coordinates, including added intersection nodes.
    * The coordinates are now rounded to the grid,
    * in preparation for snapping to the Hot Pixels
    */
    auto pts = ss->getNodedCoordinates();
    auto ptsRound = round(*pts);

    // if complete collapse this edge can be eliminated
    if (ptsRound->size() <= 1)
        return nullptr;

    // Create new nodedSS to allow adding any hot pixel nodes
    NodedSegmentString* snapSS = new NodedSegmentString(ptsRound.release(), ss->getNodeList().getConstructZ(), ss->getNodeList().getConstructM(), ss->getData());

    std::size_t snapSSindex = 0;
    for (std::size_t i = 0, sz = pts->size()-1; i < sz; i++ ) {

        const geom::CoordinateXY& currSnap = snapSS->getCoordinate(snapSSindex);

        /**
        * If the segment has collapsed completely, skip it
        */
        const CoordinateXY& p1 = pts->getAt<CoordinateXY>(i+1);
        CoordinateXY p1Round(p1);
        pm->makePrecise(p1Round);
        if (p1Round.equals2D(currSnap))
            continue;

        const CoordinateXY p0 = pts->getAt<CoordinateXY>(i);

        /**
        * Add any Hot Pixel intersections with *original* segment to rounded segment.
        * (It is important to check original segment because rounding can
        * move it enough to intersect other hot pixels not intersecting original segment)
        */
        snapSegment(p0, p1, snapSS, snapSSindex);
        snapSSindex++;
    }
    return snapSS;
}

/**
* Snaps a segment in a segmentString to HotPixels that it intersects.
*
* @param p0 the segment start coordinate
* @param p1 the segment end coordinate
* @param ss the segment string to add intersections to
* @param segIndex the index of the segment
*/
/*private*/
void
SnapRoundingNoder::snapSegment(const CoordinateXY& p0, const CoordinateXY& p1, NodedSegmentString* ss, std::size_t segIndex)
{
    /* First define a visitor to use in the pixelIndex.query() */
    struct SnapRoundingVisitor : KdNodeVisitor {
        const CoordinateXY& p0;
        const CoordinateXY& p1;
        NodedSegmentString* ss;
        std::size_t segIndex;

        SnapRoundingVisitor(const CoordinateXY& pp0, const CoordinateXY& pp1, NodedSegmentString* pss, std::size_t psegIndex)
            : p0(pp0), p1(pp1), ss(pss), segIndex(psegIndex) {};

        void visit(KdNode* node) override {
            HotPixel* hp = static_cast<HotPixel*>(node->getData());
            /**
            * If the hot pixel is not a node, and it contains one of the segment vertices,
            * then that vertex is the source for the hot pixel.
            * To avoid over-noding a node is not added at this point.
            * The hot pixel may be subsequently marked as a node,
            * in which case the intersection will be added during the final vertex noding phase.
            */
            if (! hp->isNode()) {
                if (hp->intersects(p0) || hp->intersects(p1)) {
                    return;
                }
            }
            /**
            * Add a node if the segment intersects the pixel.
            * Mark the HotPixel as a node (since it may not have been one before).
            * This ensures the vertex for it is added as a node during the final vertex noding phase.
            */
            if (hp->intersects(p0, p1)) {
                ss->addIntersection(hp->getCoordinate(), segIndex);
                hp->setToNode();
            }
        }
    };

    /* Then run the query with the visitor */
    SnapRoundingVisitor srv(p0, p1, ss, segIndex);
    pixelIndex.query(p0, p1, srv);
}


/*private*/
void
SnapRoundingNoder::addVertexNodeSnaps(NodedSegmentString* ss)
{
    const CoordinateSequence* pts = ss->getCoordinates();
    std::size_t i = 0;
    pts->forEach([this, ss, &i](const auto& p0) -> void {
        if (i > 0 && i < ss->size() - 1) {
            this->snapVertexNode(p0, ss, i);
        }
        i++;
    });
}

void
SnapRoundingNoder::snapVertexNode(const CoordinateXY& p0, NodedSegmentString* ss, std::size_t segIndex)
{

    /* First define a visitor to use in the pixelIndex.query() */
    struct SnapRoundingVertexNodeVisitor : KdNodeVisitor {

        const CoordinateXY& p0;
        NodedSegmentString* ss;
        std::size_t segIndex;

        SnapRoundingVertexNodeVisitor(const CoordinateXY& pp0, NodedSegmentString* pss, std::size_t psegIndex)
            : p0(pp0), ss(pss), segIndex(psegIndex) {};

        void visit(KdNode* node) override {
            HotPixel* hp = static_cast<HotPixel*>(node->getData());

            /**
            * If vertex pixel is a node, add it.
            */
            if (hp->isNode() && hp->getCoordinate().equals2D(p0)) {
                ss->addIntersection(p0, segIndex);
            }
        }
    };

    /* Then run the query with the visitor */
    SnapRoundingVertexNodeVisitor srv(p0, ss, segIndex);
    pixelIndex.query(p0, p0, srv);
}





} // namespace geos.noding.snapround
} // namespace geos.noding
} // namespace geos
