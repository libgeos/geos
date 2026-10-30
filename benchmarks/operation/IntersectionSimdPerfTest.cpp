/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Benchmarks for the kernels used by intersection operations, each run
 * with the scalar code path (simd:0) and the AVX-512 code path (simd:1).
 *
 * Set GEOS_BENCH_WKT to a file with one WKT geometry per line to also run
 * the RealData benchmarks.
 *
 **********************************************************************/

#include <benchmark/benchmark.h>

#include <geos/algorithm/LineIntersector.h>
#include <geos/algorithm/RayCrossingCounter.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/index/chain/MonotoneChain.h>
#include <geos/index/chain/MonotoneChainBuilder.h>
#include <geos/index/strtree/TemplateSTRtree.h>
#include <geos/io/WKTReader.h>
#include <geos/noding/IntersectionAdder.h>
#include <geos/noding/MCIndexNoder.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/overlayng/RingClipper.h>

#if __has_include(<geos/util/Simd.h>)
#include <geos/util/Simd.h>
#define HAVE_SIMD_TOGGLE 1
#endif

#include <BenchmarkUtils.h>

#include <cstdlib>
#include <fstream>
#include <random>

using geos::geom::CoordinateSequence;
using geos::geom::CoordinateXY;
using geos::geom::Envelope;
using geos::geom::Geometry;
using geos::operation::overlayng::OverlayNG;

namespace {

// Selects the code path for the duration of a benchmark.
class SimdMode {
public:
    explicit SimdMode(benchmark::State& state, std::size_t arg)
    {
#ifdef HAVE_SIMD_TOGGLE
        prev = geos::util::simd::setAvx512Enabled(state.range(arg) != 0);
        if (state.range(arg) != 0 && !geos::util::simd::isAvx512Enabled()) {
            state.SkipWithMessage("AVX-512 not supported");
        }
#else
        (void) state;
        (void) arg;
#endif
    }

    ~SimdMode()
    {
#ifdef HAVE_SIMD_TOGGLE
        geos::util::simd::setAvx512Enabled(prev);
#endif
    }

private:
    bool prev = false;
};

const std::vector<int64_t> simdModes{0, 1};

std::unique_ptr<geos::geom::Polygon>
sineStar(double x, double y, double size, std::size_t npts)
{
    return geos::benchmark::createSineStar({x, y}, size, npts);
}

const CoordinateSequence*
shellCoords(const geos::geom::Polygon& p)
{
    return p.getExteriorRing()->getCoordinatesRO();
}

} // namespace

// ---------------------------------------------------------------------------
// Kernels
// ---------------------------------------------------------------------------

