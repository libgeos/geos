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

#include <geos/index/strtree/STRtree.h>
#include <geos/index/strtree/SimpleSTRtree.h>
#include <geos/index/strtree/TemplateSTRtree.h>
#include <geos/index/quadtree/Quadtree.h>
#include <geos/index/intervalrtree/SortedPackedIntervalRTree.h>

using geos::geom::CoordinateXY;
using geos::geom::Envelope;
using geos::index::intervalrtree::SortedPackedIntervalRTree;
using geos::index::quadtree::Quadtree;
using geos::index::strtree::STRtree;
using geos::index::strtree::SimpleSTRtree;
using geos::index::strtree::TemplateSTRtree;
using geos::index::strtree::Interval;
using geos::index::strtree::ItemDistance;
using geos::index::strtree::ItemBoundable;

using TemplateIntervalTree = TemplateSTRtree<const Interval*, geos::index::strtree::IntervalTraits>;

//////////////////////////
// Test Data Generation //
//////////////////////////

static std::vector<Envelope> generate_envelopes(std::default_random_engine & e,
                                                const Envelope& extent,
                                                std::size_t n) {
    std::uniform_real_distribution<> centroid_x(extent.getMinX(), extent.getMaxX());
    std::uniform_real_distribution<> centroid_y(extent.getMinY(), extent.getMaxY());

    // distributions of width and aspect ratio fit from HydroBASINS level 7 (Africa)
    std::weibull_distribution<> size_x(1.606, 0.00989);
    std::lognormal_distribution<> y_rat(-0.027, 0.4884);

    std::vector<Envelope> envelopes(n);
    for (std::size_t i = 0; i < n; i++) {
        double cx = centroid_x(e);
        double cy = centroid_y(e);

        double width = size_x(e) * extent.getWidth();
        double height = width * y_rat(e);

        envelopes[i] = Envelope(cx - width / 2, cx + width / 2,
                               cy - height / 2, cy + height / 2);

    }

    return envelopes;
}

std::vector<CoordinateXY> generate_uniform_points(std::default_random_engine& eng,
                                                const Envelope& box,
                                                std::size_t n) {
    std::vector<CoordinateXY> pts(n);

    std::uniform_real_distribution<> qx(box.getMinX(), box.getMaxX());
    std::uniform_real_distribution<> qy(box.getMinY(), box.getMaxY());

    for (std::size_t i = 0; i < n; i++) {
        pts[i] = { qx(eng), qy(eng) };
    }

    return pts;
}

//////////////
// Visitors //
//////////////

class CountingVisitor : public geos::index::ItemVisitor {
    size_t hits = 0;
    void visitItem(void* item) override {
        hits += (item != nullptr);
    }
};

template<typename T>
struct Counter {
    size_t hits = 0;

    void operator()(const T& item) {
        (void) item;
        hits++;
    }

    void operator()(const T& item1, const T& item2) {
        (void) item1;
        (void) item2;
        hits++;
    }
};

struct EnvelopeDistance : public ItemDistance {
    double distance(const ItemBoundable* a, const ItemBoundable* b) override {
        Envelope* ea = static_cast<Envelope*>(a->getItem());
        Envelope* eb = static_cast<Envelope*>(b->getItem());

        return ea->distance(*eb);
    }

    double operator()(const Envelope* a, const Envelope* b) {
        return a->distance(*b);
    }
};

/////////////////
// 1D adapters //
/////////////////

static inline void insert(TemplateIntervalTree & tree, const Interval& i) {
    tree.insert(i, &i);
}

static inline void insert(SortedPackedIntervalRTree & tree, const Interval& i) {
    tree.insert(i.getMin(), i.getMax(), (void*) &i);
}

static inline void query(TemplateIntervalTree & tree, const Interval& i) {
    size_t hits = 0;
    tree.query(i, [&hits](const Interval* item) {
        hits += (item != nullptr);
    });
}

static inline void query(SortedPackedIntervalRTree & tree, const Interval& i) {
    CountingVisitor cv;
    tree.query(i.getMin(), i.getMax(), &cv);
}

/////////////////
// 2D adapters //
/////////////////

template<class Tree>
static inline const void* nearestNeighbour(Tree & tree, const Envelope & e) {
    EnvelopeDistance dist;
    return tree.nearestNeighbour(e, &e, dist);
}

static inline const void* nearestNeighbour(SimpleSTRtree & tree, const Envelope & e) {
    EnvelopeDistance dist;
    return tree.nearestNeighbour(&e, &e, &dist);
}

static inline const void* nearestNeighbour(STRtree & tree, const Envelope & e) {
    EnvelopeDistance dist;
    return tree.nearestNeighbour(&e, &e, &dist);
}

///////////////////
// 1D benchmarks //
///////////////////

template<class Tree>
static void BM_STRtree1DConstruct(benchmark::State& state) {
    std::default_random_engine eng(12345);
    Envelope extent(0, 1, 0, 1);
    auto envelopes = generate_envelopes(eng, extent, 10000);
    std::vector<Interval> intervals;
    intervals.reserve(envelopes.size());

    for (const auto& e : envelopes) {
        intervals.emplace_back(e.getMinY(), e.getMaxY());
    }

    Interval outside_extent(extent.getMaxY() + 100, extent.getMaxY() + 101);

    for (auto _ : state) {
        Tree tree(10);

        for (const auto& interval : intervals) {
            insert(tree, interval);
        }
        query(tree, outside_extent);
    }
}

