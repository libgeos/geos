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

#include <geos/operation/overlayng/RobustClipEnvelopeComputer.h>

#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Polygon.h>

#include "../../util/SimdAvx512.h"

namespace geos {      // geos
namespace operation { // geos.operation
namespace overlayng { // geos.operation.overlayng

using namespace geos::geom;

namespace {

#if GEOS_SIMD_AVX512
/*
 * Expands env (non-null) to include the endpoints of each segment of
 * the sequence whose envelope intersects target, following
 * Envelope::intersects(p1, p2) and Envelope::expandToInclude exactly.
 * Returns false (leaving env unchanged) if the result could depend on the
 * order of the points, which is the case only for zero extremes (-0 vs 0).
 */
template<std::size_t S>
GEOS_AVX512_TARGET bool
expandBySegmentsAvx512(const double* data, std::size_t npts, const Envelope& target, Envelope& env)
{
    namespace simd = geos::util::simd::detail;

    const __m512d tminx = _mm512_set1_pd(target.getMinX());
    const __m512d tmaxx = _mm512_set1_pd(target.getMaxX());
    const __m512d tminy = _mm512_set1_pd(target.getMinY());
    const __m512d tmaxy = _mm512_set1_pd(target.getMaxY());

    __m512d minx = _mm512_set1_pd(env.getMinX());
    __m512d maxx = _mm512_set1_pd(env.getMaxX());
    __m512d miny = _mm512_set1_pd(env.getMinY());
    __m512d maxy = _mm512_set1_pd(env.getMaxY());

    simd::SegmentBlocks<S> blocks(data, npts);
    while (blocks.next()) {
        const __mmask8 lanes = simd::laneMask(blocks.count());
        const __m512d ax = blocks.x0;
        const __m512d ay = blocks.y0;
        const __m512d bx = blocks.x1;
        const __m512d by = blocks.y1;

        // target.intersects(a, b)
        const __m512d envminx = _mm512_mask_blend_pd(_mm512_cmp_pd_mask(ax, bx, _CMP_LT_OQ), bx, ax);
        const __m512d envmaxx = _mm512_mask_blend_pd(_mm512_cmp_pd_mask(ax, bx, _CMP_GT_OQ), bx, ax);
        const __m512d envminy = _mm512_mask_blend_pd(_mm512_cmp_pd_mask(ay, by, _CMP_LT_OQ), by, ay);
        const __m512d envmaxy = _mm512_mask_blend_pd(_mm512_cmp_pd_mask(ay, by, _CMP_GT_OQ), by, ay);
        __mmask8 hit = _mm512_mask_cmp_pd_mask(lanes, envminx, tmaxx, _CMP_LE_OQ);
        hit = _mm512_mask_cmp_pd_mask(hit, envmaxx, tminx, _CMP_NLT_UQ);
        hit = _mm512_mask_cmp_pd_mask(hit, envminy, tmaxy, _CMP_NGT_UQ);
        hit = _mm512_mask_cmp_pd_mask(hit, envmaxy, tminy, _CMP_NLT_UQ);
        if (hit == 0) {
            continue;
        }

        // expandToInclude(a), expandToInclude(b): x < min ? x : min ignores NaN
        minx = _mm512_mask_min_pd(minx, hit, ax, minx);
        maxx = _mm512_mask_max_pd(maxx, hit, ax, maxx);
        miny = _mm512_mask_min_pd(miny, hit, ay, miny);
        maxy = _mm512_mask_max_pd(maxy, hit, ay, maxy);
        minx = _mm512_mask_min_pd(minx, hit, bx, minx);
        maxx = _mm512_mask_max_pd(maxx, hit, bx, maxx);
        miny = _mm512_mask_min_pd(miny, hit, by, miny);
        maxy = _mm512_mask_max_pd(maxy, hit, by, maxy);
    }

    const double rminx = _mm512_reduce_min_pd(minx);
    const double rmaxx = _mm512_reduce_max_pd(maxx);
    const double rminy = _mm512_reduce_min_pd(miny);
    const double rmaxy = _mm512_reduce_max_pd(maxy);
    if (rminx == 0 || rmaxx == 0 || rminy == 0 || rmaxy == 0) {
        return false;
    }
    env.expandToInclude(rminx, rminy);
    env.expandToInclude(rmaxx, rmaxy);
    return true;
}
#endif

bool
expandBySegmentsAvx512(const CoordinateSequence& seq, const Envelope& target, Envelope& env)
{
#if GEOS_SIMD_AVX512
    if (!geos::util::simd::detail::useAvx512() || env.isNull() || seq.size() < 2) {
        return false;
    }
    switch (geos::util::simd::detail::coordinateStride(seq)) {
        case 2: return expandBySegmentsAvx512<2>(seq.data(), seq.size(), target, env);
        case 3: return expandBySegmentsAvx512<3>(seq.data(), seq.size(), target, env);
        case 4: return expandBySegmentsAvx512<4>(seq.data(), seq.size(), target, env);
    }
#else
    (void) seq; (void) target; (void) env;
#endif
    return false;
}

} // anonymous namespace

RobustClipEnvelopeComputer::RobustClipEnvelopeComputer(const Envelope* p_targetEnv)
    : targetEnv(p_targetEnv)
    , clipEnv(*p_targetEnv)
{}


/*public static*/
Envelope
RobustClipEnvelopeComputer::getEnvelope(const Geometry* a, const Geometry* b, const Envelope* targetEnv)
{
    RobustClipEnvelopeComputer cec(targetEnv);
    cec.add(a);
    cec.add(b);
    return cec.getEnvelope();
}

/*public*/
Envelope
RobustClipEnvelopeComputer::getEnvelope()
{
    return clipEnv;
}

/*public*/
void
RobustClipEnvelopeComputer::add(const Geometry* g)
{
    if (g == nullptr || g->isEmpty())
        return;

    if (g->getGeometryTypeId() == GEOS_POLYGON)
        addPolygon(static_cast<const Polygon*>(g));
    else if (g->isCollection())
        addCollection(static_cast<const GeometryCollection*>(g));
}

/*private*/
void
RobustClipEnvelopeComputer::addCollection(const GeometryCollection* gc)
{
    for (std::size_t i = 0; i < gc->getNumGeometries(); i++) {
        const Geometry* g = gc->getGeometryN(i);
        add(g);
    }
}

/*private*/
void
RobustClipEnvelopeComputer::addPolygon(const Polygon* poly)
{
    const LinearRing* shell = poly->getExteriorRing();
    addPolygonRing(shell);

    for (std::size_t i = 0; i < poly->getNumInteriorRing(); i++) {
        const LinearRing* hole = poly->getInteriorRingN(i);
        addPolygonRing(hole);
    }
}

/**
* Adds a polygon ring to the graph. Empty rings are ignored.
*/
/*private*/
void
RobustClipEnvelopeComputer::addPolygonRing(const LinearRing* ring)
{
    // don't add empty lines
    if (ring->isEmpty())
      return;

    const CoordinateSequence* seq = ring->getCoordinatesRO();
    if (expandBySegmentsAvx512(*seq, *targetEnv, clipEnv)) {
        return;
    }

    for (std::size_t i = 1; i < seq->size(); i++) {
        addSegment(seq->getAt(i - 1), seq->getAt(i));
    }
}

/*private*/
void
RobustClipEnvelopeComputer::addSegment(const Coordinate& p1, const Coordinate& p2)
{
    if (intersectsSegment(targetEnv, p1, p2)) {
        clipEnv.expandToInclude(p1);
        clipEnv.expandToInclude(p2);
    }
}

/*private*/
bool
RobustClipEnvelopeComputer::intersectsSegment(const Envelope* env, const Coordinate& p1, const Coordinate& p2)
{
    /**
     * This is a crude test of whether segment intersects envelope.
     * It could be refined by checking exact intersection.
     * This could be based on the algorithm in the HotPixel.intersectsScaled method.
     */
    return env->intersects(p1, p2);
}



} // namespace geos.operation.overlayng
} // namespace geos.operation
} // namespace geos
