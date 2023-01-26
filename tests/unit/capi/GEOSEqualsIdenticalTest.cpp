#include <tut/tut.hpp>
// geos
#include <geos_c.h>

// std
#include <limits>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_geosidentical_data : public capitest::utility {};

typedef test_group<test_geosidentical_data> group;
typedef group::object object;

group test_geosidentical_group("capi::GEOSEqualsIdentical");

// empty inputs of different types
template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POINT EMPTY");
    geom2_ = GEOSGeomFromWKT("LINESTRING EMPTY");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 0);
}

// empty inputs of different dimensions
template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("POINT EMPTY");
    geom2_ = GEOSGeomFromWKT("POINT Z EMPTY");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 0);
}

// non-empty inputs of different dimensions
template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("POINT Z (1 2 3)");
    geom2_ = GEOSGeomFromWKT("POINT M (1 2 3)");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 0);
}

// non-empty inputs of different dimensions
template<>
template<>
void object::test<4>
()
{
    geom1_ = GEOSGeomFromWKT("POINT ZM (1 2 3 4)");
    geom2_ = GEOSGeomFromWKT("POINT Z (1 2 3)");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 0);
}

// inputs with different structure
template<>
template<>
void object::test<5>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (1 1, 2 2)");
    geom2_ = GEOSGeomFromWKT("MULTILINESTRING ((1 1, 2 2))");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 0);
}

// inputs with different type
template<>
template<>
void object::test<6>
()
{
    geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION (LINESTRING (1 1, 2 2))");
    geom2_ = GEOSGeomFromWKT("MULTILINESTRING ((1 1, 2 2))");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 0);
}

// inputs with identical non-finite values
template<>
template<>
void object::test<7>
()
{
    geom1_ = GEOSGeom_createPointFromXY(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity());
    geom2_ = GEOSGeom_createPointFromXY(std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity());

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 1);
}

// inputs with almost-identical non-finite values
template<>
template<>
void object::test<8>
()
{
    geom1_ = GEOSGeom_createPointFromXY(std::numeric_limits<double>::quiet_NaN(), 0);
    geom2_ = GEOSGeom_createPointFromXY(std::numeric_limits<double>::signaling_NaN(), 0);

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 1);
}

// equal lines
template<>
template<>
void object::test<9>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING M (1 1 0, 2 2 1)");
    geom2_ = GEOSGeomFromWKT("LINESTRING M (1 1 0, 2 2 1)");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 1);
}

// different lines
template<>
template<>
void object::test<10>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING M (1 1 0, 2 2 1)");
    geom2_ = GEOSGeomFromWKT("LINESTRING M (1 1 1, 2 2 1)");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 0);
}

// equal polygons
template<>
template<>
void object::test<11>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON ((0 0, 1 0, 1 1, 0 0))");
    geom2_ = GEOSGeomFromWKT("POLYGON ((0 0, 1 0, 1 1, 0 0))");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 1);
}

// different polygons (ordering)
template<>
template<>
void object::test<12>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON ((0 0, 1 0, 1 1, 0 0))");
    geom2_ = GEOSGeomFromWKT("POLYGON ((1 0, 1 1, 0 0, 1 0))");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 0);
}

// different polygons (number of holes)
template<>
template<>
void object::test<13>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 2 1, 2 2, 1 1))");
    geom2_ = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (1 1, 2 1, 2 2, 1 1), (3 3, 4 3, 4 4, 3 3))");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 0);
}

// identical collections
template<>
template<>
void object::test<14>
()
{
    geom1_ = GEOSGeomFromWKT("MULTILINESTRING ((1 1, 2 2), (2 2, 3 3))");
    geom2_ = GEOSGeomFromWKT("MULTILINESTRING ((1 1, 2 2), (2 2, 3 3))");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 1);
}

// different collections (structure)
template<>
template<>
void object::test<15>
()
{
    geom1_ = GEOSGeomFromWKT("MULTILINESTRING ((1 1, 2 2), (2 2, 3 3))");
    geom2_ = GEOSGeomFromWKT("MULTILINESTRING ((2 2, 3 3), (1 1, 2 2))");

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 0);
}

// negative zero and positive zero are equal
template<>
template<>
void object::test<16>
()
{
    geom1_ = GEOSGeom_createPointFromXY(1, 0.0);
    geom2_ = GEOSGeom_createPointFromXY(1, -0.0);

    ensure_equals(GEOSEqualsIdentical(geom1_, geom2_), 1);
}

} // namespace tut