template<class Tree>
static void BM_STRtree1DQuery(benchmark::State& state) {
    std::default_random_engine eng(12345);
    Envelope extent(0, 1, 0, 1);
    auto envelopes = generate_envelopes(eng, extent, 10000);
    std::vector<Interval> intervals;
    intervals.reserve(envelopes.size());

    for (const auto& e : envelopes) {
        intervals.emplace_back(e.getMinY(), e.getMaxY());
    }

    Tree tree(10);

    for (const auto& interval : intervals) {
        insert(tree, interval);
    }
    Interval outside_extent(extent.getMaxY() + 100, extent.getMaxY() + 101);
    query(tree, outside_extent); // force construction

    for (auto _ : state) {
        for (auto& i : intervals) {
            query(tree, i);
        }
    }
}

///////////////////
// 2D benchmarks //
///////////////////

template<class Tree>
static void BM_STRtree2DConstruct(benchmark::State& state) {
    std::default_random_engine eng(12345);
    Envelope extent(0, 1, 0, 1);
    auto envelopes = generate_envelopes(eng, extent, 10000);
    Envelope empty_env;

    std::vector<void*> hits;

    for (auto _ : state) {
        Tree tree;
        for (auto& e : envelopes) {
            tree.insert(&e, &e);
        }
        tree.query(&empty_env, hits); // query with empty envelope to force construction
    }
}

template<class Tree>
static void BM_STRtree2DQuery(benchmark::State& state) {
    std::default_random_engine eng(12345);
    Envelope extent(0, 1, 0, 1);
    auto envelopes = generate_envelopes(eng, extent, 10000);
    Envelope empty_env;

    std::vector<void*> hits;

    Tree tree;
    for (auto& e : envelopes) {
        tree.insert(&e, &e);
    }
    tree.query(&empty_env, hits); // query with empty envelope to force construction

    for (auto _ : state) {
        hits.clear();
        for (auto& e : envelopes) {
            tree.query(&e, hits);
        }
    }
}

static void BM_STRtree2DQueryPairs(benchmark::State& state) {
    std::default_random_engine eng(12345);
    Envelope extent(0, 1, 0, 1);
    auto envelopes = generate_envelopes(eng, extent, 10000);
    Envelope empty_env;

    TemplateSTRtree<const Envelope*> tree;
    for (auto& e : envelopes) {
        tree.insert(&e, &e);
    }
    Counter<const Envelope*> q;
    tree.query(empty_env, q); // query with empty envelope to force construction

    for (auto _ : state) {
        Counter<const Envelope*> c;
        tree.queryPairs(c);
    }
}

static void BM_STRtree2DQueryPairsNaive(benchmark::State& state) {
    std::default_random_engine eng(12345);
    Envelope extent(0, 1, 0, 1);
    auto envelopes = generate_envelopes(eng, extent, 10000);
    Envelope empty_env;

    TemplateSTRtree<const Envelope*> tree;
    for (auto& e : envelopes) {
        tree.insert(&e, &e);
    }
    Counter<const Envelope*> q;
    tree.query(empty_env, q); // query with empty envelope to force construction

    for (auto _ : state) {
        Counter<const Envelope*> c;
        for (const auto& env : envelopes) {
            tree.query(env, c);
        }
    }
}

template<class Tree>
static void BM_STRtree2DNearest(benchmark::State& state) {
    std::default_random_engine eng(12345);
    Envelope extent(0, 1, 0, 1);
    auto envelopes = generate_envelopes(eng, extent, 10000);
    Envelope empty_env;

    Envelope queryEnv = extent;
    queryEnv.expandBy(0.25 * queryEnv.getWidth(), 0.25 * queryEnv.getHeight());
    auto queryPoints = generate_uniform_points(eng, queryEnv, 10000);

    std::vector<Envelope> queryEnvelopes(queryPoints.size());
    for (std::size_t i = 0; i < queryEnvelopes.size(); i++) {
        queryEnvelopes[i] = Envelope(queryPoints[i]);
    }

    std::vector<void*> hits;

    Tree tree;
    for (auto& e : envelopes) {
        tree.insert(&e, &e);
    }
    tree.query(&empty_env, hits); // query with empty envelope to force construction

    for (auto _ : state) {
        for (auto& e : queryEnvelopes) {
            nearestNeighbour(tree, e);
        }
    }
}

BENCHMARK_TEMPLATE(BM_STRtree1DConstruct, SortedPackedIntervalRTree);
BENCHMARK_TEMPLATE(BM_STRtree1DConstruct, TemplateIntervalTree);
BENCHMARK_TEMPLATE(BM_STRtree1DQuery, SortedPackedIntervalRTree);
BENCHMARK_TEMPLATE(BM_STRtree1DQuery, TemplateIntervalTree);

BENCHMARK_TEMPLATE(BM_STRtree2DConstruct, Quadtree);
BENCHMARK_TEMPLATE(BM_STRtree2DConstruct, STRtree);
BENCHMARK_TEMPLATE(BM_STRtree2DConstruct, SimpleSTRtree);
BENCHMARK_TEMPLATE(BM_STRtree2DConstruct, TemplateSTRtree<const Envelope*>);

BENCHMARK_TEMPLATE(BM_STRtree2DNearest, STRtree);
BENCHMARK_TEMPLATE(BM_STRtree2DNearest, SimpleSTRtree);
BENCHMARK_TEMPLATE(BM_STRtree2DNearest, TemplateSTRtree<const Envelope*>);

BENCHMARK_TEMPLATE(BM_STRtree2DQuery, Quadtree);
BENCHMARK_TEMPLATE(BM_STRtree2DQuery, STRtree);
BENCHMARK_TEMPLATE(BM_STRtree2DQuery, SimpleSTRtree);
BENCHMARK_TEMPLATE(BM_STRtree2DQuery, TemplateSTRtree<const Envelope*>);

BENCHMARK(BM_STRtree2DQueryPairs);
BENCHMARK(BM_STRtree2DQueryPairsNaive);

BENCHMARK_MAIN();

