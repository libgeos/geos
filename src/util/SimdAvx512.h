/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Internal helpers for runtime-dispatched AVX-512 code paths.
 * This header is not installed.
 *
 * Kernels are compiled with a function-level target attribute so that
 * the rest of the library does not require AVX-512, and are only called
 * when useAvx512() returns true. Every kernel must produce results
 * identical to the scalar code it replaces.
 *
 **********************************************************************/

#pragma once

#include <geos/geom/CoordinateSequence.h>

#include <atomic>
#include <cstddef>
#include <cstdint>

#if !defined(GEOS_DISABLE_AVX512) && (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
#define GEOS_SIMD_AVX512 1
#else
#define GEOS_SIMD_AVX512 0
#endif

#if GEOS_SIMD_AVX512
#include <immintrin.h>
#define GEOS_AVX512_TARGET __attribute__((target("avx512f,avx512dq,avx512vl,avx512bw")))
#endif

namespace geos {
namespace util {
namespace simd {
namespace detail {

extern std::atomic<bool> avx512Enabled;

inline bool
useAvx512()
{
#if GEOS_SIMD_AVX512
    return avx512Enabled.load(std::memory_order_relaxed);
#else
    return false;
#endif
}

/// Number of doubles between consecutive coordinates of a sequence.
inline std::size_t
coordinateStride(const geom::CoordinateSequence& seq)
{
    switch(seq.getCoordinateType()) {
        case geom::CoordinateType::XY: return 2;
        case geom::CoordinateType::XYZM: return 4;
        default: return 3;
    }
}

/// Index of the lowest set bit of a non-zero value.
inline unsigned
countTrailingZeros(std::uint64_t v)
{
#if defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned>(__builtin_ctzll(v));
#else
    unsigned n = 0;
    while((v & 1) == 0) {
        v >>= 1;
        n++;
    }
    return n;
#endif
}

/// Number of set bits.
inline unsigned
popCount(std::uint64_t v)
{
#if defined(__GNUC__) || defined(__clang__)
    return static_cast<unsigned>(__builtin_popcountll(v));
#else
    unsigned n = 0;
    while(v != 0) {
        v &= v - 1;
        n++;
    }
    return n;
#endif
}

#if GEOS_SIMD_AVX512

/**
 * Loads the 8 doubles at p + offset, of which only those before p + total
 * are read; the others are zero. No pointer beyond p + total is formed.
 */
GEOS_AVX512_TARGET inline __m512d
loadPartial(const double* p, std::size_t total, std::size_t offset)
{
    if(total >= offset + 8) {
        return _mm512_loadu_pd(p + offset);
    }
    if(total <= offset) {
        return _mm512_setzero_pd();
    }
    return _mm512_maskz_loadu_pd(static_cast<__mmask8>((1u << (total - offset)) - 1u), p + offset);
}

/**
 * Loads the X and Y ordinates of `count` (1-8) consecutive coordinates
 * stored with stride `S`. Lanes at or beyond `count` are zero, and no
 * memory beyond the last requested coordinate is read.
 */
template<std::size_t S>
GEOS_AVX512_TARGET inline void
loadXY(const double* p, std::size_t count, __m512d& x, __m512d& y)
{
    static_assert(S >= 2 && S <= 4, "unsupported stride");
    const std::size_t total = S * count;

    if(S == 2) {
        const __m512d v0 = loadPartial(p, total, 0);
        const __m512d v1 = loadPartial(p, total, 8);
        x = _mm512_permutex2var_pd(v0, _mm512_setr_epi64(0, 2, 4, 6, 8, 10, 12, 14), v1);
        y = _mm512_permutex2var_pd(v0, _mm512_setr_epi64(1, 3, 5, 7, 9, 11, 13, 15), v1);
    }
    else if(S == 3) {
        const __m512d v0 = loadPartial(p, total, 0);
        const __m512d v1 = loadPartial(p, total, 8);
        const __m512d v2 = loadPartial(p, total, 16);
        const __m512d tx = _mm512_permutex2var_pd(v0, _mm512_setr_epi64(0, 3, 6, 9, 12, 15, 0, 0), v1);
        x = _mm512_permutex2var_pd(tx, _mm512_setr_epi64(0, 1, 2, 3, 4, 5, 10, 13), v2);
        const __m512d ty = _mm512_permutex2var_pd(v0, _mm512_setr_epi64(1, 4, 7, 10, 13, 0, 0, 0), v1);
        y = _mm512_permutex2var_pd(ty, _mm512_setr_epi64(0, 1, 2, 3, 4, 8, 11, 14), v2);
    }
    else {
        const __m512d v0 = loadPartial(p, total, 0);
        const __m512d v1 = loadPartial(p, total, 8);
        const __m512d v2 = loadPartial(p, total, 16);
        const __m512d v3 = loadPartial(p, total, 24);
        const __m512i ix = _mm512_setr_epi64(0, 4, 8, 12, 0, 4, 8, 12);
        const __m512i iy = _mm512_setr_epi64(1, 5, 9, 13, 1, 5, 9, 13);
        x = _mm512_mask_blend_pd(0xF0, _mm512_permutex2var_pd(v0, ix, v1), _mm512_permutex2var_pd(v2, ix, v3));
        y = _mm512_mask_blend_pd(0xF0, _mm512_permutex2var_pd(v0, iy, v1), _mm512_permutex2var_pd(v2, iy, v3));
    }
}

/**
 * Iterates over the segments of a sequence of `npts` coordinates with
 * stride `S`, eight at a time, loading each coordinate once.
 * After next(), lanes [0, count()) of x0/y0 and x1/y1 hold the start and
 * end points of segments [start(), start() + count()).
 */
template<std::size_t S>
class SegmentBlocks {
public:
    GEOS_AVX512_TARGET SegmentBlocks(const double* p_data, std::size_t p_npts)
        : data(p_data)
        , npts(p_npts)
        , nseg(p_npts > 0 ? p_npts - 1 : 0)
        , k(0)
        , n(0)
    {
        loadXY<S>(data, npts < 8 ? npts : 8, nextX, nextY);
    }

