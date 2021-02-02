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
    void testDensify(const std::string &wkt_input,
                            const std::string &wkt_output,
                            double tolerance)
    {
        int srid = 3857;

        GEOSGeometry *input = GEOSGeomFromWKT(wkt_input.c_str());
        GEOSSetSRID(input, srid);

        GEOSGeometry *expected = GEOSGeomFromWKT(wkt_output.c_str());

        GEOSGeometry *result = GEOSDensify(input, tolerance);
        ensure("result not NULL", result != nullptr);

        ensure_geometry_equals(result, expected);
        ensure_equals("result SRID == expected SRID", GEOSGetSRID(result), srid);

        GEOSGeom_destroy(input);
        GEOSGeom_destroy(expected);
        GEOSGeom_destroy(result);
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
    GEOSGeometry *input = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");

    GEOSGeometry *result = GEOSDensify(input, 0.0);
    ensure("result expected to be NULL", result == nullptr);

    result = GEOSDensify(input, -1.0);
    ensure("result expected to be NULL", result == nullptr);

    GEOSGeom_destroy(input);
}

} // namespace tut
