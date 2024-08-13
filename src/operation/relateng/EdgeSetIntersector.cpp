/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (c) 2024 Martin Davis
 * Copyright (C) 2024 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/geom/Envelope.h>
#include <geos/geom/Geometry.h>
#include <geos/noding/SegmentString.h>
#include <geos/operation/relateng/EdgeSegmentIntersector.h>
#include <geos/operation/relateng/EdgeSetIntersector.h>
#include <geos/operation/relateng/EdgeSegmentOverlapAction.h>
#include <geos/operation/relateng/RelateSegmentString.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainBuilder.h>


using geos::geom::Geometry;
using geos::geom::Envelope;
using geos::noding::SegmentString;
using geos::index::chain::MonotoneChain;
using geos::index::chain::MonotoneChainBuilder;


namespace geos {      // geos
namespace operation { // geos.operation
namespace relateng {  // geos.operation.relateng


/* private */
void
EdgeSetIntersector::addEdges(std::vector<const SegmentString*>& segStrings)
{
    for (const SegmentString* ss : segStrings) {
        addToIndex(ss);
    }
}

/* private */
void
EdgeSetIntersector::addToIndex(const SegmentString* segStr)
{
    std::vector<MonotoneChain> segChains;
    MonotoneChainBuilder::getChains(segStr->getCoordinates(), const_cast<SegmentString*>(segStr), segChains);

    for (MonotoneChain& mc : segChains) {
        if (envelope == nullptr || envelope->intersects(mc.getEnvelope())) {
            mc.setId(idCounter++);
            monoChains.push_back(mc);
            MonotoneChain* mcPtr = &(monoChains.back());
            index.insert(mcPtr->getEnvelope(), mcPtr);
        }
    }
}

/* public */
void
EdgeSetIntersector::process(EdgeSegmentIntersector& intersector)
{
    EdgeSegmentOverlapAction overlapAction(intersector);

    for (const MonotoneChain& queryChain : monoChains) {

        std::vector<const MonotoneChain*> overlapChains;
        index.query(queryChain.getEnvelope(), [&overlapChains](const MonotoneChain* mc) {
            overlapChains.push_back(mc);
            });

        for (const MonotoneChain* testChain : overlapChains) {
            /**
             * following test makes sure we only compare each pair of chains once
             * and that we don't compare a chain to itself
             */
            if (testChain->getId() <= queryChain.getId())
                continue;

            testChain->computeOverlaps(&queryChain, &overlapAction);
            if (intersector.isDone())
                return;
        }
    }
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos


