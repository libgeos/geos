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
    ChainBuilder() : m_prev(nullptr), m_i(0), m_quadrant(-1) {}

    void filter_ro(const Coordinate* c) override final {
        process(c);

        m_prev = c;
        m_i++;
    }

    std::vector<std::size_t> getEnds() {
        finishChain();
        return std::move(m_ends);
    }

private:
    void finishChain() {
        if ( m_i == 0 ) return;
        m_ends.push_back(m_i - 1);
    }

    void process(const Coordinate* curr) {
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

    std::vector<std::size_t> m_ends;
    const Coordinate* m_prev;
    std::size_t m_i;
    int m_quadrant;
};


/* static public */
void
MonotoneChainBuilder::getChains(const CoordinateSequence* pts, void* context,
                                std::vector<MonotoneChain>& mcList) {
    ChainBuilder builder;
    pts->apply_ro(&builder);

    size_t chainStart = 0;
    for (size_t chainEnd : builder.getEnds()) {
        mcList.emplace_back(*pts, chainStart, chainEnd, context);
        chainStart = chainEnd;
    }
}

} // namespace geos.index.chain
} // namespace geos.index
} // namespace geos

