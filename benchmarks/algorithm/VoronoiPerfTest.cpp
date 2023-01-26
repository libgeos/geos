/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Daniel Baston <dbaston@gmail.com>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <geos/triangulate/DelaunayTriangulationBuilder.h>
#include <geos/triangulate/VoronoiDiagramBuilder.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/profiler.h>

#include <algorithm>
#include <random>
#include <vector>
#include <memory>

#include <benchmark/benchmark.h>

#include <BenchmarkUtils.h>

using geos::geom::Coordinate;
using geos::geom::Envelope;

static void BM_DelaunayFromSeq(benchmark::State& state) {
    Envelope e(0, 100, 0, 100);
    auto gfact = geos::geom::GeometryFactory::getDefaultInstance();
    std::default_random_engine eng(12345);

    for (auto _ : state) {
        state.PauseTiming();
        auto nPts = static_cast<std::size_t>(state.range(0));
        auto sites = geos::benchmark::createRandomCoords(e, nPts, eng);
        state.ResumeTiming();

        geos::triangulate::DelaunayTriangulationBuilder dtb;
        dtb.setSites(*sites);
        auto result = dtb.getTriangles(*gfact);
    }
}

static void BM_DelaunayFromGeom(benchmark::State& state) {
    Envelope e(0, 100, 0, 100);
    auto gfact = geos::geom::GeometryFactory::getDefaultInstance();
    std::default_random_engine eng(12345);

    for (auto _ : state) {
        state.PauseTiming();
        auto nPts = static_cast<std::size_t>(state.range(0));
        auto sites = gfact->createLineString(geos::benchmark::createRandomCoords(e, nPts, eng));
        state.ResumeTiming();

        geos::triangulate::DelaunayTriangulationBuilder dtb;
        dtb.setSites(*sites);
        auto result = dtb.getTriangles(*gfact);
    }
}

static void BM_VoronoiFromSeq(benchmark::State& state) {
    Envelope e(0, 100, 0, 100);
    auto gfact = geos::geom::GeometryFactory::getDefaultInstance();
    std::default_random_engine eng(12345);

    for (auto _ : state) {
        state.PauseTiming();
        auto nPts = static_cast<std::size_t>(state.range(0));
        auto sites = geos::benchmark::createRandomCoords(e, nPts, eng);
        state.ResumeTiming();

        geos::triangulate::VoronoiDiagramBuilder vdb;
        vdb.setSites(*sites);
        auto result = vdb.getDiagram(*gfact);
    }
}

static void BM_VoronoiFromGeom(benchmark::State& state) {
    Envelope e(0, 100, 0, 100);
    auto gfact = geos::geom::GeometryFactory::getDefaultInstance();
    std::default_random_engine eng(12345);

    for (auto _ : state) {
        state.PauseTiming();
        auto nPts = static_cast<std::size_t>(state.range(0));
        auto sites = gfact->createLineString(geos::benchmark::createRandomCoords(e, nPts, eng));
        state.ResumeTiming();

        geos::triangulate::VoronoiDiagramBuilder vdb;
        vdb.setSites(*sites);
        auto result = vdb.getDiagram(*gfact);
    }
}

static void BM_OrderedVoronoiFromGeom(benchmark::State& state) {
    Envelope e(0, 100, 0, 100);
    auto gfact = geos::geom::GeometryFactory::getDefaultInstance();
    std::default_random_engine eng(12345);

    for (auto _ : state) {
        state.PauseTiming();
        auto nPts = static_cast<std::size_t>(state.range(0));
        auto sites = gfact->createLineString(geos::benchmark::createRandomCoords(e, nPts, eng));
        state.ResumeTiming();

        geos::triangulate::VoronoiDiagramBuilder vdb;
        vdb.setOrdered(true);
        vdb.setSites(*sites);
        auto result = vdb.getDiagram(*gfact);
    }
}

BENCHMARK(BM_DelaunayFromSeq)->Range(10, 1e6);
BENCHMARK(BM_DelaunayFromGeom)->Range(10, 1e6);
BENCHMARK(BM_VoronoiFromSeq)->Range(10, 1e6);
BENCHMARK(BM_VoronoiFromGeom)->Range(10, 1e6);
BENCHMARK(BM_OrderedVoronoiFromGeom)->Range(10, 1e6);

BENCHMARK_MAIN();
