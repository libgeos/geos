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

#pragma once

#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/strtree/TemplateSTRtree.h>


// Forward declarations
namespace geos {
namespace geom {
class CoordinateSequence;
class Envelope;
}
}

namespace geos {
namespace operation {
namespace buffer {

class GEOS_DLL SegmentMCIndex {
    using CoordinateSequence = geos::geom::CoordinateSequence;
    using Envelope = geos::geom::Envelope;
    using MonotoneChain = geos::index::chain::MonotoneChain;
    using MonotoneChainSelectAction = geos::index::chain::MonotoneChainSelectAction;
    template<typename ItemType>
    using TemplateSTRtree = geos::index::strtree::TemplateSTRtree<ItemType>;

private:

    TemplateSTRtree<const MonotoneChain*> index;
    std::vector<MonotoneChain> segChains;

    void buildIndex(const CoordinateSequence* segs);

public:

    SegmentMCIndex(const CoordinateSequence* segs);

    void query(const Envelope* env, MonotoneChainSelectAction& action);
};


} // geos.operation.buffer
} // geos.operation
} // geos