// Clips a ring to a box covering its middle, as done by OverlayNG
static void BM_RingClipper(benchmark::State& state) {
    SimdMode mode(state, 1);
    auto poly = sineStar(0, 0, 100, static_cast<std::size_t>(state.range(0)));
    Envelope clipEnv(-30, 40, -45, 25);
    geos::operation::overlayng::RingClipper clipper(&clipEnv);

    for (auto _ : state) {
        auto clipped = clipper.clip(shellCoords(*poly));
        benchmark::DoNotOptimize(clipped);
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_RingClipper)->ArgsProduct({{100, 1000, 10000, 100000}, simdModes});

// Point-in-ring tests of random points against a ring
static void BM_PointInRing(benchmark::State& state) {
    SimdMode mode(state, 1);
    std::default_random_engine eng(12345);
    auto poly = sineStar(0, 0, 100, static_cast<std::size_t>(state.range(0)));
    auto pts = geos::benchmark::createRandomCoords(*poly->getEnvelopeInternal(), 64, eng);
    const CoordinateSequence& ring = *shellCoords(*poly);

    for (auto _ : state) {
        for (const auto& p : pts->items<CoordinateXY>()) {
            benchmark::DoNotOptimize(geos::algorithm::RayCrossingCounter::locatePointInRing(p, ring));
        }
    }
    state.SetItemsProcessed(state.iterations() * 64 * state.range(0));
}
BENCHMARK(BM_PointInRing)->ArgsProduct({{16, 100, 1000, 10000}, simdModes});

// Monotone chain construction for a ring
static void BM_MonotoneChainBuilder(benchmark::State& state) {
    SimdMode mode(state, 1);
    auto poly = sineStar(0, 0, 100, static_cast<std::size_t>(state.range(0)));
    const CoordinateSequence* seq = shellCoords(*poly);

    std::vector<geos::index::chain::MonotoneChain> chains;
    for (auto _ : state) {
        chains.clear();
        geos::index::chain::MonotoneChainBuilder::getChains(seq, nullptr, chains);
        benchmark::DoNotOptimize(chains.data());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}
BENCHMARK(BM_MonotoneChainBuilder)->ArgsProduct({{100, 1000, 10000, 100000}, simdModes});

// Intersection of segment pairs with overlapping envelopes which
// do not intersect (arg 0) or do intersect (arg 1)
static void BM_LineIntersector(benchmark::State& state) {
    SimdMode mode(state, 1);
    std::default_random_engine eng(12345);
    std::uniform_real_distribution<> pos(0, 1);
    std::uniform_real_distribution<> off(-0.2, 0.2);

    const bool wantIntersecting = state.range(0) != 0;
    const std::size_t n = 4096;
    geos::algorithm::LineIntersector li;
    std::vector<CoordinateXY> pts;
    while (pts.size() < 4 * n) {
        CoordinateXY p1(pos(eng), pos(eng));
        CoordinateXY p2(p1.x + off(eng), p1.y + off(eng));
        CoordinateXY q1(p1.x + off(eng), p1.y + off(eng));
        CoordinateXY q2(q1.x + off(eng), q1.y + off(eng));
        if (Envelope::intersects(p1, p2, q1, q2)) {
            li.computeIntersection(p1, p2, q1, q2);
            if (li.hasIntersection() == wantIntersecting) {
                pts.insert(pts.end(), {p1, p2, q1, q2});
            }
        }
    }

    for (auto _ : state) {
        std::size_t count = 0;
        for (std::size_t i = 0; i < pts.size(); i += 4) {
            li.computeIntersection(pts[i], pts[i + 1], pts[i + 2], pts[i + 3]);
            count += li.hasIntersection();
        }
        benchmark::DoNotOptimize(count);
    }
    state.SetItemsProcessed(state.iterations() * static_cast<int64_t>(n));
}
BENCHMARK(BM_LineIntersector)->ArgsProduct({{0, 1}, simdModes});

// Full noding of two crossing rings, as done by OverlayNG
static void BM_MCIndexNoder(benchmark::State& state) {
    using namespace geos::noding;
    SimdMode mode(state, 1);
    auto npts = static_cast<std::size_t>(state.range(0));
    auto a = sineStar(0, 0, 100, npts);
    auto b = sineStar(20, 10, 100, npts);
    std::shared_ptr<const CoordinateSequence> ca(shellCoords(*a)->clone());
    std::shared_ptr<const CoordinateSequence> cb(shellCoords(*b)->clone());

    for (auto _ : state) {
        NodedSegmentString ssa(ca, false, false, nullptr);
        NodedSegmentString ssb(cb, false, false, nullptr);
        std::vector<SegmentString*> input{&ssa, &ssb};

        geos::algorithm::LineIntersector li;
        IntersectionAdder si(li);
        MCIndexNoder noder(&si);
        noder.computeNodes(input);
        benchmark::DoNotOptimize(si.numIntersections);
    }
}
BENCHMARK(BM_MCIndexNoder)->ArgsProduct({{1000, 10000, 100000}, simdModes});

// Envelope queries against an STRtree
static void BM_STRtreeQuery(benchmark::State& state) {
    SimdMode mode(state, 1);
    std::default_random_engine eng(12345);
    std::uniform_real_distribution<> pos(0, 1000);
    std::uniform_real_distribution<> size(0, 5);

    auto n = static_cast<std::size_t>(state.range(0));
    geos::index::strtree::TemplateSTRtree<std::size_t> tree;
    for (std::size_t i = 0; i < n; i++) {
        double x = pos(eng), y = pos(eng);
        tree.insert(Envelope(x, x + size(eng), y, y + size(eng)), i);
    }
    std::vector<Envelope> queries;
    for (std::size_t i = 0; i < 1000; i++) {
        double x = pos(eng), y = pos(eng);
        queries.emplace_back(x, x + 10, y, y + 10);
    }
    tree.build();

    for (auto _ : state) {
        std::size_t hits = 0;
        for (const auto& q : queries) {
            tree.query(q, [&hits](const std::size_t&) { hits++; });
        }
        benchmark::DoNotOptimize(hits);
    }
}
BENCHMARK(BM_STRtreeQuery)->ArgsProduct({{10000, 1000000}, simdModes});

// Self-join of an STRtree, as done by MCIndexNoder
static void BM_STRtreeQueryPairs(benchmark::State& state) {
    SimdMode mode(state, 1);
    std::default_random_engine eng(12345);
    std::uniform_real_distribution<> pos(0, 1000);
    std::uniform_real_distribution<> size(0, 5);

    auto n = static_cast<std::size_t>(state.range(0));
    geos::index::strtree::TemplateSTRtree<std::size_t> tree;
    for (std::size_t i = 0; i < n; i++) {
        double x = pos(eng), y = pos(eng);
        tree.insert(Envelope(x, x + size(eng), y, y + size(eng)), i);
    }
    tree.build();

    for (auto _ : state) {
        std::size_t hits = 0;
        tree.queryPairs([&hits](const std::size_t&, const std::size_t&) { hits++; });
        benchmark::DoNotOptimize(hits);
    }
}
BENCHMARK(BM_STRtreeQueryPairs)->ArgsProduct({{10000, 100000}, simdModes});

// ---------------------------------------------------------------------------
// Operations
// ---------------------------------------------------------------------------

static void BM_OverlayIntersection(benchmark::State& state) {
    SimdMode mode(state, 1);
    auto npts = static_cast<std::size_t>(state.range(0));
    auto a = sineStar(0, 0, 100, npts);
    auto b = sineStar(60, 30, 100, npts);

    for (auto _ : state) {
        auto result = OverlayNG::overlay(a.get(), b.get(), OverlayNG::INTERSECTION);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_OverlayIntersection)->ArgsProduct({{100, 1000, 10000, 100000}, simdModes});

static void BM_Intersects(benchmark::State& state) {
    SimdMode mode(state, 1);
    auto npts = static_cast<std::size_t>(state.range(0));
    auto a = sineStar(0, 0, 100, npts);
    auto b = sineStar(60, 30, 100, npts);

    for (auto _ : state) {
        benchmark::DoNotOptimize(a->intersects(b.get()));
    }
}
BENCHMARK(BM_Intersects)->ArgsProduct({{100, 1000, 10000, 100000}, simdModes});

// Prepared polygon against random small polygons, as in a spatial join
static void BM_PreparedIntersects(benchmark::State& state) {
    SimdMode mode(state, 1);
    std::default_random_engine eng(12345);
    auto npts = static_cast<std::size_t>(state.range(0));
    auto a = sineStar(0, 0, 100, npts);
    auto prepA = geos::geom::prep::PreparedGeometryFactory::prepare(a.get());

    std::uniform_real_distribution<> pos(-100, 100);
    std::vector<std::unique_ptr<geos::geom::Polygon>> tests;
    for (int i = 0; i < 256; i++) {
        tests.push_back(sineStar(pos(eng), pos(eng), 5, 32));
    }

    for (auto _ : state) {
        std::size_t hits = 0;
        for (const auto& t : tests) {
            hits += prepA->intersects(t.get());
        }
        benchmark::DoNotOptimize(hits);
    }
}
BENCHMARK(BM_PreparedIntersects)->ArgsProduct({{1000, 100000}, simdModes});

// ---------------------------------------------------------------------------
// Real data (GEOS_BENCH_WKT)
// ---------------------------------------------------------------------------

namespace {

struct RealData {
    std::vector<std::unique_ptr<Geometry>> geoms;
    // pairs with intersecting envelopes, i <= j
    std::vector<std::pair<std::size_t, std::size_t>> pairs;
};

const RealData*
realData()
{
    static std::unique_ptr<RealData> data = []() -> std::unique_ptr<RealData> {
        const char* path = std::getenv("GEOS_BENCH_WKT");
        if (path == nullptr) {
            return nullptr;
        }
        auto d = std::make_unique<RealData>();
        geos::io::WKTReader reader;
        std::ifstream f(path);
        std::string line;
        while (std::getline(f, line)) {
            if (!line.empty()) {
                d->geoms.push_back(reader.read(line));
            }
        }
        geos::index::strtree::TemplateSTRtree<std::size_t> tree;
        for (std::size_t i = 0; i < d->geoms.size(); i++) {
            tree.insert(*d->geoms[i]->getEnvelopeInternal(), i);
        }
        for (std::size_t i = 0; i < d->geoms.size(); i++) {
            tree.query(*d->geoms[i]->getEnvelopeInternal(), [&d, i](std::size_t j) {
                if (i <= j) {
                    d->pairs.emplace_back(i, j);
                }
            });
        }
        return d;
    }();
    return data.get();
}

} // namespace

static void BM_RealData_Intersection(benchmark::State& state) {
    SimdMode mode(state, 0);
    const RealData* d = realData();
    if (d == nullptr) {
        state.SkipWithMessage("GEOS_BENCH_WKT not set");
        return;
    }
    for (auto _ : state) {
        for (const auto& pr : d->pairs) {
            const Geometry* a = d->geoms[pr.first].get();
            const Geometry* b = d->geoms[pr.second].get();
            if (a->intersects(b)) {
                benchmark::DoNotOptimize(a->intersection(b));
            }
        }
    }
}
BENCHMARK(BM_RealData_Intersection)->ArgsProduct({simdModes})->Unit(benchmark::kMillisecond)->MinTime(0.5);

static void BM_RealData_Intersects(benchmark::State& state) {
    SimdMode mode(state, 0);
    const RealData* d = realData();
    if (d == nullptr) {
        state.SkipWithMessage("GEOS_BENCH_WKT not set");
        return;
    }
    for (auto _ : state) {
        std::size_t hits = 0;
        for (const auto& pr : d->pairs) {
            hits += d->geoms[pr.first]->intersects(d->geoms[pr.second].get());
        }
        benchmark::DoNotOptimize(hits);
    }
}
BENCHMARK(BM_RealData_Intersects)->ArgsProduct({simdModes})->Unit(benchmark::kMillisecond);

static void BM_RealData_PreparedIntersects(benchmark::State& state) {
    SimdMode mode(state, 0);
    const RealData* d = realData();
    if (d == nullptr) {
        state.SkipWithMessage("GEOS_BENCH_WKT not set");
        return;
    }
    for (auto _ : state) {
        std::size_t hits = 0;
        std::size_t current = SIZE_MAX;
        std::unique_ptr<geos::geom::prep::PreparedGeometry> prep;
        for (const auto& pr : d->pairs) {
            if (pr.first != current) {
                current = pr.first;
                prep = geos::geom::prep::PreparedGeometryFactory::prepare(d->geoms[current].get());
            }
            hits += prep->intersects(d->geoms[pr.second].get());
        }
        benchmark::DoNotOptimize(hits);
    }
}
BENCHMARK(BM_RealData_PreparedIntersects)->ArgsProduct({simdModes})->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
