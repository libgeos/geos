/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 * Copyright (C) 2005 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: operation/buffer/SubgraphDepthLocater.java r320 (JTS-1.12)
 *
 **********************************************************************/

#include <vector>
#include <cassert>
#include <algorithm>

#include <geos/operation/buffer/BufferSubgraph.h>
#include <geos/operation/buffer/SubgraphDepthLocater.h>

#include <geos/algorithm/Orientation.h>

#include <geos/geom/Envelope.h>
#include <geos/geom/CoordinateSequence.h>

#include <geos/geomgraph/DirectedEdge.h>
#include <geos/geomgraph/Edge.h>
#include <geos/geom/Position.h>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif


using namespace geos::geomgraph;
using namespace geos::algorithm;
using namespace geos::geom;

namespace geos {
namespace operation { // geos.operation
namespace buffer { // geos.operation.buffer

/*
 * A segment from a directed edge which has been assigned a depth value
 * for its sides.
 */
class DepthSegment {

private:
    geom::LineSegment upwardSeg;

public:

    int leftDepth;

    /// @param seg will be copied to private space
    DepthSegment(const geom::LineSegment& seg, int depth)
        :
        upwardSeg(seg),
        leftDepth(depth)
    {
        // input seg is assumed to be normalized
        //upwardSeg.normalize();
    }

    /**
     * Compares a point to a segment for left/right position, 
     * as long as the point lies within the segment Y extent.
     * Otherwise the point is not comparable.
     * If the point is not comparable or it lies on the segment
     * returns 0.
     */
    int 
    comparePointInYExtent(const Coordinate& p, const LineSegment& seg) const
    {
        //-- if point is comparable to segment
        if (p.y >= seg.minY() && p.y <= seg.maxY()) {
          //-- flip sign, since orientation and order relation are opposite
          int orient = seg.orientationIndex(p);
          switch (orient) {
          case Orientation::LEFT: return -1;
          case Orientation::RIGHT: return 1;
          }
          //-- collinear, so indeterminate
        }
        //-- not computable
        return 0;
    }

