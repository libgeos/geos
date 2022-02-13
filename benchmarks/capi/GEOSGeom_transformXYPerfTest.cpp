#include <iostream>

#include <benchmark/benchmark.h>
#include <geos_c.h>

static int SCALE_USER_DATA(double* x, double* y, void* userdata) {
    double scale = *(double*)(userdata);
    (*x) *= scale;
    (*y) *= scale;
    return 1;
}

GEOSGeometry* create_polygon(size_t N) {
    // create a linearring N coords long
    std::vector<double> buf(2 * (N + 1));
    double d = 0.0;
    for (auto& di : buf) {
        di = d;
        d += 1.0;
    }

    // close the ring
    buf[N * 2] = buf[0];
    buf[(N * 2) + 1] = buf[1];

    auto seq = GEOSCoordSeq_copyFromBuffer(buf.data(), N + 1, false, false);
    assert(seq != nullptr);
    auto ring = GEOSGeom_createLinearRing(seq);
    return GEOSGeom_createPolygon(ring, nullptr, 0);
}

template <size_t N>
static void BM_GEOSGeom_transformXY(benchmark::State& state) {
    initGEOS(nullptr, nullptr);

    auto geom = create_polygon(N);
    GEOSGeometry *out_geom;

    double scale = 2.0;

    for (auto _ : state) {
        out_geom = GEOSGeom_transformXY(geom, SCALE_USER_DATA, (void*)(&scale));

        // not really part of benchmark but need to cleanup geom
        GEOSGeom_destroy(out_geom);
    }

    GEOSGeom_destroy(geom);
    finishGEOS();
}

template <size_t N>
static void BM_Geom_from_transformed_coords(benchmark::State& state) {
    initGEOS(nullptr, nullptr);

    const GEOSCoordSequence *seq;
    GEOSCoordSequence *out_seq;
    unsigned int i, size;
    double x, y, scale=2.0;
    const GEOSGeometry *ring;
    GEOSGeometry *out_ring, *out_geom;

    auto geom = create_polygon(N);

    for (auto _ : state) {
        ring = GEOSGetExteriorRing(geom);
        seq = GEOSGeom_getCoordSeq(ring);
        GEOSCoordSeq_getSize(seq, &size);
        out_seq = GEOSCoordSeq_create(size, 2);

        for (i = 0; i < size; i++) {
            GEOSCoordSeq_getXY(seq, i, &x, &y);
            SCALE_USER_DATA(&x, &y, (void *)(&scale));
            GEOSCoordSeq_setXY(out_seq, i, x, y);
        }

        out_ring = GEOSGeom_createLinearRing(out_seq);
        out_geom = GEOSGeom_createPolygon(out_ring, nullptr, 0);

        // not really part of benchmark but need to cleanup geom
        GEOSGeom_destroy(out_geom);
    }

    GEOSGeom_destroy(geom);
    finishGEOS();
}

// N = 10
BENCHMARK_TEMPLATE(BM_GEOSGeom_transformXY, 10);
BENCHMARK_TEMPLATE(BM_Geom_from_transformed_coords, 10);

// N = 1,000
BENCHMARK_TEMPLATE(BM_GEOSGeom_transformXY, 1000);
BENCHMARK_TEMPLATE(BM_Geom_from_transformed_coords, 1000);

// N = 10,000
BENCHMARK_TEMPLATE(BM_GEOSGeom_transformXY, 10000);
BENCHMARK_TEMPLATE(BM_Geom_from_transformed_coords, 10000);

BENCHMARK_MAIN();