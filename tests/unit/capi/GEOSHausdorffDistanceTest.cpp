//
// Test Suite for C-API GEOSHausdorffDistance

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <math.h>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeoshausdorffdistance_data {
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
    GEOSGeometry* geom3_;
    GEOSWKTWriter* w_;

    static void
    notice(const char* fmt, ...)
    {
        std::fprintf(stdout, "NOTICE: ");

        va_list ap;
        va_start(ap, fmt);
        std::vfprintf(stdout, fmt, ap);
        va_end(ap);

        std::fprintf(stdout, "\n");
    }

    test_capigeoshausdorffdistance_data()
        : geom1_(nullptr), geom2_(nullptr), geom3_(nullptr), w_(nullptr)
    {
        initGEOS(notice, notice);
        w_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(w_, 1);
    }

    ~test_capigeoshausdorffdistance_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSGeom_destroy(geom3_);
        GEOSWKTWriter_destroy(w_);
        geom1_ = nullptr;
        geom2_ = nullptr;
        geom3_ = nullptr;
        finishGEOS();
    }

};

typedef test_group<test_capigeoshausdorffdistance_data> group;
typedef group::object object;

group test_capigeoshausdorffdistance_group("capi::GEOSHausdorffDistance");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (130 0, 0 0, 0 150)");
    geom2_ = GEOSGeomFromWKT("LINESTRING (10 10, 10 150, 130 10)");

    double dist;
    int ret = GEOSHausdorffDistance(geom1_, geom2_, &dist);

    ensure_equals(ret, 1);
    ensure_distance(dist, 14.142135623730951, 1e-12);
}

template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (130 0, 0 0, 0 150)");
    geom2_ = GEOSGeomFromWKT("LINESTRING (10 10, 10 150, 130 10)");

    double dist;
    int ret = GEOSHausdorffDistanceDensify(geom1_, geom2_, 0.5, &dist);

    ensure_equals(ret, 1);
    ensure_distance(dist, 70., 1e-12);
}

} // namespace tut
