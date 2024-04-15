/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2024 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <benchmark/benchmark.h>

#include <BenchmarkUtils.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/operation/distance/DistanceOp.h>

using geos::operation::distance::DistanceOp;

static void BM_PointPointDistance(benchmark::State& state) {
    geos::geom::Envelope e(-100, 100, -100, 100);
    auto points = geos::benchmark::createPoints(e, 100000);

    for (auto _ : state) {
        for (std::size_t i = 0; i < points.size(); i++) {
            for (std::size_t j = 0; i < points.size(); i++) {
                points[i]->distance(points[j].get());
           }
        }
    }
}

BENCHMARK(BM_PointPointDistance);

BENCHMARK_MAIN();

