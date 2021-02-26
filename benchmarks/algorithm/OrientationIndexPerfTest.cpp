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

#include <geos/geom/Coordinate.h>
#include <geos/algorithm/CGAlgorithmsDD.h>

using geos::geom::Coordinate;
using geos::algorithm::CGAlgorithmsDD;

static void BM_OrientationIndexFilter(benchmark::State& state) {
    Coordinate p0(219.3649559090992, 140.84159161824724);
    Coordinate p1(168.9018919682399, -5.713787599646864);
    Coordinate p(186.80814046338352, 46.28973405831556);

    for (auto _ : state) {
        benchmark::DoNotOptimize(CGAlgorithmsDD::orientationIndexFilter(p0.x, p0.y, p1.x, p1.y, p.x, p.y));
    }
}

static void BM_OrientationIndex(benchmark::State& state) {
    Coordinate p0(219.3649559090992, 140.84159161824724);
    Coordinate p1(168.9018919682399, -5.713787599646864);
    Coordinate p(186.80814046338352, 46.28973405831556);

    for (auto _ : state) {
        benchmark::DoNotOptimize(CGAlgorithmsDD::orientationIndex(p0.x, p0.y, p1.x, p1.y, p.x, p.y));
    }
}

BENCHMARK(BM_OrientationIndexFilter);
BENCHMARK(BM_OrientationIndex);

BENCHMARK_MAIN();

