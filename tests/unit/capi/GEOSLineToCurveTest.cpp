#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstring>

#include "capi_test_utils.h"

namespace tut {

//
// Test Group
//
struct test_capigeoslinetocurve_data : public capitest::utility {
    test_capigeoslinetocurve_data() {
        params_ = GEOSLineToCurveParams_create();
    }

    ~test_capigeoslinetocurve_data() override {
        GEOSLineToCurveParams_destroy(params_);
    }

    GEOSLineToCurveParams* params_;
};

using group = test_group<test_capigeoslinetocurve_data>;
using object = group::object;

group test_capigeoslinetocurve_group("capi::GEOSLineToCurve");

//
// Test Cases
//

template<>
template<>
void object::test<1>()
{
    set_test_name("empty input preserved");

    input_ = fromWKT("LINESTRING EMPTY");
    result_ = GEOSLineToCurve(input_, params_);
    ensure_equals(GEOSGeomTypeId(result_), GEOS_LINESTRING);
    ensure(GEOSisEmpty(result_));
}

template<>
template<>
void object::test<2>()
{
    set_test_name("curved input copied");

    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    result_ = GEOSLineToCurve(input_, params_);

    ensure_geometry_equals_identical(result_, input_);
}


template<>
template<>
void object::test<3>()
{
    set_test_name("LineString to CircularString, non-default conversion parameters");

    input_ = fromWKT("LINESTRING(0 0,13.3975 50,50 86.6025,100 100,150 86.6025,186.6025 50,200 0)");

    ensure(GEOSLineToCurveParams_setRadiusTolerance(params_, 1e-5));

    result_ = GEOSLineToCurve(input_, params_);
    expected_ = fromWKT("CIRCULARSTRING (0 0, 100 100, 200 0)");

    ensure_geometry_equals_exact(result_, expected_, 1e-3);

    // max step angle too small to allow conversion
    GEOSGeom_destroy(result_);
    ensure(GEOSLineToCurveParams_setMaxStepDegrees(params_, 10));
    result_ = GEOSLineToCurve(input_, params_);
    ensure_geometry_equals_identical(result_, input_);

    // max angle difference too small to allow conversion
    GEOSGeom_destroy(result_);
    ensure(GEOSLineToCurveParams_setMaxStepDegrees(params_, 90));
    ensure(GEOSLineToCurveParams_setMaxAngleDifferenceDegrees(params_, 1e-14));
    result_ = GEOSLineToCurve(input_, params_);
    ensure_geometry_equals_identical(result_, input_);
}

} // namespace tut
