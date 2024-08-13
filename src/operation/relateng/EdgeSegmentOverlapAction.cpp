/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2001-2002 Vivid Solutions Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: index/chain/MonotoneChainOverlapAction.java rev. 1.6 (JTS-1.10)
 *
 **********************************************************************/

#include <geos/index/chain/MonotoneChainOverlapAction.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/noding/SegmentString.h>
#include <geos/noding/SegmentIntersector.h>
#include <geos/operation/relateng/EdgeSegmentOverlapAction.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/LineSegment.h>
#include <geos/profiler.h>

//#include <stdio.h>

using geos::index::chain::MonotoneChain;
using geos::noding::SegmentString;
using geos::noding::SegmentIntersector;

namespace geos {
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* public override */
void
EdgeSegmentOverlapAction::overlap(
    const MonotoneChain& mc1, std::size_t start1,
    const MonotoneChain& mc2, std::size_t start2)
{
    SegmentString* ss1 = static_cast<SegmentString*>(mc1.getContext());
    SegmentString* ss2 = static_cast<SegmentString*>(mc2.getContext());
    si.processIntersections(ss1, start1, ss2, start2);
}


} // namespace geos.index.chain
} // namespace geos.index
} // namespace geos
