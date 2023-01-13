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
#include <geos/geom/Envelope.h>

#include <BenchmarkUtils.h>

#include <array>
#include <random>

using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYM;
using geos::geom::CoordinateXYZM;
using geos::geom::CoordinateSequence;
using geos::geom::Envelope;
using geos::algorithm::LineIntersector;

template<typename CoordType>
static void BM_PointIntersection(benchmark::State& state) {
    std::size_t n = 1000;
    std::size_t seed = 12345;

    std::default_random_engine e(seed);

    std::array<Envelope, 4> envelopes {
        Envelope{-1, 0, 0, 1},  // NW
        Envelope{0, 1, 0, 1},   // NE
        Envelope{-1, 0, -1, 0}, // SW
        Envelope{0, 1, -1, 0}   // SE
    };

    std::array<CoordinateSequence, 4> pts;
    for (std::size_t i = 0; i < envelopes.size(); i++) {
        pts[i] = *geos::benchmark::createRandomCoords(envelopes[i], n, e);
    }

    LineIntersector li;
    std::size_t i = 0;
    for (auto _ : state) {
        const auto& p1 = pts[0].getAt<CoordType>(i);
        const auto& p2 = pts[2].getAt<CoordType>(i);
        const auto& q1 = pts[1].getAt<CoordType>(i);
        const auto& q2 = pts[3].getAt<CoordType>(i);

        li.computeIntersection(p1, p2, q1, q2);
        li.computeIntersection(p2, p1, q2, q1);
        li.computeIntersection(q2, p1, p2, q1);
        li.computeIntersection(q1, p1, p2, q2);

        i++;
        if (i > n) {
            i = 0;
        }
    }
}

template<typename CoordType>
static void BM_Collinear(benchmark::State& state) {
    CoordinateXYZM p1_xyzm(10, 10, 1, 8);
    CoordinateXYZM p2_xyzm(20, 10, 2, 8);
    CoordinateXYZM q1_xyzm(22, 10, 3, 9);
    CoordinateXYZM q2_xyzm(30, 10, 4, 11);

    CoordType p1(p1_xyzm);
    CoordType p2(p2_xyzm);
    CoordType q1(q1_xyzm);
    CoordType q2(q2_xyzm);

    for (auto _ : state) {
        LineIntersector li;
        li.computeIntersection(p1, p2, q1, q2);
    }
}
BENCHMARK_TEMPLATE(BM_PointIntersection, CoordinateXY);
BENCHMARK_TEMPLATE(BM_PointIntersection, Coordinate);
BENCHMARK_TEMPLATE(BM_PointIntersection, CoordinateXYZM);
BENCHMARK_TEMPLATE(BM_Collinear, CoordinateXY);
BENCHMARK_TEMPLATE(BM_Collinear, Coordinate);
BENCHMARK_TEMPLATE(BM_Collinear, CoordinateXYZM);

BENCHMARK_MAIN();

