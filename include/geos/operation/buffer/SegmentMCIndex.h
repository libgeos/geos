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

using geos::geom::CoordinateSequence;
using geos::geom::Envelope;
using namespace geos::index;

namespace geos {
namespace operation {
namespace buffer {

class GEOS_DLL SegmentMCIndex {

private:

    strtree::TemplateSTRtree<const index::chain::MonotoneChain*> index;
    std::vector<chain::MonotoneChain> segChains;

    void buildIndex(const CoordinateSequence* segs);

public:

    SegmentMCIndex(const CoordinateSequence* segs);

    void query(const Envelope* env, chain::MonotoneChainSelectAction& action);
};


} // geos.operation.buffer
} // geos.operation
} // geos
