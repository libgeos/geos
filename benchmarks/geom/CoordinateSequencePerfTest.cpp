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

#include <geos/geom/CoordinateSequence.h>

using geos::geom::CoordinateSequence;

static void BM_Size(benchmark::State& state) {

    CoordinateSequence z(1533);

    for (auto _ : state) {
        benchmark::DoNotOptimize(z.size());
    }
}

BENCHMARK(BM_Size);

BENCHMARK_MAIN();

