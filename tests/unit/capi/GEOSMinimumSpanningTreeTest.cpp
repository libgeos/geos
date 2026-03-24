//
// Test Suite for C-API GEOSMinimumSpanningTree

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include <vector>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosmst_data : public capitest::utility {
};

typedef test_group<test_capigeosmst_data> group;
typedef group::object object;

group test_capigeosmst_group("capi::GEOSMinimumSpanningTree");

//
// Test Cases
//

// Standard case
template<>
template<>
void object::test<1> ()
{
    constexpr int size = 3;
    GEOSGeometry* geoms[size];
    geoms[0] = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)"); // 10
    geoms[1] = GEOSGeomFromWKT("LINESTRING(10 0, 5 10)"); // ~11.18
    geoms[2] = GEOSGeomFromWKT("LINESTRING(5 10, 0 0)"); // ~11.18

    size_t* result = GEOSMinimumSpanningTree(geoms, size);

    ensure(nullptr != result);
    
    int count = 0;
    for (int i = 0; i < size; ++i) {
        if (result[i] > 0) count++;
    }
    ensure_equals(count, 2);
    ensure(result[0] > 0); // Shortest edge must be in
    
    // Check component IDs are same
    size_t compId = 0;
    for (int i = 0; i < size; ++i) {
        if (result[i] > 0) {
            if (compId == 0) compId = result[i];
            else ensure_equals(result[i], compId);
        }
    }

    GEOSFree(result);

    for(auto& input : geoms) {
        GEOSGeom_destroy(input);
    }
}

// Disconnected graph
template<>
template<>
void object::test<2> ()
{
    constexpr int size = 2;
    GEOSGeometry* geoms[size];
    geoms[0] = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)");
    geoms[1] = GEOSGeomFromWKT("LINESTRING(20 0, 30 0)");

    size_t* result = GEOSMinimumSpanningTree(geoms, size);

    ensure(nullptr != result);
    ensure(result[0] > 0);
    ensure(result[1] > 0);
    ensure(result[0] != result[1]);

    GEOSFree(result);

    for(auto& input : geoms) {
        GEOSGeom_destroy(input);
    }
}

// Mixed inputs (non-linestrings)
template<>
template<>
void object::test<3> ()
{
    constexpr int size = 3;
    GEOSGeometry* geoms[size];
    geoms[0] = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)");
    geoms[1] = GEOSGeomFromWKT("POINT(5 5)");
    geoms[2] = GEOSGeomFromWKT("POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))");

    size_t* result = GEOSMinimumSpanningTree(geoms, size);

    ensure(nullptr != result);
    ensure(result[0] > 0);
    ensure_equals(result[1], 0u);
    ensure_equals(result[2], 0u);

    GEOSFree(result);

    for(auto& input : geoms) {
        GEOSGeom_destroy(input);
    }
}

// Empty input
template<>
template<>
void object::test<4> ()
{
    size_t* result = GEOSMinimumSpanningTree(nullptr, 0);
    ensure(nullptr == result);
}

// Null array entries
template<>
template<>
void object::test<5> ()
{
    constexpr int size = 2;
    GEOSGeometry* geoms[size];
    geoms[0] = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)");
    geoms[1] = 0;

    size_t* result = GEOSMinimumSpanningTree(geoms, size);

    ensure(nullptr != result);
    ensure(result[0] > 0);
    ensure_equals(result[1], 0u);

    GEOSFree(result);

    GEOSGeom_destroy(geoms[0]);
}

// Curved inputs
template<>
template<>
void object::test<6> ()
{
    constexpr int size = 2;
    GEOSGeometry* geoms[size];
    geoms[0] = GEOSGeomFromWKT("CIRCULARSTRING(0 0, 5 5, 10 0)");
    geoms[1] = GEOSGeomFromWKT("COMPOUNDCURVE((10 0, 10 10), CIRCULARSTRING(10 10, 5 15, 0 10), (0 10, 0 0))");

    size_t* result = GEOSMinimumSpanningTree(geoms, size);

    ensure(nullptr != result);
    // Forms a loop, MST should pick one.
    
    int count = 0;
    for (int i = 0; i < size; ++i) {
        if (result[i] > 0) count++;
    }
    ensure_equals(count, 1);

    GEOSFree(result);

    for(auto& input : geoms) {
        GEOSGeom_destroy(input);
    }
}

} // namespace tut
