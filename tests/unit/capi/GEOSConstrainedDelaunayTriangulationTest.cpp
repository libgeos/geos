//
// Test Suite for C-API GEOSConstrainedDelaunayTriangulation

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capiconstrainedgeosdelaunaytriangulation_data : public capitest::utility {
    test_capiconstrainedgeosdelaunaytriangulation_data() {
    }
};

typedef test_group<test_capiconstrainedgeosdelaunaytriangulation_data> group;
typedef group::object object;

group test_capiconstrainedgeosdelaunaytriangulation_group("capi::GEOSConstrainedDelaunayTriangulation");

//
// Test Cases
//

// Empty polygon
template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON EMPTY");
    ensure_equals(GEOSisEmpty(geom1_), 1);

    geom2_ = GEOSConstrainedDelaunayTriangulation(geom1_);
    ensure (geom2_ != nullptr);
    ensure_equals(GEOSisEmpty(geom2_), 1);
    ensure_equals(GEOSGeomTypeId(geom2_), GEOS_GEOMETRYCOLLECTION);
}

// Single point
template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("POINT(0 0)");
    geom2_ = GEOSConstrainedDelaunayTriangulation(geom1_);
    ensure_equals(GEOSisEmpty(geom2_), 1);
    ensure_equals(GEOSGeomTypeId(geom2_), GEOS_GEOMETRYCOLLECTION);
}

// Polygon
template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON ((10 10, 20 40, 90 90, 90 10, 10 10))");
    geom2_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION (POLYGON ((10 10, 20 40, 90 10, 10 10)), POLYGON ((90 90, 20 40, 90 10, 90 90)))");
    geom3_ = GEOSConstrainedDelaunayTriangulation(geom1_);
    ensure_geometry_equals(geom2_, geom3_);
}



} // namespace tut

