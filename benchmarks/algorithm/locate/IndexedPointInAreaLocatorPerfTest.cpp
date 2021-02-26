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

#include <geos/algorithm/locate/IndexedPointInAreaLocator.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/util/Densifier.h>
#include <geos/geom/util/SineStarFactory.h>

using geos::geom::Coordinate;
using geos::geom::GeometryFactory;
using geos::geom::util::SineStarFactory;
using geos::geom::util::Densifier;
using geos::algorithm::locate::IndexedPointInAreaLocator;

static void BM_IndexedPointInAreaLocator(benchmark::State& state) {
    auto gfact = GeometryFactory::getDefaultInstance();
    SineStarFactory ssf(gfact);
    auto poly = ssf.createSineStar();
    auto geom = Densifier::densify(poly.get(), 1);

    std::default_random_engine e(12345);
    std::uniform_real_distribution<> xdist(poly->getEnvelopeInternal()->getMinX(), poly->getEnvelopeInternal()->getMaxX());
    std::uniform_real_distribution<> ydist(poly->getEnvelopeInternal()->getMinY(), poly->getEnvelopeInternal()->getMaxY());

    IndexedPointInAreaLocator ipa(*geom);

    for (auto _ : state) {
        Coordinate c(0, 0);
        ipa.locate(&c);
    }
}

BENCHMARK(BM_IndexedPointInAreaLocator);

BENCHMARK_MAIN();