    /**
     * A comparison operation which orders segments left to right
     * along some horizontal line.
     * If segments don't touch the same line, 
     * or touch at the same point,
     * they are compared in their Y extent.
     *
     * <pre>
     * DS1 < DS2   if   DS1.seg is left of DS2.seg
     * DS1 > DS2   if   DS1.seg is right of DS2.seg
     * </pre>
     *
     * @param other
     * @return
     */
    int
    compareTo(const DepthSegment& other) const
    {
      
        /**
         * If segments are disjoint in X, X values provides ordering.
         * This is the most common case.
         */
        if (upwardSeg.minX() > other.upwardSeg.maxX())
            return 1;
        if (upwardSeg.maxX() < other.upwardSeg.minX())
            return -1;
        /**
         * The segments Y ranges should intersect since they lie on same stabbing line.
         * But check for this and provide a result based on Y ordering
         */
        if (upwardSeg.minY() > other.upwardSeg.maxY())
            return 1;
        if (upwardSeg.maxY() < other.upwardSeg.minY())
            return -1;
        
        /**
         * Check if some segment point is left or right
         * of the other segment in its Y extent.
         */
        int comp00 = comparePointInYExtent(upwardSeg.p0, other.upwardSeg);
        if (comp00 != 0) return comp00;
        int comp01 = comparePointInYExtent(upwardSeg.p1, other.upwardSeg);
        if (comp01 != 0) return comp01;
        //-- negate orientation for other/this checks
        int comp10 = -comparePointInYExtent(other.upwardSeg.p0, upwardSeg);
        if (comp10 != 0) return comp10;
        int comp11 = -comparePointInYExtent(other.upwardSeg.p1, upwardSeg);
        if (comp11 != 0) return comp11;
        
        /**
         * If point checks in Y range are indeterminate,
         * segments touch at a point
         * and lie above and below that point, or are horizontal.
         * Order according to their Y values.
         * (The ordering in this case doesn't matter, it just has to be consistent)
         */
        if (upwardSeg.maxY() > other.upwardSeg.maxY())
            return 1;
        if (upwardSeg.maxY() < other.upwardSeg.maxY())
            return -1;
        
        /**
         * If both are horizontal order by X
         */
        if (upwardSeg.isHorizontal() && other.upwardSeg.isHorizontal()) {
            if (upwardSeg.minX() < other.upwardSeg.minX())
                return -1;
            if (upwardSeg.minX() > other.upwardSeg.minX())
                return 1;
        }
        
        // assert: segments are equal
        return 0;    }
};

struct DepthSegmentLessThan {
    bool
    operator()(const DepthSegment* first, const DepthSegment* second)
    {
        assert(first);
        assert(second);
        if(first->compareTo(*second) < 0) {
            return true;
        }
        else {
            return false;
        }
    }
};

/*public*/
int
SubgraphDepthLocater::getDepth(const Coordinate& p)
{
    std::vector<DepthSegment*> stabbedSegments;
    findStabbedSegments(p, stabbedSegments);

    // if no segments on stabbing line subgraph must be outside all others
    if(stabbedSegments.empty()) {
        return 0;
    }

    DepthSegment *ds = *std::min_element(stabbedSegments.begin(),
        stabbedSegments.end(), DepthSegmentLessThan());
    int ret = ds->leftDepth;

#if GEOS_DEBUG
    std::cerr << "SubgraphDepthLocater::getDepth(" << p.toString() << "): " << ret << std::endl;
#endif

    for(std::vector<DepthSegment*>::iterator
            it = stabbedSegments.begin(), itEnd = stabbedSegments.end();
            it != itEnd;
            ++it) {
        delete *it;
    }

    return ret;
}

/*private*/
void
SubgraphDepthLocater::findStabbedSegments(const Coordinate& stabbingRayLeftPt,
        std::vector<DepthSegment*>& stabbedSegments)
{
    std::size_t size = subgraphs->size();
    for(std::size_t i = 0; i < size; ++i) {
        BufferSubgraph* bsg = (*subgraphs)[i];

        // optimization - don't bother checking subgraphs
        // which the ray does not intersect
        const Envelope* env = bsg->getEnvelope();
        if(stabbingRayLeftPt.y < env->getMinY()
                || stabbingRayLeftPt.y > env->getMaxY()
                || stabbingRayLeftPt.x > env->getMaxX()) {
            continue;
        }

        findStabbedSegments(stabbingRayLeftPt, bsg->getDirectedEdges(),
                            stabbedSegments);
    }
}

/*private*/
void
SubgraphDepthLocater::findStabbedSegments(
    const Coordinate& stabbingRayLeftPt,
    std::vector<DirectedEdge*>* dirEdges,
    std::vector<DepthSegment*>& stabbedSegments)
{

    /*
     * Check all forward DirectedEdges only. This is still general,
     * because each Edge has a forward DirectedEdge.
     */
    for(std::size_t i = 0, n = dirEdges->size(); i < n; ++i) {
        DirectedEdge* de = (*dirEdges)[i];
        if(!de->isForward()) {
            continue;
        }

        const Envelope* env = de->getEdge()->getEnvelope();
        if(stabbingRayLeftPt.y < env->getMinY()
                || stabbingRayLeftPt.y > env->getMaxY()
                || stabbingRayLeftPt.x > env->getMaxX()) {
            continue;
        }

        findStabbedSegments(stabbingRayLeftPt, de, stabbedSegments);
    }
}

/*private*/
void
SubgraphDepthLocater::findStabbedSegments(
    const Coordinate& stabbingRayLeftPt,
    DirectedEdge* dirEdge,
    std::vector<DepthSegment*>& stabbedSegments)
{
    const CoordinateSequence* pts = dirEdge->getEdge()->getCoordinates();

// It seems that LineSegment is *very* slow... undef this
// to see yourself
// LineSegment has been refactored to be mostly inline, still
// it makes copies of the given coordinates, while the 'non-LineSegment'
// based code below uses pointers instead. I'll kip the SKIP_LS
// defined until LineSegment switches to Coordinate pointers instead.
//
#define SKIP_LS 1

    auto n = pts->getSize() - 1;
    for(std::size_t i = 0; i < n; ++i) {
#ifndef SKIP_LS
        seg.p0 = pts->getAt(i);
        seg.p1 = pts->getAt(i + 1);
#if GEOS_DEBUG
        std::cerr << " SubgraphDepthLocater::findStabbedSegments: segment " << i
             << " (" << seg << ") ";
#endif

#else
        const Coordinate* low = &(pts->getAt(i));
        const Coordinate* high = &(pts->getAt(i + 1));
        const Coordinate* swap = nullptr;

#endif

#ifndef SKIP_LS
        // ensure segment always points upwards
        //if (seg.p0.y > seg.p1.y)
        {
            seg.reverse();
#if GEOS_DEBUG
            std::cerr << " reverse (" << seg << ") ";
#endif
        }
#else
        if(low->y > high->y) {
            swap = low;
            low = high;
            high = swap;
        }
#endif

        // skip segment if it is left of the stabbing line
        // skip if segment is above or below stabbing line
#ifndef SKIP_LS
        double maxx = std::max(seg.p0.x, seg.p1.x);
#else
        double maxx = std::max(low->x, high->x);
#endif
        if(maxx < stabbingRayLeftPt.x) {
#if GEOS_DEBUG
            std::cerr << " segment is left to stabbing line, skipping " << std::endl;
#endif
            continue;
        }

        // skip horizontal segments (there will be a non-horizontal
        // one carrying the same depth info
#ifndef SKIP_LS
        if(seg.isHorizontal())
#else
        if(low->y == high->y)
#endif
        {
#if GEOS_DEBUG
            std::cerr << " segment is horizontal, skipping " << std::endl;
#endif
            continue;
        }

        // skip if segment is above or below stabbing line
#ifndef SKIP_LS
        if(stabbingRayLeftPt.y < seg.p0.y ||
                stabbingRayLeftPt.y > seg.p1.y)
#else
        if(stabbingRayLeftPt.y < low->y ||
                stabbingRayLeftPt.y > high->y)
#endif
        {
#if GEOS_DEBUG
            std::cerr << " segment above or below stabbing line, skipping " << std::endl;
#endif
            continue;
        }

        // skip if stabbing ray is right of the segment
#ifndef SKIP_LS
        if(Orientation::index(seg.p0, seg.p1,
#else
        if(Orientation::index(*low, *high,
#endif
                              stabbingRayLeftPt) == Orientation::RIGHT) {
#if GEOS_DEBUG
            std::cerr << " stabbing ray right of segment, skipping" << std::endl;
#endif
            continue;
        }

#ifndef SKIP_LS
        // stabbing line cuts this segment, so record it
        int depth = dirEdge->getDepth(Position::LEFT);
        // if segment direction was flipped, use RHS depth instead
        if(!(seg.p0 == pts->getAt(i)))
            depth = dirEdge->getDepth(Position::RIGHT);
#else
        int depth = swap ?
                    dirEdge->getDepth(Position::RIGHT)
                    :
                    dirEdge->getDepth(Position::LEFT);
#endif

#if GEOS_DEBUG
        std::cerr << " depth: " << depth << std::endl;
#endif

#ifdef SKIP_LS
        seg.p0 = *low;
        seg.p1 = *high;
#endif

        DepthSegment* ds = new DepthSegment(seg, depth);
        stabbedSegments.push_back(ds);
    }
}

} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos
