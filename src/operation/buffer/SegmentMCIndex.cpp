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

#include <geos/operation/buffer/SegmentMCIndex.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Envelope.h>
#include <geos/index/ItemVisitor.h>
#include <geos/index/chain/MonotoneChainBuilder.h>
#include <geos/index/chain/MonotoneChainSelectAction.h>

using geos::geom::CoordinateSequence;
using geos::geom::Envelope;
using namespace geos::index;

namespace geos {
namespace operation {
namespace buffer {

/* public */
SegmentMCIndex::SegmentMCIndex(const CoordinateSequence* segs)
{
    buildIndex(segs);
}

/* private */
void
SegmentMCIndex::buildIndex(const CoordinateSequence* segs)
{
    chain::MonotoneChainBuilder::getChains(segs, nullptr, segChains);
    for (chain::MonotoneChain& mc : segChains) {
        index.insert(&(mc.getEnvelope()), &mc);
    }
    return;
}

/* public */
void
SegmentMCIndex::query(const Envelope* env, chain::MonotoneChainSelectAction& action)
{
    index.query(*env, [&env, &action](const chain::MonotoneChain* indexChain) {
            indexChain->select(*env, action);
            return true;
            }
        );
}


} // namespace geos.operation.buffer
} // namespace geos.operation
} // namespace geos

