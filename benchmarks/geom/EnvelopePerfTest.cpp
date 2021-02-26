/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2021 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <benchmark/benchmark.h>

#include <geos/geom/Envelope.h>

using geos::geom::Coordinate;
using geos::geom::Envelope;

static void BM_EnvelopeIntersectsEnvelope(benchmark::State& state) {
    Envelope a = Envelope(2, 3, 4, 5);
    Envelope b = Envelope(6, 7, 8, 9);
    for (auto _ : state) {
        benchmark::DoNotOptimize(a.intersects(&b));
    }
}

static void BM_CoordinateEnvelopeIntersectsEnvelope(benchmark::State& state) {
    Coordinate p0(2, 4);
    Coordinate p1(3, 5);
    Coordinate q0(7, 9);
    Coordinate q1(6, 8);

    for (auto _ : state) {
        benchmark::DoNotOptimize(Envelope::intersects(p0, p1, q0, q1));
    }
}

BENCHMARK(BM_EnvelopeIntersectsEnvelope);
BENCHMARK(BM_CoordinateEnvelopeIntersectsEnvelope);

BENCHMARK_MAIN();

