#include <tut/tut.hpp>
// geos
#include <geos_c.h>
#include <geos/constants.h>

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
    set_test_name("curved inputs, curved output");
    useContext();

    geom1_ = fromWKT("CURVEPOLYGON (COMPOUNDCURVE( CIRCULARSTRING (-5 0, 0 5, 5 0), (5 0, -5 0)))");
    geom2_ = fromWKT("CURVEPOLYGON (COMPOUNDCURVE( CIRCULARSTRING (-5 0, 0 -5, 5 0), (5 0, -5 0)))");

    ensure(geom1_);
    ensure(geom2_);
    GEOSSetSRID_r(ctxt_, geom1_, 4326);

    result_ = GEOSUnion_r(ctxt_, geom1_, geom2_);
    ensure("curved geometry not supported", result_ == nullptr);

    GEOSCurveToLineParams_setTolerance_r(ctxt_, curveToLineParams_, GEOS_CURVETOLINE_STEP_DEGREES, 1);
    GEOSContext_setCurveToLineParams_r(ctxt_, curveToLineParams_);

    // Input converted to line, output not converted to curve
    result_ = GEOSUnion_r(ctxt_, geom1_, geom2_);
    ensure(result_);
    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_POLYGON);
    {
        double area = -1;
        ensure_equals(GEOSArea_r(ctxt_, result_, &area), 1);
        ensure_equals("area does not match expected", area, geos::MATH_PI*5*5, 5e-3);
    }
    ensure_equals(GEOSGetSRID_r(ctxt_, result_), 4326);
    GEOSGeom_destroy_r(ctxt_, result_);

    // Input converted to line, output converted to curve
    GEOSContext_setLineToCurveParams_r(ctxt_, lineToCurveParams_);
    result_ = GEOSUnion_r(ctxt_, geom1_, geom2_);

    ensure_equals(GEOSGeomTypeId_r(ctxt_, result_), GEOS_CURVEPOLYGON);
    {
        double area = -1;
        ensure_equals(GEOSArea_r(ctxt_, result_, &area), 1);
        ensure_equals("area does not match expected", area, geos::MATH_PI*5*5, 1e-6);
    }
    ensure_equals(GEOSGetSRID_r(ctxt_, result_), 4326);
}

} // namespace tut

