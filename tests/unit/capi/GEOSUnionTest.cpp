#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosunion_data : public capitest::utility {};

typedef test_group<test_geosunion_data> group;
typedef group::object object;

group test_geosunion("capi::GEOSUnion");

template<>
template<>
void object::test<1>()
{
    GEOSGeometry* a = GEOSGeomFromWKT("POINT (2 8)");
    GEOSGeometry* b = GEOSGeomFromWKT("POINT (3 9)");

    ensure(a);
    ensure(b);

    GEOSSetSRID(a, 4326);

    GEOSGeometry* result = GEOSUnion(a, b);
    GEOSGeometry* expected = GEOSGeomFromWKT("MULTIPOINT ((2 8), (3 9))");

    ensure(result);
    ensure(expected);

    ensure_equals(GEOSEqualsExact(result, expected, 0), 1);
    ensure_equals(GEOSGetSRID(a), GEOSGetSRID(result));

    GEOSGeom_destroy(a);
    GEOSGeom_destroy(b);
    GEOSGeom_destroy(result);
    GEOSGeom_destroy(expected);
}

// Verify that no crash occurs in overlay with NaN coordinates
// https://github.com/libgeos/geos/issues/606
template<>
template<>
void object::test<2>()
{
    std::string wkb1 = "010100000000000000000000000000000000000840";
    std::string wkb2 = "01020000000300000049544C553736090000FFFF544E494F500000000000000000FFFFFF2B2B2B2B2BFFFFFFFFFFFFFFFF00FFFFFFFFFFFFFF";

    geom1_ = GEOSGeomFromHEX_buf((unsigned char*) wkb1.c_str(), wkb1.size());
    geom2_ = GEOSGeomFromHEX_buf((unsigned char*) wkb2.c_str(), wkb2.size());

    ensure(geom1_);
    ensure(geom2_);

    GEOSGeometry* result = GEOSUnion(geom1_, geom2_);
    (void) result; // no crash
}

template<>
template<>
void object::test<3>()
{
    set_test_name("curved inputs");

    geom1_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = fromWKT("LINESTRING (1 0, 2 1)");

    result_ = GEOSUnion(geom1_, geom2_);
    ensure(result_);

    expected_ = fromWKT("MULTICURVE (CIRCULARSTRING (1.7071067812 0.7071067812, 1.9238795325 0.3826834324, 2 0), CIRCULARSTRING (0 0, 0.6173165676 0.9238795325, 1.7071067812 0.7071067812), (1.7071067812 0.7071067812, 2 1), (1 0, 1.7071067812 0.7071067812))");
    ensure_geometry_equals(result_, expected_, 1e-8);
}

} // namespace tut

