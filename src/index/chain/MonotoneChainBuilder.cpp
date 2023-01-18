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
 * Last port: index/chain/MonotoneChainBuilder.java r388 (JTS-1.12)
 *
 **********************************************************************/

#include <geos/index/chain/MonotoneChainBuilder.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/Quadrant.h>


#include <cassert>
#include <cstdio>
#include <vector>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#if GEOS_DEBUG
#include <iostream>
#endif


using namespace geos::geom;

namespace geos {
namespace index { // geos.index
namespace chain { // geos.index.chain

/** \brief
 * Finds the index of the last point in each monotone chain
 * of the provided coordinate sequence.
 */
class ChainBuilder : public CoordinateFilter {
public:
    ChainBuilder(const CoordinateSequence* pts, void* context, std::vector<MonotoneChain> & list) :
     m_prev(nullptr),
     m_i(0),
     m_quadrant(-1),
     m_start(0),
     m_seq(pts),
     m_context(context),
     m_list(list) {}

    void filter_ro(const CoordinateXY* c) override {
        process(c);

        m_prev = c;
        m_i++;
    }

    void finish() {
        finishChain();
    }

private:
    void finishChain() {
        if ( m_i == 0 ) return;
        std::size_t chainEnd = m_i - 1;
        m_list.emplace_back(*m_seq, m_start, chainEnd, m_context);
        m_start = chainEnd;
    }

    void process(const CoordinateXY* curr) {
        if (m_prev == nullptr || curr->equals2D(*m_prev)) {
            return;
        }

        int currQuad = Quadrant::quadrant(*m_prev, *curr);

        if (m_quadrant < 0) {
            m_quadrant = currQuad;
        }

        if (currQuad != m_quadrant) {
            finishChain();
            m_quadrant = currQuad;
        }
    }

    const CoordinateXY* m_prev;
    std::size_t m_i;
    int m_quadrant;
    std::size_t m_start;
    const CoordinateSequence* m_seq;
    void* m_context;
    std::vector<MonotoneChain>& m_list;
};


/* static public */
void
MonotoneChainBuilder::getChains(const CoordinateSequence* pts, void* context,
                                std::vector<MonotoneChain>& mcList) {
    ChainBuilder builder(pts, context, mcList);
    pts->apply_ro(&builder);
    builder.finish();
}

} // namespace geos.index.chain
} // namespace geos.index
} // namespace geos

