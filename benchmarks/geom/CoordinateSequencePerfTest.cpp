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
#include <geos/geom/CoordinateFilter.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/geom/SegmentInspector.h>

using geos::geom::Coordinate;
using geos::geom::CoordinateArraySequence;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateFilter;
using geos::geom::SegmentInspector;

constexpr size_t N = 10000;

std::unique_ptr<CoordinateSequence> getSeq(std::size_t n) {
    // prevent devirtualization by constructing this outside the benchmark function itself
    geos::geom::CoordinateArraySequenceFactory casf;
    return casf.create(n);
}

static void BM_CoordinateSequenceForLoopCacheSize(benchmark::State& state) {
    auto seq = getSeq(N);

    for (auto _ : state) {
        const Coordinate* q;
        auto sz = seq->size();
        for (size_t i = 0; i < sz; i++) {
            benchmark::DoNotOptimize(q = &seq->getAt(i));
        }
    }
}

static void BM_CoordinateSequenceSegmentForLoopCacheSize(benchmark::State& state) {
    auto seq = getSeq(N);

    for (auto _ : state) {
        const Coordinate* q;
        auto sz = seq->size();
        for (size_t i = 1; i < sz; i++) {
            benchmark::DoNotOptimize(q = &seq->getAt(i - 1));
            benchmark::DoNotOptimize(q = &seq->getAt(i));
        }
    }
}

static void BM_CoordinateSequenceForLoop(benchmark::State& state) {
    auto seq = getSeq(N);

    for (auto _ : state) {
        const Coordinate* q;
        for (size_t i = 0; i < seq->size(); i++) {
            benchmark::DoNotOptimize(q = &seq->getAt(i));
        }
    }
}

static void BM_CoordinateSequenceFilter(benchmark::State& state) {
    auto seq = getSeq(N);

    class Filter : public CoordinateFilter {
        void filter_ro(const Coordinate* c) override {
            benchmark::DoNotOptimize(c);
        }
    };
    Filter f;

    for (auto _ : state) {
        seq->apply_ro(&f);
    }
}

static void BM_CoordinateSequenceSegmentInspector(benchmark::State& state) {
    auto seq = getSeq(N);

    class Inspector : public SegmentInspector {
        void inspect(const Coordinate& c1, const Coordinate& c2) override {
            benchmark::DoNotOptimize(c1);
            benchmark::DoNotOptimize(c2);
        }
    };
    Inspector f;

    for (auto _ : state) {
        seq->apply_ro(&f);
    }
}

static void BM_CoordinateSequenceForEach(benchmark::State& state) {
    auto seq = getSeq(N);

    for (auto _ : state) {
        const Coordinate* q;
        seq->forEach([&q](const Coordinate* c) {
            benchmark::DoNotOptimize(q = c);
        });
    }
}

static void BM_CoordinateSequenceSegmentForEach(benchmark::State& state) {
    auto seq = getSeq(N);

    for (auto _ : state) {
        const Coordinate* q;
        seq->forEachSegment([&q](const Coordinate& c1, const Coordinate& c2) {
            (void) c2;
            benchmark::DoNotOptimize(q = &c1);
        });
    }
}

BENCHMARK(BM_CoordinateSequenceForLoop);
BENCHMARK(BM_CoordinateSequenceForLoopCacheSize);
BENCHMARK(BM_CoordinateSequenceFilter);
BENCHMARK(BM_CoordinateSequenceForEach);

BENCHMARK(BM_CoordinateSequenceSegmentForLoopCacheSize);
BENCHMARK(BM_CoordinateSequenceSegmentInspector);
BENCHMARK(BM_CoordinateSequenceSegmentForEach);

BENCHMARK_MAIN();

