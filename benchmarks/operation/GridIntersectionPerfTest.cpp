/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2025 ISciences LLC
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

#include <benchmark/benchmark.h>
#include <BenchmarkUtils.h>

#include <geos/geom/Envelope.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/operation/grid/Grid.h>

#include <geos/operation/grid/GridIntersection.h>
#include <geos/operation/intersection/Rectangle.h>
#include <geos/operation/intersection/RectangleIntersection.h>

using geos::geom::CoordinateXY;
using geos::geom::Envelope;
using geos::geom::Geometry;
using Grid = geos::operation::grid::Grid<geos::operation::grid::bounded_extent>;

template<bool AreaOnly>
struct GridIntersection {
    static double Intersection(const Envelope& env, int nx, int ny, const Geometry& g) {
         Grid grid(env, env.getWidth() / nx, env.getHeight() / ny);
         if constexpr (AreaOnly) {
             auto result = geos::operation::grid::GridIntersection::getIntersectionFractions(grid, g);
             float area = 0;
             for (std::size_t i = 0; i < result->getNumRows(); i++) {
                 for (std::size_t j = 0; j < result->getNumCols(); j++) {
                    area += (*result)(i, j);
                 }
             }
            return static_cast<double>(area);
         } else {
             auto subdivided = geos::operation::grid::GridIntersection::subdividePolygon(grid, g, true);
             return subdivided->getArea();
         }
    }
};

using GridIntersectionAreaOnly = GridIntersection<true>;
using GridIntersectionFull = GridIntersection<false>;

template<bool UseRectangleIntersection>
struct SingleIntersection {

    static double Intersection(const Envelope& env, int nx, int ny, const Geometry& g) {
        double dx = env.getWidth() / nx;
        double dy = env.getHeight() / ny;

        double x0 = env.getMinX();
        double y0 = env.getMinY();

        const auto& gfact = *g.getFactory();
        auto prepGeom = geos::geom::prep::PreparedGeometryFactory::prepare(&g);

        double area = 0;

        for (int i = 0; i < nx; i++) {
            for (int j = 0; j < ny; j++) {
                Envelope subEnv(x0 + i*dx, x0 + (i+1)*dx, y0 + j*dy, y0 + (j+1)*dy);
                auto cellGeom = gfact.toGeometry(&subEnv);
                if (!prepGeom->intersects(cellGeom.get())) {
                    continue;
                }

                std::unique_ptr<Geometry> isect;
                if constexpr (UseRectangleIntersection) {
                    geos::operation::intersection::Rectangle rect(x0 + i*dx, y0 + j*dy, x0 + (i+1)*dx, y0 + (j+1)*dy);
                    isect = geos::operation::intersection::RectangleIntersection::clip(g, rect);
                } else {
                    isect = g.intersection(cellGeom.get());
                }

                area += isect->getArea();
            }
        }

        return area;
    }
};

using PolygonIntersection = SingleIntersection<false>;
using RectangleIntersection = SingleIntersection<true>;

template<typename Impl>
static void BM_GridIntersection(benchmark::State& state)
{
    auto nCells = state.range(0);

    auto nx = static_cast<int>(std::ceil(std::sqrt(nCells)));
    auto ny = static_cast<int>(std::ceil(std::sqrt(nCells)));

    CoordinateXY center;
    Envelope env(0, nx, 0, ny);
    env.centre(center);

    auto geom = geos::benchmark::createSineStar(center, env.getWidth() / 2, 500);

    for (auto _ : state) {
        Impl::Intersection(env, nx, ny, *geom);
    }

}

BENCHMARK_TEMPLATE(BM_GridIntersection, GridIntersectionAreaOnly)->Range(1000, 1000000);
BENCHMARK_TEMPLATE(BM_GridIntersection, GridIntersectionFull)->Range(1000, 1000000);
BENCHMARK_TEMPLATE(BM_GridIntersection, RectangleIntersection)->Range(1000, 1000000);
BENCHMARK_TEMPLATE(BM_GridIntersection, PolygonIntersection)->Range(1000, 1000000);

BENCHMARK_MAIN();
