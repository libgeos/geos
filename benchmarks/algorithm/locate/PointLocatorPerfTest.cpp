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

#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/algorithm/locate/SimplePointInAreaLocator.h>

#include <BenchmarkUtils.h>

using geos::algorithm::locate::SimplePointInAreaLocator;
using geos::algorithm::locate::IndexedPointInAreaLocator;

auto nPtsRange = benchmark::CreateRange(4, 8000, 2);
auto nTestsRange = benchmark::CreateDenseRange(1, 20, 1);

template<class Locator>
static void BM_PointInAreaLocator(benchmark::State& state) {
    std::default_random_engine eng(12345);

    auto nRingPts = static_cast<std::size_t>(state.range(0));
    auto nTestPts = static_cast<std::size_t>(state.range(1));

    auto geom = geos::benchmark::createSineStar({0, 0}, 100, nRingPts);

    auto test_pts = geos::benchmark::createRandomCoords(*geom->getEnvelopeInternal(), nTestPts, eng);

    for (auto _ : state) {
        Locator loc(*geom);
        for (const auto& coord : test_pts->items<geos::geom::CoordinateXY>()) {
            loc.locate(&coord);
        }
    }
}

BENCHMARK_TEMPLATE(BM_PointInAreaLocator, IndexedPointInAreaLocator)->ArgsProduct({nPtsRange, nTestsRange});
BENCHMARK_TEMPLATE(BM_PointInAreaLocator, SimplePointInAreaLocator)->ArgsProduct({nPtsRange, nTestsRange});

BENCHMARK_MAIN();
