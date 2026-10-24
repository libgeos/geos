/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2020 Paul Ramsey <pramsey@cleverelephant.ca>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/operation/overlayng/RingClipper.h>

#include "../../util/SimdAvx512.h"

#include <cstdint>
#include <vector>

using geos::geom::CoordinateSequence;
using geos::geom::CoordinateXYZM;

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng


/*public*/
std::unique_ptr<CoordinateSequence>
RingClipper::clip(const CoordinateSequence* cs) const
{
    std::unique_ptr<CoordinateSequence> pts;
    for (int edgeIndex = 0; edgeIndex < 4; edgeIndex++) {
        bool closeRing = (edgeIndex == 3);
        pts = clipToBoxEdge(cs, edgeIndex, closeRing);
        if (pts->size() == 0)
            return pts;
        cs = pts.get();
    }
    return pts;
}

namespace {

using geos::util::simd::detail::countTrailingZeros;
using geos::util::simd::detail::popCount;

/*
 * Per-point flags for one clipping pass: whether each point is inside
 * the clip edge, and whether it repeats the XY of the previous point.
 */
class EdgePointFlags {
public:
    explicit EdgePointFlags(std::size_t n)
        : nWords((n + 63) / 64)
        , bits(2 * nWords, 0)
    {}

    std::uint64_t* insideWords() { return bits.data(); }
    std::uint64_t* repeatedWords() { return bits.data() + nWords; }

    bool isInside(std::size_t i) const { return test(0, i); }
    bool isRepeated(std::size_t i) const { return test(nWords, i); }

    void setInside(std::size_t i) { set(0, i); }
    void setRepeated(std::size_t i) { set(nWords, i); }

    /// First index in [i, end) whose point is outside, or end.
    std::size_t nextOutside(std::size_t i, std::size_t end) const {
        return nextSet(0, i, end, true);
    }

    /// First index in [i, end) whose point is repeated, or end.
    std::size_t nextRepeated(std::size_t i, std::size_t end) const {
        return nextSet(nWords, i, end, false);
    }

    /// Upper bound on the number of points produced by clipping (n points).
    std::size_t maxClippedSize(std::size_t n) const {
        // each inside point is copied, and each change between inside
        // and outside adds an intersection point
        std::size_t count = 0;
        std::size_t changes = 0;
        std::uint64_t prevBit = isInside(n - 1);
        for (std::size_t w = 0; w < nWords; w++) {
            std::uint64_t word = bits[w];
            std::size_t nbits = (w == nWords - 1 && (n & 63) != 0) ? (n & 63) : 64;
            std::uint64_t valid = nbits == 64 ? ~std::uint64_t{0} : (std::uint64_t{1} << nbits) - 1;
            std::uint64_t shifted = (word << 1) | prevBit;
            count += popCount(word);
            changes += popCount((word ^ shifted) & valid);
            prevBit = (word >> (nbits - 1)) & 1;
        }
        return count + changes + 1;
    }

private:
    std::size_t nWords;
    std::vector<std::uint64_t> bits;

    bool test(std::size_t base, std::size_t i) const {
        return (bits[base + (i >> 6)] >> (i & 63)) & 1;
    }

    void set(std::size_t base, std::size_t i) {
        bits[base + (i >> 6)] |= std::uint64_t{1} << (i & 63);
    }

