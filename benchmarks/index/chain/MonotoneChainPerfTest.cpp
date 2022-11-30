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

#include <random>

#include <benchmark/benchmark.h>

#include <geos/geom/CoordinateSequence.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainOverlapAction.h>

using geos::geom::Coordinate;
using geos::geom::LineSegment;
using geos::geom::CoordinateSequence;
using geos::geom::Envelope;
using geos::index::chain::MonotoneChain;
using geos::index::chain::MonotoneChainOverlapAction;

static void BM_MonotoneChainOverlaps(benchmark::State& state) {
    CoordinateSequence cs1;
    CoordinateSequence cs2;

    std::default_random_engine e(12345);
    std::uniform_real_distribution<> dist(0, 1);

    Coordinate prev(0, 0);
    for (size_t i = 0; i < 100; i++) {
        Coordinate c(prev.x + dist(e), prev.y + dist(e));
        cs1.add(c);
        prev = c;
    }

    prev = Coordinate(0, 100);
    for (size_t i = 0; i < 100; i++) {
        Coordinate c(prev.x + dist(e), prev.y - dist(e));
        cs2.add(c);
        prev = c;
    }

    MonotoneChain mc1(cs1, 0, cs1.size(), nullptr);
    MonotoneChain mc2(cs2, 0, cs1.size(), nullptr);

    struct EmptyOverlapAction : public MonotoneChainOverlapAction {
        virtual void
        overlap(const LineSegment& seg1, const LineSegment& seg2) {
            (void) seg1;
            (void) seg2;
        }
    };
    EmptyOverlapAction mco;

    for (auto _ : state) {
        mc1.computeOverlaps(&mc2, &mco);
    }
}

BENCHMARK(BM_MonotoneChainOverlaps);

BENCHMARK_MAIN();

