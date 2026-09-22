//
// Test Suite for C-API GEOSDirectedHausdorffDistance /
// GEOSSymmetricHausdorffDistance

#include <tut/tut.hpp>
#include <geos_c.h>

#include "capi_test_utils.h"

#include <cmath>

namespace tut {

struct test_capigeosdirectedhausdorffdistance_data : public capitest::utility {
};

typedef test_group<test_capigeosdirectedhausdorffdistance_data> group;
typedef group::object object;

group test_capigeosdirectedhausdorffdistance_group(
    "capi::GEOSDirectedHausdorffDistance");

template<>
template<>
void object::test<1>()
{
    set_test_name("locus pair is not the discrete under-estimate");

    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 100 0, 10 100, 10 100)");
    geom2_ = GEOSGeomFromWKT("LINESTRING (0 100, 0 10, 80 10)");

    double discrete = 0.0;
    double directed = 0.0;
    double symmetric = 0.0;
    ensure_equals(GEOSHausdorffDistance(geom1_, geom2_, &discrete), 1);
    ensure_equals(GEOSDirectedHausdorffDistance(geom1_, geom2_, &directed), 1);
    ensure_equals(GEOSSymmetricHausdorffDistance(geom1_, geom2_, &symmetric), 1);

    ensure(discrete <= 22.360679774997898 + 1e-9);
    constexpr double LOCUS_HD = 910.0 / 19.0;
    constexpr double LOCUS_TOL = 0.05;
    ensure(std::fabs(directed - LOCUS_HD) <= LOCUS_TOL);
    ensure(std::fabs(symmetric - LOCUS_HD) <= LOCUS_TOL);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("empty writes NaN, not 0");

    geom1_ = GEOSGeomFromWKT("LINESTRING EMPTY");
    geom2_ = GEOSGeomFromWKT("LINESTRING (0 0, 2 1)");

    double dist = 0.0;
    ensure_equals(GEOSDirectedHausdorffDistance(geom1_, geom2_, &dist), 1);
    ensure(std::isnan(dist));
}

template<>
template<>
void object::test<3>()
{
    set_test_name("WithPoints realizing pair");

    geom1_ = GEOSGeomFromWKT("POINT (0 0)");
    geom2_ = GEOSGeomFromWKT("POINT (3 4)");

    double dist = 0.0;
    double p1x = 0, p1y = 0, p2x = 0, p2y = 0;
    ensure_equals(GEOSDirectedHausdorffDistanceWithPoints(
                      geom1_, geom2_, &dist, &p1x, &p1y, &p2x, &p2y), 1);
    ensure_distance(dist, 5.0, 1e-12);
    ensure_equals(p1x, 0.0);
    ensure_equals(p1y, 0.0);
    ensure_equals(p2x, 3.0);
    ensure_equals(p2y, 4.0);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("Within");

    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 2 0)");
    geom2_ = GEOSGeomFromWKT("LINESTRING (0 0, 2 1)");

    ensure_equals(GEOSDirectedHausdorffDistanceWithin(geom1_, geom2_, 0.5), 0);
    ensure_equals(GEOSDirectedHausdorffDistanceWithin(geom1_, geom2_, 1.0), 1);
    ensure_equals(GEOSDirectedHausdorffDistanceWithin(geom1_, geom2_, 2.0), 1);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("discrete C API is unchanged on the witness pair");

    geom1_ = GEOSGeomFromWKT("LINESTRING (0 0, 100 0, 10 100, 10 100)");
    geom2_ = GEOSGeomFromWKT("LINESTRING (0 100, 0 10, 80 10)");

    double discrete = 0.0;
    ensure_equals(GEOSHausdorffDistance(geom1_, geom2_, &discrete), 1);
    ensure_distance(discrete, 22.360679774997898, 1e-9);
}

} // namespace tut