    std::size_t nextSet(std::size_t base, std::size_t i, std::size_t end, bool invert) const {
        while (i < end) {
            std::uint64_t w = bits[base + (i >> 6)];
            if (invert) {
                w = ~w;
            }
            w &= ~std::uint64_t{0} << (i & 63);
            if (w != 0) {
                std::size_t found = (i & ~std::size_t{63}) + countTrailingZeros(w);
                return found < end ? found : end;
            }
            i = (i & ~std::size_t{63}) + 64;
        }
        return end;
    }
};

#if GEOS_SIMD_AVX512
/*
 * Sets the inside flag of each point for which (useY ? y : x) is greater than
 * `value` (or less than, if `less` is set), and the repeated flag of each point
 * with the same XY as its predecessor.
 */
template<std::size_t S>
GEOS_AVX512_TARGET void
classifyAvx512(const double* data, std::size_t n, bool useY, bool less, double value,
               EdgePointFlags& flags)
{
    namespace simd = geos::util::simd::detail;

    // a < b is evaluated as -a > -b; negation is exact and preserves NaN
    const __m512d sign = _mm512_set1_pd(less ? -1.0 : 1.0);
    const __m512d threshold = _mm512_mul_pd(_mm512_set1_pd(value), sign);

    auto* inside = reinterpret_cast<std::uint8_t*>(flags.insideWords());
    auto* repeated = reinterpret_cast<std::uint8_t*>(flags.repeatedWords());

    __m512d prevX = _mm512_setzero_pd();
    __m512d prevY = _mm512_setzero_pd();
    for (std::size_t i = 0; i < n; i += 8) {
        const std::size_t count = n - i < 8 ? n - i : 8;
        const __mmask8 lanes = simd::laneMask(count);

        __m512d x, y;
        simd::loadXY<S>(data + i * S, count, x, y);

        const __m512d ord = _mm512_mul_pd(useY ? y : x, sign);
        inside[i / 8] = _mm512_mask_cmp_pd_mask(lanes, ord, threshold, _CMP_GT_OQ);

        // shift in the last point of the previous block
        const __m512d px = _mm512_castsi512_pd(_mm512_alignr_epi64(
            _mm512_castpd_si512(x), _mm512_castpd_si512(prevX), 7));
        const __m512d py = _mm512_castsi512_pd(_mm512_alignr_epi64(
            _mm512_castpd_si512(y), _mm512_castpd_si512(prevY), 7));
        __mmask8 rep = _mm512_mask_cmp_pd_mask(lanes, x, px, _CMP_EQ_OQ) &
                       _mm512_mask_cmp_pd_mask(lanes, y, py, _CMP_EQ_OQ);
        if (i == 0) {
            rep &= static_cast<__mmask8>(0xFE);
        }
        repeated[i / 8] = rep;

        prevX = x;
        prevY = y;
    }
}
#endif

/*
 * Computes the flags with AVX-512, if available.
 * Returns false if the scalar path must be used.
 */
bool
classifyAvx512(const CoordinateSequence* pts, bool useY, bool less, double value,
               EdgePointFlags& flags)
{
#if GEOS_SIMD_AVX512
    if (geos::util::simd::detail::useAvx512()) {
        switch (geos::util::simd::detail::coordinateStride(*pts)) {
            case 2: classifyAvx512<2>(pts->data(), pts->size(), useY, less, value, flags); return true;
            case 3: classifyAvx512<3>(pts->data(), pts->size(), useY, less, value, flags); return true;
            case 4: classifyAvx512<4>(pts->data(), pts->size(), useY, less, value, flags); return true;
        }
    }
#else
    (void) pts; (void) useY; (void) less; (void) value; (void) flags;
#endif
    return false;
}

} // anonymous namespace

