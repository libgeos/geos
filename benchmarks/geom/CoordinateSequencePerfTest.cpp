/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2022 ISciences, LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <benchmark/benchmark.h>

#include <geos/geom/CoordinateSequence.h>

using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;

static void BM_Size(benchmark::State& state) {
    CoordinateSequence z(1533);

    for (auto _ : state) {
        benchmark::DoNotOptimize(z.size());
    }
}

static void BM_Initialize(benchmark::State& state) {
    bool hasZ = false;
    bool hasM = false;

    for (auto _ : state) {
        CoordinateSequence seq(1000, hasZ, hasM, true);
    }
}

static void BM_HasRepeatedPoints(benchmark::State & state) {
    CoordinateSequence seq(12345, false, false);
    for (std::size_t i = 0; i < seq.size(); ++i) {
        double di = static_cast<double>(i);
        seq.setAt(Coordinate(di, di + 0.1), i);
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(seq.hasRepeatedPoints());
    }
}

BENCHMARK(BM_Size);
BENCHMARK(BM_Initialize);
BENCHMARK(BM_HasRepeatedPoints);

BENCHMARK_MAIN();