    GEOS_AVX512_TARGET bool next()
    {
        k += n;
        if(k >= nseg) {
            return false;
        }
        n = nseg - k < 8 ? nseg - k : 8;

        x0 = nextX;
        y0 = nextY;
        const std::size_t following = k + 8;
        if(following < npts) {
            const std::size_t c = npts - following;
            loadXY<S>(data + following * S, c < 8 ? c : 8, nextX, nextY);
        }
        else {
            nextX = _mm512_setzero_pd();
            nextY = _mm512_setzero_pd();
        }
        // shift in the first point of the following block
        x1 = _mm512_castsi512_pd(_mm512_alignr_epi64(_mm512_castpd_si512(nextX), _mm512_castpd_si512(x0), 1));
        y1 = _mm512_castsi512_pd(_mm512_alignr_epi64(_mm512_castpd_si512(nextY), _mm512_castpd_si512(y0), 1));
        return true;
    }

    std::size_t start() const { return k; }
    std::size_t count() const { return n; }

    __m512d x0, y0, x1, y1;

private:
    const double* data;
    std::size_t npts;
    std::size_t nseg;
    std::size_t k;
    std::size_t n;
    __m512d nextX, nextY;
};

/// Mask with the low `count` (0-8) bits set.
inline __mmask8
laneMask(std::size_t count)
{
    return count >= 8 ? static_cast<__mmask8>(0xFF) : static_cast<__mmask8>((1u << count) - 1u);
}

#endif // GEOS_SIMD_AVX512

} // namespace geos::util::simd::detail
} // namespace geos::util::simd
} // namespace geos::util
} // namespace geos
