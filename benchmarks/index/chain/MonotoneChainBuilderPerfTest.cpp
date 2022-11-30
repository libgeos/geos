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
#include <geos/index/chain/MonotoneChainBuilder.h>

using geos::geom::Coordinate;
using geos::geom::LineSegment;
using geos::geom::CoordinateSequence;
using geos::geom::Envelope;
using geos::index::chain::MonotoneChain;
using geos::index::chain::MonotoneChainBuilder;

static void BM_MonotoneChainBuilder(benchmark::State& state) {
    std::default_random_engine e(12345);

    // distribution of offsets from previous vertex
    std::uniform_real_distribution<> dist(0, 1);

    // determine chain lengths using a geometric distribution with a 30% probability of
    // a vertex ending a chain.
    std::geometric_distribution<> chain_length_dist(0.3);

    // distribution for determining quadrant of the next chain
    std::uniform_real_distribution<> sign_dist(-1, 1);

    std::size_t num_points = 1000;

    CoordinateSequence cs;
    Coordinate prev(0, 0);
    while(cs.size() <= num_points) {
        size_t chain_length = 1 + static_cast<size_t>(chain_length_dist(e));

        double xsign = sign_dist(e) > 0 ? 1 : -1;
        double ysign = sign_dist(e) > 0 ? 1 : -1;

        for (size_t k = 0; k < chain_length && cs.size() <= num_points; k++) {
            Coordinate c(prev.x + xsign * dist(e),
                         prev.y + ysign * dist(e));
            cs.add(c);
            prev = c;
        }
    }

    for (auto _ : state) {
        std::vector<MonotoneChain> chains;
        MonotoneChainBuilder::getChains(&cs, nullptr, chains);
    }
}

BENCHMARK(BM_MonotoneChainBuilder);

BENCHMARK_MAIN();
