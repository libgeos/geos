/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/snapround/MCIndexPointSnapper.java r486 (JTS-1.12+)
 *
 **********************************************************************/

#include <geos/noding/snapround/MCIndexPointSnapper.h>
#include <geos/noding/snapround/HotPixel.h>
#include <geos/noding/snapround/SimpleSnapRounder.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/spatialIndex.h>
#include <geos/geom/Envelope.h>
#include <geos/index/chain/MonotoneChainSelectAction.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/util.h>

#include <algorithm>

using namespace geos::index;
using namespace geos::geom;

namespace geos {
namespace noding { // geos.noding
namespace snapround { // geos.noding.snapround

class HotPixelSnapAction: public index::chain::MonotoneChainSelectAction {

public:

    HotPixelSnapAction(HotPixel& nHotPixel,
                       SegmentString* nParentEdge,
                       size_t nVertexIndex)
        :
        MonotoneChainSelectAction(),
        hotPixel(nHotPixel),
        parentEdge(nParentEdge),
        vertexIndex(nVertexIndex),
        isNodeAddedVar(false)
    {}

    /**
    * Reports whether the HotPixel caused a node to be added in any target
    * segmentString (including its own). If so, the HotPixel must be added as a
    * node as well.
    *
    * @return true if a node was added in any target segmentString.
    */
    bool
    isNodeAdded() const
    {
        return isNodeAddedVar;
    }

    /**
    * Check if a segment of the monotone chain intersects
    * the hot pixel vertex and introduce a snap node if so.
    * Optimized to avoid noding segments which
    * contain the vertex (which otherwise
    * would cause every vertex to be noded).
    */
    void
    select(chain::MonotoneChain& mc, size_t startIndex) override
    {
        // This is casting away 'constness'!
        NodedSegmentString& ss = *(static_cast<NodedSegmentString*>(mc.getContext()));

        if (parentEdge == &ss) {
            // exit if hotpixel is equal to endpoint of target segment
            if (startIndex == vertexIndex || startIndex + 1 == vertexIndex)
                return;
        }
        // snap and record if a node was created
        isNodeAddedVar |= hotPixel.addSnappedNode(ss, startIndex);
    }

    void
    select(const LineSegment& ls) override
    {
        ::geos::ignore_unused_variable_warning(ls);
    }

private:
    HotPixel& hotPixel;
    SegmentString* parentEdge;
    size_t vertexIndex;
    bool isNodeAddedVar;

    // Declare type as noncopyable
    HotPixelSnapAction(const HotPixelSnapAction& other) = delete;
    HotPixelSnapAction& operator=(const HotPixelSnapAction& rhs) = delete;
};

class MCIndexPointSnapperVisitor: public ItemVisitor {

public:
    MCIndexPointSnapperVisitor(const Envelope& nPixelEnv, HotPixelSnapAction& nAction)
        :
        pixelEnv(nPixelEnv),
        action(nAction)
    {}

    ~MCIndexPointSnapperVisitor() override {}

    void
    visitItem(void* item) override
    {
        chain::MonotoneChain& testChain =
            *(static_cast<chain::MonotoneChain*>(item));
        testChain.select(pixelEnv, action);
    }

private:
    const Envelope& pixelEnv;
    chain::MonotoneChainSelectAction& action;

    // Declare type as noncopyable
    MCIndexPointSnapperVisitor(const MCIndexPointSnapperVisitor& other);
    MCIndexPointSnapperVisitor& operator=(const MCIndexPointSnapperVisitor& rhs);
};

/* public */
bool
MCIndexPointSnapper::snap(HotPixel& hotPixel,
                          SegmentString* parentEdge,
                          size_t vertexIndex)
{
    const Envelope& pixelEnv = hotPixel.getSafeEnvelope();
    HotPixelSnapAction hotPixelSnapAction(hotPixel, parentEdge, vertexIndex);
    MCIndexPointSnapperVisitor visitor(pixelEnv, hotPixelSnapAction);

    index.query(&pixelEnv, visitor);

    return hotPixelSnapAction.isNodeAdded();
}

} // namespace geos.noding.snapround
} // namespace geos.noding
} // namespace geos

