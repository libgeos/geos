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

#pragma once

#include <geos/export.h>
#include <geos/geom/LineSegment.h>
#include <geos/index/chain/MonotoneChainOverlapAction.h>


// Forward declarations
namespace geos {
namespace index {
namespace chain {
    class MonotoneChain;
}
}
namespace noding {
    class SegmentIntersector;
}
}

namespace geos {
namespace operation { // geos::operation
namespace relateng {  // geos::operation::relateng

/** \brief
 * The action for the internal iterator for performing
 * overlap queries on a MonotoneChain.
 */
class GEOS_DLL EdgeSegmentOverlapAction : public geos::index::chain::MonotoneChainOverlapAction {
    using MonotoneChain = geos::index::chain::MonotoneChain;
    using SegmentIntersector = geos::noding::SegmentIntersector;

private:

    SegmentIntersector& si;


public:

    EdgeSegmentOverlapAction(SegmentIntersector& p_si)
        : si(p_si)
        {}

    void overlap(
        const MonotoneChain& mc1, std::size_t start1,
        const MonotoneChain& mc2, std::size_t start2) override;


};

} // namespace geos::index::chain
} // namespace geos::index
} // namespace geos

