//
// test suite for c-api GEOSLineSubstring

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// test group
//

// common data used in test cases.
struct test_capigeoslinesubstring_data : public capitest::utility {
};

typedef test_group<test_capigeoslinesubstring_data> group;
typedef group::object object;

group test_capigeoslinesubstring_group("capi::GEOSLineSubstring");

//
// test cases
//

// basic LineString input
template<>
template<>
void object::test<1>
()
{
    input_ = GEOSGeomFromWKT("LINESTRING (0 0, 2 2)");
    result_ = GEOSLineSubstring(input_, 0, 0.5);
    expected_ = GEOSGeomFromWKT("LINESTRING (0 0, 1 1)");

    ensure_geometry_equals(result_, expected_);
}

// MultiLineString also accepted
template<>
template<>
void object::test<2>
()
{
    input_ = GEOSGeomFromWKT("MULTILINESTRING((0 0, 0 100),(0 -5, 0 0))");
    result_ = GEOSLineSubstring(input_, 0.5, 1);
    expected_ = GEOSGeomFromWKT("MULTILINESTRING ((0 52.5, 0 100), (0 -5, 0 0))");

    ensure_geometry_equals(result_, expected_);
}

// collapse returns zero-length linestring
template<>
template<>
void object::test<3>
()
{
    input_ = GEOSGeomFromWKT("LINESTRING (0 0, 2 2)");
    result_ = GEOSLineSubstring(input_, 0.5, 0.5);
    expected_ = GEOSGeomFromWKT("LINESTRING (1 1, 1 1)");

    ensure_geometry_equals(result_, expected_);
}

// fractions out of range throw an error
template<>
template<>
void object::test<4>
()
{
    input_ = GEOSGeomFromWKT("LINESTRING (0 0, 2 2)");

    result_ = GEOSLineSubstring(input_, 0.5, 1.5);
    ensure(result_ == nullptr);

    result_ = GEOSLineSubstring(input_, 0.5, -0.1);
    ensure(result_ == nullptr);
}

// Z dimension is interpolated
template<>
template<>
void object::test<5>
()
{
    set_test_name("Z dimension is interpolated, M dimension is dropped");

    input_ = GEOSGeomFromWKT("LINESTRING ZM (0 0 0 0, 2 2 5 10)");
    result_ = GEOSLineSubstring(input_, 0, 0.5);
    expected_ = GEOSGeomFromWKT("LINESTRING Z (0 0 0, 1 1 2.5)");

    ensure_geometry_equals_identical(result_, expected_);
}

// reversed fractions give a reversed substring
template<>
template<>
void object::test<6>
()
{
    input_ = GEOSGeomFromWKT("LINESTRING (0 0, 1 1)");
    result_ = GEOSLineSubstring(input_, 0.5, 0);
    expected_ = GEOSGeomFromWKT("LINESTRING(0.5 0.5, 0 0)");

    ensure_geometry_equals(result_, expected_);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("curved inputs");
    useContext();

    input_ = fromWKT("CIRCULARSTRING (-5 0, 0 5, 5 0)");
    ensure(input_);

    result_ = GEOSLineSubstring_r(ctxt_, input_, 0.5, 0);
    ensure("curved geometries not supported", result_ == nullptr);

    GEOSContext_setCurveToLineParams_r(ctxt_, curveToLineParams_);
    GEOSLineToCurveParams_setRadiusTolerance_r(ctxt_, lineToCurveParams_, 0.5);
    GEOSContext_setLineToCurveParams_r(ctxt_, lineToCurveParams_);

    result_ = GEOSLineSubstring_r(ctxt_, input_, 0.204832, 0.795167);
    ensure(result_);

    expected_ = fromWKT("CIRCULARSTRING (-4 3, 0 5, 4 3)");
    ensure_geometry_equals_exact(result_, expected_, 0.01);
}

// NaN start fraction
// https://github.com/libgeos/geos/issues/1077
template<>
template<>
void object::test<8>
()
{
    input_ = fromWKT("LINESTRING EMPTY");
    double start = std::numeric_limits<double>::quiet_NaN();
    double end = 0.1;
    result_ = GEOSLineSubstring(input_, start, end);

    ensure(result_ == nullptr);
}

template<>
template<>
void object::test<9>
()
{
    input_ = fromWKT("POLYGON ((0 0, 1 0, 1 1, 0 0))");
    result_ = GEOSLineSubstring(input_, 0.2, 0.3);

    ensure(result_ == nullptr);
}

} // namespace tut

