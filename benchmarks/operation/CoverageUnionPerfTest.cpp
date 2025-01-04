/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 Daniel Baston
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <stdexcept>
#include <benchmark/benchmark.h>

#include <BenchmarkUtils.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/coverage/CoverageUnion.h>
#include <geos/operation/union/CoverageUnion.h>

struct SegmentSet {
    static void Union(const geos::geom::GeometryCollection& coll) {
        geos::operation::geounion::CoverageUnion::Union(&coll);
    }
};

struct BoundaryChain {
    static void Union(const geos::geom::GeometryCollection& coll) {
        auto result = geos::coverage::CoverageUnion::Union(&coll);
    }
};

template<typename Impl>
static void BM_CoverageUnion(benchmark::State& state) {
    const auto& gfact = *geos::geom::GeometryFactory::getDefaultInstance();

    auto nCells = state.range(0);

    auto nx = static_cast<int>(std::ceil(std::sqrt(nCells)));
    auto ny = static_cast<int>(std::ceil(std::sqrt(nCells)));

    nCells = nx*ny;

    geos::geom::Envelope env(0, nx, 0, ny);

    auto cells = geos::benchmark::createGeometriesOnGrid(env, static_cast<std::size_t>(nCells), [&gfact](const auto& base) {
        geos::geom::Envelope box(base.x, base.x + 1, base.y, base.y + 1);
        return gfact.toGeometry(&box);
    });

    auto coll = gfact.createGeometryCollection(std::move(cells));

    for (auto _ : state) {
        Impl::Union(*coll);
    }
}

BENCHMARK_TEMPLATE(BM_CoverageUnion, SegmentSet)->Range(1000, 1000000);
BENCHMARK_TEMPLATE(BM_CoverageUnion, BoundaryChain)->Range(1000, 1000000);

BENCHMARK_MAIN();

