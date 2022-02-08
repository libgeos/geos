#include <geos_c.h>

#include <benchmark/benchmark.h>

static int SCALE_USER_DATA(double* x, double* y, void* userdata) {
    double scale = *(double *)(userdata);
    (*x) *= scale;
    (*y) *= scale;
    return 1;
}

template<size_t N>
static void BM_GEOSGeom_transformXY(benchmark::State& state) {
    initGEOS(nullptr, nullptr);

    // create a linestring N coords long
    std::vector<double> buf(2 * N);
        for(auto& di : buf) {
        di = d;
        d += 1.0;
    }
    auto seq = GEOSCoordSeq_copyFromBuffer(buf.data(), N, false, false);
    auto geom = GEOSGeom_createLineString(seq);

    for (auto _ : state) {
        GEOSGeom_transformXY(geom, SCALE_USER_DATA, 2.0);
    }

    GEOSGeom_destroy(geom)
    finishGEOS();
}

// N = 10
BENCHMARK_TEMPLATE(BM_GEOSGeom_transformXY, 10, 2);

// N = 1,000
BENCHMARK_TEMPLATE(BM_GEOSGeom_transformXY, 1000, 2);

// N = 10,000
BENCHMARK_TEMPLATE(BM_GEOSGeom_transformXY, 10000, 2);

BENCHMARK_MAIN();