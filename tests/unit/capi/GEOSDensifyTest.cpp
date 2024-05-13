//
// Test Suite for C-API GEOSDensify

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_capigeosdensify_data : public capitest::utility
{
    void testDensify(
        const std::string &wkt_input,
        const std::string &wkt_output,
        double tolerance)
    {
        int srid = 3857;

        input_ = fromWKT(wkt_input.c_str());
        GEOSSetSRID(input_, srid);

        expected_ = fromWKT(wkt_output.c_str());

        result_ = GEOSDensify(input_, tolerance);
        ensure("result not NULL", result_ != nullptr);

        ensure_geometry_equals_identical(result_, expected_);
        ensure_equals("result SRID == expected SRID", GEOSGetSRID(result_), srid);
    }
};

typedef test_group<test_capigeosdensify_data> group;
typedef group::object object;

group test_capigeosdensify_group("capi::GEOSDensify");

//
// Test Cases
//

// Densify with a tolerance greater than or equal to length of all edges.
// Result should match inputs.
template <>
template <>
void object::test<1>()
{
    testDensify(
        "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))",
        "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 2, 2 2, 2 1, 1 1))",
        10.0
    );
}

// Densify with a tolerance that evenly subdivides all outer and inner edges.
template <>
template <>
void object::test<2>()
{
    testDensify(
        "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 1 7, 7 7, 7 1, 1 1))",
        "POLYGON ((0 0, 5 0, 10 0, 10 5, 10 10, 5 10, 0 10, 0 5, 0 0), (1 1, 1 4, 1 7, 4 7, 7 7, 7 4, 7 1, 4 1, 1 1))",
        5.0
    );
}

// Densify a LINESTRING
template <>
template <>
void object::test<3>()
{
    testDensify(
        "LINESTRING (0 0, 0 6 )",
        "LINESTRING (0 0, 0 3, 0 6)",
        3.0
    );
}

// Ensure that tolerance results in the right number of subdivisions
// ceil(6 / 2.9999999) = 3 new segments; 2 new vertices
template <>
template <>
void object::test<4>()
{
    testDensify(
        "LINESTRING (0 0, 0 6 )",
        "LINESTRING (0 0, 0 2, 0 4, 0 6)",
        2.9999999
    );
}

// Densify a LINEARRING
template <>
template <>
void object::test<5>()
{
    testDensify(
        "LINEARRING (0 0, 0 6, 6 6, 0 0)",
        "LINEARRING (0 0, 0 3, 0 6, 3 6, 6 6, 4 4, 2 2, 0 0)",
        3.0
    );
}

// Densify a POINT
// Results should match inputs
template <>
template <>
void object::test<6>()
{
    testDensify(
        "POINT (0 0)",
        "POINT (0 0)",
        3.0
    );
}

// Densify a MULTIPOINT
// Results should match inputs
template <>
template <>
void object::test<7>()
{
    testDensify(
        "MULTIPOINT ((0 0), (10 10))",
        "MULTIPOINT ((0 0), (10 10))",
        3.0
    );
}

// Densify an empty polygon
// Results should match inputs
template <>
template <>
void object::test<8>()
{
    testDensify(
        "POLYGON EMPTY",
        "POLYGON EMPTY",
        3.0
    );
}

// Densify with an invalid tolerances should fail
// Note: this raises "IllegalArgumentException: Tolerance must be positive:
template <>
template <>
void object::test<9>()
{
    input_ = fromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");

    result_ = GEOSDensify(input_, 0.0);
    ensure("result expected to be NULL", result_ == nullptr);

    result_ = GEOSDensify(input_, -1.0);
    ensure("result expected to be NULL", result_ == nullptr);
}

template<>
template<>
void object::test<10>()
{
    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure(input_);

    result_ = GEOSDensify(input_, 0.1);
    ensure("curved geometries not supported", result_ == nullptr);
}
// Densify a LINESTRING Z, check that Z gets interpolated
template <>
template <>
void object::test<11>()
{
    testDensify(
        "LINESTRING Z (0 0 0, 0 6 2)",
        "LINESTRING Z (0 0 0, 0 3 1, 0 6 2)",
        3.0
    );
}

// Densify a LINEARRING Z
template <>
template <>
void object::test<12>()
{
    testDensify(
        "LINEARRING Z (0 0 0, 0 6 2, 6 6 12, 0 0 0)", 
        "LINEARRING Z (0 0 0, 0 3 1, 0 6 2, 3 6 7, 6 6 12, 4 4 8, 2 2 4, 0 0 0)",
        3.0
    );
}

// Densify a POLYGON Z
template <>
template <>
void object::test<13>()
{
    testDensify(
        "POLYGON Z ((0 0 0, 10 0 2, 10 10 10, 0 10 2, 0 0 0), (1 1 0, 1 7 0, 7 7 0, 7 1 0, 1 1 0))",
        "POLYGON Z ((0 0 0, 5 0 1, 10 0 2, 10 5 6, 10 10 10, 5 10 6, 0 10 2, 0 5 1, 0 0 0), (1 1 0, 1 4 0, 1 7 0, 4 7 0, 7 7 0, 7 4 0, 7 1 0, 4 1 0, 1 1 0))",
        5.0
    );
}

} // namespace tut
