/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2018 Paul Ramsey <pramsey@cleverlephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: algorithm/Orientation.java @ 2017-09-04
 *
 **********************************************************************/

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

#include <geos/algorithm/Area.h>
#include <geos/algorithm/Orientation.h>
#include <geos/algorithm/CGAlgorithmsDD.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/util/IllegalArgumentException.h>

#include "../util/SimdAvx512.h"

using geos::geom::CoordinateXY;

namespace geos {
namespace algorithm { // geos.algorithm

namespace {

#if GEOS_SIMD_AVX512
/*
 * Finds the index of the highest point of the ring that ends a rising segment,
 * as computed by the first loop of Orientation::isCCW: the last rising
 * endpoint at the maximum height, or 0 if no rising endpoint is as high as
 * the first point. Returns false if any Y ordinate is NaN.
 */
template<std::size_t S>
GEOS_AVX512_TARGET bool
findUpHiAvx512(const double* data, std::size_t npts, std::uint32_t& iUpHi)
{
    namespace simd = geos::util::simd::detail;

    __m512d maxY = _mm512_set1_pd(-std::numeric_limits<double>::infinity());
    __m512i maxIndex = _mm512_setzero_si512();
    __mmask8 nan = 0;

    simd::SegmentBlocks<S> blocks(data, npts);
    while (blocks.next()) {
        const __mmask8 lanes = simd::laneMask(blocks.count());
        const __m512d y0 = blocks.y0;
        const __m512d y1 = blocks.y1;
        nan = static_cast<__mmask8>(nan | _mm512_mask_cmp_pd_mask(lanes, y1, y1, _CMP_UNORD_Q));

        // segments rising to a point at least as high as the lane maximum
        const __mmask8 rising = _mm512_mask_cmp_pd_mask(lanes, y1, y0, _CMP_GT_OQ);
        const __mmask8 higher = _mm512_mask_cmp_pd_mask(rising, y1, maxY, _CMP_GE_OQ);
        const auto end = static_cast<long long>(blocks.start() + 1);
        maxY = _mm512_mask_mov_pd(maxY, higher, y1);
        maxIndex = _mm512_mask_mov_epi64(maxIndex, higher,
            _mm512_add_epi64(_mm512_set1_epi64(end), _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 6, 7)));
    }
    if (nan || std::isnan(data[1])) {
        return false;
    }

    const double y = _mm512_reduce_max_pd(maxY);
    if (!(y >= data[1])) {
        iUpHi = 0;
        return true;
    }
    const __mmask8 atMax = _mm512_cmp_pd_mask(maxY, _mm512_set1_pd(y), _CMP_EQ_OQ);
    iUpHi = static_cast<std::uint32_t>(_mm512_mask_reduce_max_epi64(atMax, maxIndex));
    return true;
}
#endif

bool
findUpHiAvx512(const geom::CoordinateSequence& ring, std::uint32_t& iUpHi)
{
#if GEOS_SIMD_AVX512
    if (geos::util::simd::detail::useAvx512()) {
        switch (geos::util::simd::detail::coordinateStride(ring)) {
            case 2: return findUpHiAvx512<2>(ring.data(), ring.size(), iUpHi);
            case 3: return findUpHiAvx512<3>(ring.data(), ring.size(), iUpHi);
            case 4: return findUpHiAvx512<4>(ring.data(), ring.size(), iUpHi);
        }
    }
#else
    (void) ring; (void) iUpHi;
#endif
    return false;
}

} // anonymous namespace

/* public static */
// inlining this method worsened performance slightly
int
Orientation::index(const geom::CoordinateXY& p1, const geom::CoordinateXY& p2,
                   const geom::CoordinateXY& q)
{
    return CGAlgorithmsDD::orientationIndex(p1, p2, q);
}


/* public static */
bool
Orientation::isCCW(const geom::CoordinateSequence* ring)
{
    // # of points without closing endpoint
    int inPts = static_cast<int>(ring->size()) - 1;
    // sanity check
    if (inPts < 3)
        return false;

    uint32_t nPts = static_cast<uint32_t>(inPts);
    /**
     * Find first highest point after a lower point, if one exists
     * (e.g. a rising segment)
     * If one does not exist, hiIndex will remain 0
     * and the ring must be flat.
     * Note this relies on the convention that
     * rings have the same start and end point.
     */
    const CoordinateXY* upHiPt = &ring->getAt<CoordinateXY>(0);
    const CoordinateXY* upLowPt = &CoordinateXY::getNull();

    uint32_t iUpHi = 0;
    if (findUpHiAvx512(*ring, iUpHi)) {
        if (iUpHi > 0) {
            upHiPt = &ring->getAt<CoordinateXY>(iUpHi);
            upLowPt = &ring->getAt<CoordinateXY>(iUpHi - 1);
        }
    }
    else {
        double prevY = upHiPt->y;
        for (uint32_t i = 1; i <= nPts; i++) {
            double py = ring->getY(i);
            /**
            * If segment is upwards and endpoint is higher, record it
            */
            if (py > prevY && py >= upHiPt->y) {
                iUpHi = i;
                upHiPt = &ring->getAt<CoordinateXY>(i);
                upLowPt = &ring->getAt<CoordinateXY>(i-1);
            }
            prevY = py;
        }
    }
    /**
     * Check if ring is flat and return default value if so
     */
    if (iUpHi == 0) return false;

    /**
     * Find the next lower point after the high point
     * (e.g. a falling segment).
     * This must exist since ring is not flat.
     */
    uint32_t iDownLow = iUpHi;
    do {
        iDownLow = (iDownLow + 1) % nPts;
    } while (iDownLow != iUpHi && ring->getY(iDownLow) == upHiPt->y );

    const CoordinateXY& downLowPt = ring->getAt<CoordinateXY>(iDownLow);
    uint32_t iDownHi = iDownLow > 0 ? iDownLow - 1 : nPts - 1;
    const CoordinateXY& downHiPt = ring->getAt<CoordinateXY>(iDownHi);

    /**
     * Two cases can occur:
     * 1) the hiPt and the downPrevPt are the same.
     *    This is the general position case of a "pointed cap".
     *    The ring orientation is determined by the orientation of the cap
     * 2) The hiPt and the downPrevPt are different.
     *    In this case the top of the cap is flat.
     *    The ring orientation is given by the direction of the flat segment
     */
    if (upHiPt->equals2D(downHiPt)) {
        /**
        * Check for the case where the cap has configuration A-B-A.
        * This can happen if the ring does not contain 3 distinct points
        * (including the case where the input array has fewer than 4 elements), or
        * it contains coincident line segments.
        */
        if (upLowPt->equals2D(*upHiPt) || downLowPt.equals2D(*upHiPt) || upLowPt->equals2D(downLowPt))
            return false;

        /**
        * It can happen that the top segments are coincident.
        * This is an invalid ring, which cannot be computed correctly.
        * In this case the orientation is 0, and the result is false.
        */
        int orientationIndex = index(*upLowPt, *upHiPt, downLowPt);
        return orientationIndex == COUNTERCLOCKWISE;
    }
    else {
        /**
        * Flat cap - direction of flat top determines orientation
        */
        double delX = downHiPt.x - upHiPt->x;
        return delX < 0;
    }
}

/* public static */
bool
Orientation::isCCWArea(const geom::CoordinateSequence* ring)
{
    return algorithm::Area::ofRingSigned(ring) < 0;
}




} // namespace geos.algorithm
} //namespace geos
