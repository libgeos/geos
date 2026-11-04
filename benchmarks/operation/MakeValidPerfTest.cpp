/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2026 GEOS contributors
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Benchmark for MakeValid on MultiPolygon inputs (issue #1504).
 *
 * Reproduces the scenario where a MultiPolygon containing a single
 * invalid part costs up to ~55x the equivalent GeometryCollection,
 * because all parts are dragged through boundary noding, unique-point
 * extraction and BuildArea/symdiff.
 *
 * Cases (run one per process for clean peak-RSS readings):
 *   grid <n>      n*n valid unit squares spaced 2 apart + 1 far bowtie
 *                 (the issue #1504 scenario; envelope-disjoint components)
 *   chain <n>     n corner-touching squares (one envelope-connected
 *                 component) + 1 bowtie far away (a second component)
 *   overlap <n>   n overlapping squares (single connected component,
 *                 adversarial: cannot be split)
 *   bigbowtie <n> ~n small valid polygons + one huge invalid bowtie with
 *                 ~n vertices (mimics issue #1504 real data shape)
 *   sentinel <n>  n valid unit squares at Y=0 spaced 2 apart + 1 invalid
 *                 bowtie spanning the whole row at Y=100 (long-X,
 *                 Y-disjoint). Clustering must expire by maxX; a minX
 *                 prefix window is N(N+1)/2 envelope tests. To confirm
 *                 linear scaling run n=16000 / 32000 / 64000.
 *
 * An optional 4th argument "coll" runs the same parts as a
 * GeometryCollection, which MakeValid processes member-wise; it provides
 * a reference for the best achievable per-part cost.
 *
 * Timing uses geos::util::Profile like the other GEOS benchmarks. Peak RSS
 * uses getrusage where available (POSIX); ru_maxrss is bytes on macOS and
 * KiB on Linux/BSD, which is normalized below. On platforms without
 * getrusage (e.g. Windows) peak RSS is reported as unavailable.
 *
 * Usage: perf_makevalid <grid|chain|overlap|bigbowtie|sentinel> [n] [reps] [coll]
 **********************************************************************/

#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/operation/valid/MakeValid.h>
#include <geos/profiler.h>

#include <cmath>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#if defined(_WIN32)
// getrusage is not available on Windows; report peak RSS as unavailable.
#else
#include <sys/resource.h>
#endif

using namespace geos::geom;
using namespace geos::operation::valid;

namespace {

std::unique_ptr<Geometry> coll_geom;

// Peak resident set size in bytes; 0 when the platform offers no way to
// measure it. ru_maxrss is bytes on macOS and KiB on Linux/BSD.
long
peakRSS()
{
#if defined(_WIN32)
    return 0;
#else
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
#if defined(__APPLE__)
    return static_cast<long>(ru.ru_maxrss);
#else
    return static_cast<long>(ru.ru_maxrss) * 1024;
#endif
#endif
}

std::unique_ptr<Polygon>
square(const GeometryFactory* gf, double x, double y, double side = 1.0)
{
    auto cs = std::make_unique<CoordinateSequence>();
    cs->add(x, y);
    cs->add(x + side, y);
    cs->add(x + side, y + side);
    cs->add(x, y + side);
    cs->add(x, y);
    auto ring = gf->createLinearRing(std::move(cs));
    return gf->createPolygon(std::move(ring));
}

// Self-intersecting "bowtie" polygon (invalid), optionally with many
// vertices (zig-zagging between two diagonals so the ring crosses itself).
std::unique_ptr<Polygon>
bowtie(const GeometryFactory* gf, double x, double y, std::size_t nverts)
{
    auto cs = std::make_unique<CoordinateSequence>();
    cs->add(x, y);
    cs->add(x + 1, y + 1);
    if (nverts > 5) {
        // Insert extra vertices along both crossing diagonals,
        // zig-zagging between them to inflate vertex count.
        std::size_t nseg = (nverts - 5) / 4;
        for (std::size_t i = 1; i <= nseg; i++) {
            double t = static_cast<double>(i) / static_cast<double>(nseg + 1);
            cs->add(x + t * 2.0, y + 1.0);      // intermediate wiggle
            cs->add(x + t * 2.0, y);            //   on segment 1
            cs->add(x + t * 2.0, y + 1.0 - t);  // and on the diagonal
        }
    }
    cs->add(x + 1, y);
    cs->add(x, y + 1);
    cs->add(x, y);
    auto ring = gf->createLinearRing(std::move(cs));
    return gf->createPolygon(std::move(ring));
}

struct Fixture {
    std::unique_ptr<Geometry> multipolygon;
    std::string name;
};

// n*n disjoint unit squares spaced 2 apart, plus one bowtie far away.
Fixture
makeGrid(std::size_t n)
{
    auto gf = GeometryFactory::getDefaultInstance();
    std::vector<std::unique_ptr<Geometry>> polys;
    polys.reserve(n * n + 1);
    for (std::size_t i = 0; i < n; i++) {
        for (std::size_t j = 0; j < n; j++) {
            polys.push_back(square(gf, 2.0 * static_cast<double>(i),
                                   2.0 * static_cast<double>(j)));
        }
    }
    polys.push_back(bowtie(gf, -10.0, -10.0, 5));

    auto mp = gf->createMultiPolygon(std::move(polys));
    return {std::move(mp), "grid"};
}

// n corner-touching squares (diagonal chain: all one envelope-connected
// component), plus one bowtie far away (second component).
Fixture
makeChain(std::size_t n)
{
    auto gf = GeometryFactory::getDefaultInstance();
    std::vector<std::unique_ptr<Geometry>> polys;
    polys.reserve(n + 1);
    for (std::size_t i = 0; i < n; i++) {
        double d = static_cast<double>(i);
        polys.push_back(square(gf, d, d));
    }
    polys.push_back(bowtie(gf, -10.0, -10.0, 5));
    auto mp = gf->createMultiPolygon(std::move(polys));
    return {std::move(mp), "chain"};
}

// n overlapping squares (every square intersects the next: a single
// envelope-connected component that cannot be split).
Fixture
makeOverlap(std::size_t n)
{
    auto gf = GeometryFactory::getDefaultInstance();
    std::vector<std::unique_ptr<Geometry>> polys;
    polys.reserve(n);
    for (std::size_t i = 0; i < n; i++) {
        double d = 0.5 * static_cast<double>(i);
        polys.push_back(square(gf, d, d));
    }
    auto mp = gf->createMultiPolygon(std::move(polys));
    return {std::move(mp), "overlap"};
}

// n small valid polygons + one huge invalid bowtie with ~n vertices
// (shape of the real data in issue #1504).
Fixture
makeBigBowtie(std::size_t n)
{
    auto gf = GeometryFactory::getDefaultInstance();
    std::vector<std::unique_ptr<Geometry>> polys;
    polys.reserve(n + 1);
    std::size_t side = static_cast<std::size_t>(
        std::sqrt(static_cast<double>(n)));
    for (std::size_t i = 0; i < side * side; i++) {
        polys.push_back(square(gf, 2.0 * static_cast<double>(i % side),
                               2.0 * static_cast<double>(i / side)));
    }
    polys.push_back(bowtie(gf, -10.0, -10.0, n));
    auto mp = gf->createMultiPolygon(std::move(polys));
    return {std::move(mp), "bigbowtie"};
}

// n disjoint unit squares at Y=0 spaced 2 apart, plus one invalid bowtie
// spanning the whole row at Y=100 (long X, Y-disjoint). Expiry must be
// by maxX; a minX-prefix active window compares every pair.
Fixture
makeSentinel(std::size_t n)
{
    auto gf = GeometryFactory::getDefaultInstance();
    std::vector<std::unique_ptr<Geometry>> polys;
    polys.reserve(n + 1);
    for (std::size_t i = 0; i < n; i++) {
        polys.push_back(square(gf, 2.0 * static_cast<double>(i), 0.0));
    }
    double span = n == 0 ? 1.0 : 2.0 * static_cast<double>(n) - 1.0;
    auto cs = std::make_unique<CoordinateSequence>();
    cs->add(0.0, 100.0);
    cs->add(span, 101.0);
    cs->add(span, 100.0);
    cs->add(0.0, 101.0);
    cs->add(0.0, 100.0);
    auto ring = gf->createLinearRing(std::move(cs));
    polys.push_back(gf->createPolygon(std::move(ring)));
    auto mp = gf->createMultiPolygon(std::move(polys));
    return {std::move(mp), "sentinel"};
}

void
runCase(const Fixture& fix, bool useCollection, std::size_t reps)
{
    const Geometry* g = fix.multipolygon.get();
    std::string kind = "MULTIPOLYGON";
    if (useCollection) {
        // Same parts as a GeometryCollection: per-member processing.
        auto gf = fix.multipolygon->getFactory();
        std::vector<std::unique_ptr<Geometry>> parts;
        parts.reserve(fix.multipolygon->getNumGeometries());
        for (std::size_t i = 0; i < fix.multipolygon->getNumGeometries(); i++) {
            parts.push_back(fix.multipolygon->getGeometryN(i)->clone());
        }
        coll_geom = gf->createGeometryCollection(std::move(parts));
        g = coll_geom.get();
        kind = "GEOMCOLLECTION";
    }

    MakeValid mv;
    geos::util::Profile profile("makevalid");
    std::unique_ptr<Geometry> result;
    for (std::size_t i = 0; i < reps; i++) {
        profile.start();
        result = mv.build(g);
        profile.stop();
        if (result->getGeometryTypeId() == GEOS_GEOMETRYCOLLECTION
            && result->getNumGeometries() == 0) {
            std::cerr << "unexpected empty result\n";
            std::exit(1);
        }
    }

    std::cout << fix.name
              << " n=" << fix.multipolygon->getNumGeometries()
              << " verts=" << g->getNumPoints()
              << " " << kind
              << " result_type=" << result->getGeometryType()
              << " valid=" << (result->isValid() ? "yes" : "NO")
              << " time=" << profile.getAvg() / 1e6 << "s";

    long rss = peakRSS();
    if (rss > 0) {
        std::cout << " peakRSS=" << static_cast<double>(rss) / (1024.0 * 1024.0) << "MB";
    } else {
        std::cout << " peakRSS=unavailable";
    }
    std::cout << std::endl;
}

} // namespace

int
main(int argc, char** argv)
{
    if (argc < 2) {
        std::cerr << "usage: perf_makevalid <grid|chain|overlap|bigbowtie|sentinel>"
                     " [n] [reps] [coll]\n";
        return 1;
    }
    std::string cs = argv[1];
    std::size_t n = argc > 2 ? std::strtoul(argv[2], nullptr, 10) : 80;
    std::size_t reps = argc > 3 ? std::strtoul(argv[3], nullptr, 10) : 1;
    bool useCollection = argc > 4 && std::string(argv[4]) == "coll";

    Fixture fix = [&, cs]() -> Fixture {
        if (cs == "grid") {
            return makeGrid(n);
        }
        if (cs == "chain") {
            return makeChain(n);
        }
        if (cs == "overlap") {
            return makeOverlap(n);
        }
        if (cs == "bigbowtie") {
            return makeBigBowtie(n);
        }
        if (cs == "sentinel") {
            return makeSentinel(n);
        }
        std::cerr << "unknown case: " << cs << "\n";
        std::exit(1);
    }();

    runCase(fix, useCollection, reps);
    return 0;
}
