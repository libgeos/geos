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

#include <geos_c.h>

#include <benchmark/benchmark.h>

static std::vector<double> create_buffer(std::size_t N, unsigned int dim) {
    std::vector<double> buf(2 * N);
    double d = 0.0;
    for(auto& di : buf) {
        di = d;
        d += 1.0;
    }

    return buf;
}

template<size_t N, size_t dim>
static void BM_CoordSeq_CreateByOrdinate(benchmark::State& state) {
    initGEOS(nullptr, nullptr);
    auto buf = create_buffer(N, dim);

    for (auto _ : state) {
        double* ptr = buf.data();
        auto seq = GEOSCoordSeq_create(N, dim);

        for (std::size_t i = 0; i < N; i++) {
            GEOSCoordSeq_setX(seq, i, *ptr++);
            GEOSCoordSeq_setY(seq, i, *ptr++);
            if (dim == 3) {
                GEOSCoordSeq_setZ(seq, i, *ptr++);
            }
        }

        GEOSCoordSeq_destroy(seq);
    }

    finishGEOS();
}

template<size_t N, size_t dim>
static void BM_CoordSeq_CreateByCoordinate(benchmark::State& state) {
    initGEOS(nullptr, nullptr);

    auto buf = create_buffer(N, dim);

    for (auto _ : state) {
        double* ptr = buf.data();
        auto seq = GEOSCoordSeq_create(N, dim);

        for (std::size_t i = 0; i < N; i++) {
            double x = *ptr++;
            double y = *ptr++;
            if (dim == 2) {
                GEOSCoordSeq_setXY(seq, i, x, y);
            }
            if (dim == 3) {
                double z = *ptr++;
                GEOSCoordSeq_setXYZ(seq, i, x, y, z);
            }
        }

        GEOSCoordSeq_destroy(seq);
    }

    finishGEOS();
}

template<size_t N, size_t dim>
static void BM_CoordSeq_CopyFromBuffer(benchmark::State& state) {
    initGEOS(nullptr, nullptr);

    auto buf = create_buffer(N, dim);

    for (auto _ : state) {
        auto seq = GEOSCoordSeq_copyFromBuffer(buf.data(), N, dim);
        GEOSCoordSeq_destroy(seq);
    }

    finishGEOS();
}

// N = 10
BENCHMARK_TEMPLATE(BM_CoordSeq_CreateByOrdinate, 10, 2);
BENCHMARK_TEMPLATE(BM_CoordSeq_CreateByCoordinate, 10, 2);
BENCHMARK_TEMPLATE(BM_CoordSeq_CopyFromBuffer, 10, 2);

BENCHMARK_TEMPLATE(BM_CoordSeq_CreateByOrdinate, 10, 3);
BENCHMARK_TEMPLATE(BM_CoordSeq_CreateByCoordinate, 10, 3);
BENCHMARK_TEMPLATE(BM_CoordSeq_CopyFromBuffer, 10, 3);

// N = 1,0000
BENCHMARK_TEMPLATE(BM_CoordSeq_CreateByOrdinate, 1000, 2);
BENCHMARK_TEMPLATE(BM_CoordSeq_CreateByCoordinate, 1000, 2);
BENCHMARK_TEMPLATE(BM_CoordSeq_CopyFromBuffer, 1000, 2);

BENCHMARK_TEMPLATE(BM_CoordSeq_CreateByOrdinate, 1000, 3);
BENCHMARK_TEMPLATE(BM_CoordSeq_CreateByCoordinate, 1000, 3);
BENCHMARK_TEMPLATE(BM_CoordSeq_CopyFromBuffer, 1000, 3);

// N = 10,000
BENCHMARK_TEMPLATE(BM_CoordSeq_CreateByOrdinate, 10000, 2);
BENCHMARK_TEMPLATE(BM_CoordSeq_CreateByCoordinate, 10000, 2);
BENCHMARK_TEMPLATE(BM_CoordSeq_CopyFromBuffer, 10000, 2);

BENCHMARK_TEMPLATE(BM_CoordSeq_CreateByOrdinate, 10000, 3);
BENCHMARK_TEMPLATE(BM_CoordSeq_CreateByCoordinate, 10000, 3);
BENCHMARK_TEMPLATE(BM_CoordSeq_CopyFromBuffer, 10000, 3);

BENCHMARK_MAIN();


