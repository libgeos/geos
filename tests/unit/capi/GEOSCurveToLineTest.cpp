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
struct test_capigeoscurvetoline_data : public capitest::utility {
    test_capigeoscurvetoline_data() {
        params_ = GEOSCurveToLineParams_create();
    }

    ~test_capigeoscurvetoline_data() override {
        GEOSCurveToLineParams_destroy(params_);
    }

    GEOSCurveToLineParams* params_;
};

using group = test_group<test_capigeoscurvetoline_data>;
using object = group::object;

group test_capigeoscurvetoline_group("capi::GEOSCurveToLine");

//
// Test Cases
//

template<>
template<>
void object::test<1>()
{
    set_test_name("empty input converted to linear type");

    input_ = fromWKT("CIRCULARSTRING EMPTY");
    result_ = GEOSCurveToLine(input_, params_);
    ensure_equals(GEOSGeomTypeId(result_), GEOS_LINESTRING);
    ensure(GEOSisEmpty(result_));
}

template<>
template<>
void object::test<2>()
{
    set_test_name("linear input copied");

    input_ = fromWKT("LINESTRING (0 0, 1 1, 2 0)");
    result_ = GEOSCurveToLine(input_, params_);

    ensure_geometry_equals_identical(result_, input_);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("CircularString, step size degrees");

    input_ = fromWKT("CIRCULARSTRING (0 0, 100 100, 200 0)");

    ensure(GEOSCurveToLineParams_setTolerance(params_, GEOS_CURVETOLINE_STEP_DEGREES, 30));
    result_ = GEOSCurveToLine(input_, params_);
    expected_ = fromWKT("LINESTRING(0 0,13.3975 50,50 86.6025,100 100,150 86.6025,186.6025 50,200 0)");

    ensure_geometry_equals_exact(result_, expected_, 1e-3);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("CircularString, maximum deviation");

    input_ = fromWKT("CIRCULARSTRING (0 0, 100 100, 200 0)");
    ensure(GEOSCurveToLineParams_setTolerance(params_, GEOS_CURVETOLINE_MAX_DEVIATION, 10));
    result_ = GEOSCurveToLine(input_, params_);
    expected_ = fromWKT("LINESTRING(0 0,30 70,100 100,170 70,200 0)");

    ensure_geometry_equals_exact(result_, expected_, 1.4);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("invalid tolerance type");

    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure(!GEOSCurveToLineParams_setTolerance(params_, -3, 10));
}

template<>
template<>
void object::test<6>()
{
    set_test_name("invalid tolerance value");

    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure(!GEOSCurveToLineParams_setTolerance(params_, GEOS_CURVETOLINE_STEP_DEGREES, -1));
    ensure(!GEOSCurveToLineParams_setTolerance(params_, GEOS_CURVETOLINE_STEP_DEGREES, std::numeric_limits<double>::quiet_NaN()));
}

} // namespace tut