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

#include "../../util/SimdAvx512.h"

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


#if GEOS_SIMD_AVX512
/*
 * Finds the monotone chains of a sequence exactly as ChainBuilder does,
 * computing the quadrants of eight segments at a time.
 */
template<std::size_t S>
GEOS_AVX512_TARGET void
getChainsAvx512(const CoordinateSequence* pts, void* context,
                std::vector<MonotoneChain>& mcList)
{
    namespace simd = geos::util::simd::detail;

    const std::size_t n = pts->size();
    if (n == 0) {
        return;
    }
    const double* data = pts->data();

    std::size_t start = 0;
    // quadrant of the last non-repeated segment, encoded as east * 2 + north,
    // or -1 before the first one
    int quadrant = -1;

    simd::SegmentBlocks<S> blocks(data, n);
    while (blocks.next()) {
        // segment k + lane runs from point k + lane to point k + lane + 1
        const std::size_t k = blocks.start();
        const std::size_t count = blocks.count();
        const __mmask8 lanes = simd::laneMask(count);
        const __m512d x0 = blocks.x0;
        const __m512d y0 = blocks.y0;
        const __m512d x1 = blocks.x1;
        const __m512d y1 = blocks.y1;

        const unsigned repeated = _mm512_mask_cmp_pd_mask(lanes, x1, x0, _CMP_EQ_OQ) &
                                  _mm512_mask_cmp_pd_mask(lanes, y1, y0, _CMP_EQ_OQ);
        const unsigned east = _mm512_mask_cmp_pd_mask(lanes, x1, x0, _CMP_GE_OQ);
        const unsigned north = _mm512_mask_cmp_pd_mask(lanes, y1, y0, _CMP_GE_OQ);

        if (repeated == 0 && quadrant >= 0) {
            const unsigned prevEast = (east << 1) | static_cast<unsigned>(quadrant >> 1);
            const unsigned prevNorth = (north << 1) | static_cast<unsigned>(quadrant & 1);
            unsigned change = ((east ^ prevEast) | (north ^ prevNorth)) & lanes;
            while (change) {
                const std::size_t chainEnd = k + simd::countTrailingZeros(change);
                mcList.emplace_back(*pts, start, chainEnd, context);
                start = chainEnd;
                change &= change - 1;
            }
            const std::size_t last = count - 1;
            quadrant = static_cast<int>(((east >> last) & 1) * 2 + ((north >> last) & 1));
        }
        else {
            for (std::size_t lane = 0; lane < count; lane++) {
                if ((repeated >> lane) & 1) {
                    continue;
                }
                const int q = static_cast<int>(((east >> lane) & 1) * 2 + ((north >> lane) & 1));
                if (quadrant < 0) {
                    quadrant = q;
                }
                if (q != quadrant) {
                    const std::size_t chainEnd = k + lane;
                    mcList.emplace_back(*pts, start, chainEnd, context);
                    start = chainEnd;
                    quadrant = q;
                }
            }
        }
    }

    mcList.emplace_back(*pts, start, n - 1, context);
}
#endif

/* static public */
void
MonotoneChainBuilder::getChains(const CoordinateSequence* pts, void* context,
                                std::vector<MonotoneChain>& mcList) {
#if GEOS_SIMD_AVX512
    if (geos::util::simd::detail::useAvx512()) {
        switch (geos::util::simd::detail::coordinateStride(*pts)) {
            case 2: getChainsAvx512<2>(pts, context, mcList); return;
            case 3: getChainsAvx512<3>(pts, context, mcList); return;
            case 4: getChainsAvx512<4>(pts, context, mcList); return;
        }
    }
#endif

    ChainBuilder builder(pts, context, mcList);
    pts->apply_ro(&builder);
    builder.finish();
}

} // namespace geos.index.chain
} // namespace geos.index
} // namespace geos

