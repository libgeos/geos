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

#include <geos/algorithm/LineIntersector.h>

using geos::geom::Coordinate;
using geos::algorithm::LineIntersector;

static void BM_PointIntersection(benchmark::State& state) {
    Coordinate p1(10, 10);
    Coordinate p2(20, 20);
    Coordinate q1(20, 10);
    Coordinate q2(10, 20);

    for (auto _ : state) {
        LineIntersector li;
        li.computeIntersection(p1, p2, q1, q2);
    }
}

static void BM_Collinear(benchmark::State& state) {
    Coordinate p1(10, 10);
    Coordinate p2(20, 10);
    Coordinate q1(22, 10);
    Coordinate q2(30, 10);

    for (auto _ : state) {
        LineIntersector li;
        li.computeIntersection(p1, p2, q1, q2);
    }
}

BENCHMARK(BM_PointIntersection);
BENCHMARK(BM_Collinear);

BENCHMARK_MAIN();