/*private*/
std::unique_ptr<CoordinateSequence>
RingClipper::clipToBoxEdge(const CoordinateSequence* pts, int edgeIndex, bool closeRing) const
{
    // TODO: is it possible to avoid copying array 4 times?
    auto ptsClip = std::make_unique<CoordinateSequence>(0, pts->hasZ(), pts->hasM());

    const std::size_t n = pts->size();
    if (n == 0) {
        return ptsClip;
    }

    EdgePointFlags flags(n);
    bool classified = false;
    if (!clipEnv.isNull()) {
        bool useY = (edgeIndex == BOX_BOTTOM || edgeIndex == BOX_TOP);
        bool less = (edgeIndex == BOX_RIGHT || edgeIndex == BOX_TOP);
        double edgeValue;
        switch (edgeIndex) {
            case BOX_BOTTOM: edgeValue = clipEnv.getMinY(); break;
            case BOX_RIGHT: edgeValue = clipEnv.getMaxX(); break;
            case BOX_TOP: edgeValue = clipEnv.getMaxY(); break;
            case BOX_LEFT:
            default: edgeValue = clipEnv.getMinX();
        }
        classified = classifyAvx512(pts, useY, less, edgeValue, flags);
    }
    if (!classified) {
        for (std::size_t i = 0; i < n; i++) {
            const CoordinateXY& p = pts->getAt<CoordinateXY>(i);
            if (isInsideEdge(p, edgeIndex)) {
                flags.setInside(i);
            }
            if (i > 0 && p.equals2D(pts->getAt<CoordinateXY>(i - 1))) {
                flags.setRepeated(i);
            }
        }
    }

    ptsClip->reserve(flags.maxClippedSize(n));

    // p0 is the point preceding p1 (initially the last point)
    bool isInside0 = flags.isInside(n - 1);
    std::size_t i = 0;
    while (i < n) {
        bool isInside1 = flags.isInside(i);

        if (isInside0 && isInside1) {
            // Copy the run of inside points starting at i. Each point would be
            // added with allowRepeated=false; since the previous point is already
            // the last point added, only points repeating their predecessor are
            // dropped. (At i == 0 nothing has been added yet, and point 0 is never
            // flagged as repeated.)
            std::size_t runEnd = flags.nextOutside(i, n);
            while (i < runEnd) {
                while (i < runEnd && flags.isRepeated(i)) {
                    i++;
                }
                if (i == runEnd) {
                    break;
                }
                std::size_t copyEnd = flags.nextRepeated(i + 1, runEnd);
                ptsClip->add(*pts, i, copyEnd - 1);
                i = copyEnd;
            }
            continue;
        }

        const CoordinateXY& p0 = pts->getAt<CoordinateXY>(i == 0 ? n - 1 : i - 1);
        const CoordinateXY& p1 = pts->getAt<CoordinateXY>(i);
        if (isInside1) {
            // p0 is outside
            CoordinateXY intPt;
            intersection(p0, p1, edgeIndex, intPt);
            ptsClip->add(intPt, false);

            CoordinateXYZM p1zm;
            pts->getAt(i, p1zm);
            ptsClip->add(p1zm, false);
        }
        else if (isInside0) {
            CoordinateXY intPt;
            intersection(p0, p1, edgeIndex, intPt);
            ptsClip->add(intPt, false);
        }

        // else p0-p1 is outside box, so it is dropped
        isInside0 = isInside1;
        i++;
    }

    // add closing point if required
    if (closeRing) {
        ptsClip->closeRing();
    }

    return ptsClip;
}

/*private*/
void
RingClipper::intersection(const CoordinateXY& a, const CoordinateXY& b, int edgeIndex, CoordinateXY& rsltPt) const
{
    switch (edgeIndex) {
    case BOX_BOTTOM:
        rsltPt = CoordinateXY(intersectionLineY(a, b, clipEnv.getMinY()), clipEnv.getMinY());
        break;
    case BOX_RIGHT:
        rsltPt = CoordinateXY(clipEnv.getMaxX(), intersectionLineX(a, b, clipEnv.getMaxX()));
        break;
    case BOX_TOP:
        rsltPt = CoordinateXY(intersectionLineY(a, b, clipEnv.getMaxY()), clipEnv.getMaxY());
        break;
    case BOX_LEFT:
    default:
        rsltPt = Coordinate(clipEnv.getMinX(), intersectionLineX(a, b, clipEnv.getMinX()));
    }
    return;
}

/*private*/
double
RingClipper::intersectionLineY(const CoordinateXY& a, const CoordinateXY& b, double y)
{
    double m = (b.x - a.x) / (b.y - a.y);
    double intercept = (y - a.y) * m;
    return a.x + intercept;
}

/*private*/
double
RingClipper::intersectionLineX(const CoordinateXY& a, const CoordinateXY& b, double x)
{
    double m = (b.y - a.y) / (b.x - a.x);
    double intercept = (x - a.x) * m;
    return a.y + intercept;
}

/*private*/
bool
RingClipper::isInsideEdge(const CoordinateXY& p, int edgeIndex) const
{
    if (clipEnv.isNull()) {
        return false;
    }

    bool isInside = false;
    switch (edgeIndex) {
    case BOX_BOTTOM: // bottom
        isInside = p.y > clipEnv.getMinY();
        break;
    case BOX_RIGHT: // right
        isInside = p.x < clipEnv.getMaxX();
        break;
    case BOX_TOP: // top
        isInside = p.y < clipEnv.getMaxY();
        break;
    case BOX_LEFT:
    default: // left
        isInside = p.x > clipEnv.getMinX();
    }
    return isInside;
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
