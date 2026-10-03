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
    set_test_name("maximum distance point pair within segment interior");

    geom1_ = fromWKT("LINESTRING (0 0, 100 0, 10 100, 10 100)");
    geom2_ = fromWKT("LINESTRING (0 100, 0 10, 80 10)");

    double discrete = 0.0;
    double directed12 = 0.0;
    double directed21 = 0.0;
    double symmetric = 0.0;
    ensure_equals(GEOSHausdorffDistance(geom1_, geom2_, &discrete), 1);
    ensure_equals(GEOSDirectedHausdorffDistance(geom1_, geom2_, &directed12), 1);
    ensure_equals(GEOSDirectedHausdorffDistance(geom2_, geom1_, &directed21), 1);
    ensure_equals(GEOSSymmetricHausdorffDistance(geom1_, geom2_, &symmetric), 1);

    ensure_equals("GEOSHausdorffDistance", discrete, 22.36, 1e-2);
    ensure_equals("Directed 1->2", directed12, 47.891845703125);
    ensure_equals("Directed 2->1", directed21, 44.533046060947164);
    ensure_equals("Symmetric", symmetric, 47.891845703125);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("empty writes NaN, not 0");

    geom1_ = fromWKT("LINESTRING EMPTY");
    geom2_ = fromWKT("LINESTRING (0 0, 2 1)");

    double dist = 0.0;
    ensure_equals(GEOSDirectedHausdorffDistance(geom1_, geom2_, &dist), 1);
    ensure(std::isnan(dist));
}

template<>
template<>
void object::test<3>()
{
    set_test_name("GEOSDirectedHausdorffDistanceWithPoints");

    geom1_ = fromWKT("POINT (0 0)");
    geom2_ = fromWKT("POINT (3 4)");

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
    set_test_name("GEOSDirectedHausdorffDistanceWithin");

    geom1_ = fromWKT("LINESTRING (0 0, 2 0)");
    geom2_ = fromWKT("LINESTRING (0 0, 2 1)");

    ensure_equals(GEOSDirectedHausdorffDistanceWithin(geom1_, geom2_, 0.5), 0);
    ensure_equals(GEOSDirectedHausdorffDistanceWithin(geom1_, geom2_, 1.0), 1);
    ensure_equals(GEOSDirectedHausdorffDistanceWithin(geom1_, geom2_, 2.0), 1);
}


} // namespace tut