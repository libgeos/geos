//
// Test Suite for C-API GEOSPolygonize*

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include <array>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeospolygonize_data : public capitest::utility {
};

typedef test_group<test_capigeospolygonize_data> group;
typedef group::object object;

group test_capigeospolygonize_group("capi::GEOSPolygonize");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    set_test_name("GEOSPolygonizer_getCutEdges");

    constexpr int size = 2;
    GEOSGeometry* geoms[size] = { nullptr };

    geoms[0] = GEOSGeomFromWKT("LINESTRING(1 3, 3 3, 3 1, 1 1, 1 3)");
    geoms[1] = GEOSGeomFromWKT("LINESTRING(1 3, 3 3, 3 1, 1 1, 1 3)");
    GEOSSetSRID(geoms[0], 4326);

    result_ = GEOSPolygonizer_getCutEdges(geoms, size);

    ensure(result_);
    ensure_equals(GEOSGetNumGeometries(result_), size);
    ensure_equals(GEOSGetSRID(result_), 4326);

    for(auto& input : geoms) {
        GEOSGeom_destroy(input);
    }
}

template<>
template<>
void object::test<2>
()
{
    constexpr int size = 6;
    GEOSGeometry* geoms[size] = { nullptr };

    // Example from JTS Developer's Guide, Chapter 6 - Polygonization
    geoms[0] = GEOSGeomFromWKT("LINESTRING(0 0, 10 10)"); // isolated edge
    geoms[1] = GEOSGeomFromWKT("LINESTRING(185 221, 100 100)"); // dangling edge
    geoms[2] = GEOSGeomFromWKT("LINESTRING(185 221, 88 275, 180 316)");
    geoms[3] = GEOSGeomFromWKT("LINESTRING(185 221, 292 281, 180 316)");
    geoms[4] = GEOSGeomFromWKT("LINESTRING(189 98, 83 187, 185 221)");
    geoms[5] = GEOSGeomFromWKT("LINESTRING(189 98, 325 168, 185 221)");

    GEOSGeometry* g = GEOSPolygonizer_getCutEdges(geoms, size);

    ensure(nullptr != g);
    ensure_equals(GEOSGetNumGeometries(g), 0);

    GEOSGeom_destroy(g);

    for(auto& input : geoms) {
        GEOSGeom_destroy(input);
    }
}

template<>
template<>
void object::test<3>
()
{
    set_test_name("two nested rings");

    constexpr int size = 2;
    GEOSGeometry* geoms[size];
    geoms[0] = GEOSGeomFromWKT("LINESTRING (100 100, 100 300, 300 300, 300 100, 100 100)");
    geoms[1] = GEOSGeomFromWKT("LINESTRING (150 150, 150 250, 250 250, 250 150, 150 150)");
    GEOSSetSRID(geoms[0], 4326);

    // GEOSPolygonize gives us a collection of two polygons
    GEOSGeometry* g = GEOSPolygonize(geoms, size);
    ensure(nullptr != g);
    ensure_equals(GEOSGetNumGeometries(g), 2);
    ensure_equals(GEOSGeomTypeId(g), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetSRID(g), 4326);
    GEOSGeom_destroy(g);

    // GEOSPolygonize_valid gives us a single polygon with a hole
    g = GEOSPolygonize_valid(geoms, 2);

    ensure(nullptr != g);
    ensure_equals(GEOSGetNumGeometries(g), 1);
    ensure_equals(GEOSGeomTypeId(g), GEOS_POLYGON);
    ensure_equals(GEOSGetSRID(g), 4326);
    GEOSGeom_destroy(g);

    for(auto& input : geoms) {
        GEOSGeom_destroy(input);
    }
}

template<>
template<>
void object::test<4>
()
{
    set_test_name("GEOSPolygonize_valid producing an empty GeometryCollection");

    constexpr int size = 1;
    GEOSGeometry* geoms[size];
    geoms[0] = GEOSGeomFromWKT("LINESTRING (0 0, 1 1)");
    GEOSSetSRID(geoms[0], 4326);

    result_ = GEOSPolygonize_valid(geoms, size);

    ensure(result_);
    ensure_equals(GEOSGetNumGeometries(result_), 0);
    ensure_equals(GEOSGeomTypeId(result_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetSRID(result_), 4326);

    for(auto& input : geoms) {
        GEOSGeom_destroy(input);
    }
}

template<>
template<>
void object::test<5>
()
{
    set_test_name("GEOSPolygonize_valid producing a MultiPolygon");

    constexpr int size = 2;
    GEOSGeometry* geoms[size];
    geoms[0] = GEOSGeomFromWKT("LINESTRING (0 0, 1 0, 1 1, 0 1, 0 0)");
    geoms[1] = GEOSGeomFromWKT("LINESTRING (1 1, 2 1, 2 2, 1 2, 1 1)");
    GEOSSetSRID(geoms[0], 4326);

    result_ =  GEOSPolygonize_valid(geoms, size);

    ensure(result_);
    ensure_equals(GEOSGetNumGeometries(result_), 2);
    ensure_equals(GEOSGeomTypeId(result_), GEOS_MULTIPOLYGON);
    ensure_equals(GEOSGetSRID(result_), 4326);

    for(auto& input : geoms) {
        GEOSGeom_destroy(input);
    }
}

// Test GEOSPolygonize_full
template<>
template<>
void object::test<6>
()
{
    set_test_name("GEOSPolygonize_full with MultiLineString input");

    geom1_ = GEOSGeomFromWKT("MULTILINESTRING ((0 0, 1 0, 1 1, 0 1, 0 0),  (0 0, 0.5 0.5),  (1 1, 2 2, 1 2, 2 1, 1 1))");
    GEOSSetSRID(geom1_, 4326);

    GEOSGeometry* cuts;
    GEOSGeometry* dangles;
    GEOSGeometry* invalidRings;

    result_ = GEOSPolygonize_full(geom1_, &cuts, &dangles, &invalidRings);

    expected_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION(POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0)))");
    GEOSGeometry* expected_cuts = GEOSGeomFromWKT("GEOMETRYCOLLECTION EMPTY");
    GEOSGeometry* expected_dangles = GEOSGeomFromWKT("GEOMETRYCOLLECTION(LINESTRING (0 0, 0.5 0.5))");
    GEOSGeometry* expected_invalidRings = GEOSGeomFromWKT("GEOMETRYCOLLECTION(LINESTRING (1 1, 2 2, 1 2, 2 1, 1 1))");

    ensure_geometry_equals(result_, expected_);
    ensure_geometry_equals(cuts, expected_cuts);
    ensure_geometry_equals(dangles, expected_dangles);
    ensure_geometry_equals(invalidRings, expected_invalidRings);

    ensure_equals(GEOSGetSRID(result_), 4326);
    ensure_equals(GEOSGetSRID(cuts), 4326);
    ensure_equals(GEOSGetSRID(dangles), 4326);
    ensure_equals(GEOSGetSRID(invalidRings), 4326);

    GEOSGeom_destroy(cuts);
    GEOSGeom_destroy(dangles);
    GEOSGeom_destroy(invalidRings);

    GEOSGeom_destroy(expected_cuts);
    GEOSGeom_destroy(expected_dangles);
    GEOSGeom_destroy(expected_invalidRings);
}

template<>
template<>
void object::test<7>
()
{
    set_test_name("GEOSPolygonize with curved inputs");

    constexpr int size = 2;
    GEOSGeometry* geoms[size];
    geoms[0] = GEOSGeomFromWKT("LINESTRING (0 0, 2 0)");
    geoms[1] = GEOSGeomFromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    GEOSSetSRID(geoms[0], 4326);

    for (auto& geom : geoms) {
        ensure(geom != nullptr);
    }

    result_ = GEOSPolygonize(geoms, size);
    ensure(result_);

    expected_ = fromWKT("GEOMETRYCOLLECTION( CURVEPOLYGON (COMPOUNDCURVE((0 0, 2 0), CIRCULARSTRING (2 0, 1 1, 0 0))))");
    ensure_geometry_equals(result_, expected_);
    ensure_equals(GEOSGetSRID(result_), 4326);

    for(auto& input : geoms) {
        GEOSGeom_destroy(input);
    }
}

template<>
template<>
void object::test<8>
()
{
    set_test_name("GEOSPolygonize with LINESTRING ZM inputs");

    geom1_ = fromWKT("LINESTRING ZM (0 0 5 4, 2 0 6 5, 2 2 7 6)");
    geom2_ = fromWKT("LINESTRING ZM (2 2 7 6, 0 0 5 4)");

    std::array<GEOSGeometry*, 2> geoms = {geom1_, geom2_};
    result_ = GEOSPolygonize(geoms.data(), geoms.size());
    ensure(result_);

    expected_ = fromWKT("GEOMETRYCOLLECTION ZM (POLYGON ZM ((2 2 7 6, 2 0 6 5, 0 0 5 4, 2 2 7 6)))");
    ensure(expected_);

    ensure_geometry_equals_identical(result_, expected_);
}

template<>
template<>
void object::test<9>()
{
    set_test_name("GEOSPolygonize_valid with curved inputs");

    input_ = fromWKT("MULTICURVE ((10 0, 0 0, 0 10, 10 10), CIRCULARSTRING (10 10, 5 5, 10 0), COMPOUNDCURVE ((10 10, 20 10, 20 0, 10 0), CIRCULARSTRING (10 0, 15 5, 10 10)))");
    GEOSSetSRID(input_, 4326);

    result_ = GEOSPolygonize_valid(&input_, 1);
    ensure(result_);

    expected_ = fromWKT("MULTISURFACE ("
        "CURVEPOLYGON (COMPOUNDCURVE ((0 0, 0 10, 10 10), CIRCULARSTRING (10 10, 5 5, 10 0), (10 0, 0 0))),"
        "CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING (10 0, 15 5, 10 10), (10 10, 20 10, 20 0, 10 0))))");

    ensure_geometry_equals(result_, expected_);
    ensure_equals(GEOSGetSRID(result_), 4326);
}

template<>
template<>
void object::test<10>()
{
    set_test_name("GEOSPolygonize_full with curved inputs");

    input_ = fromWKT("MULTICURVE (CIRCULARSTRING (0 0, 1 1, 2 0), CIRCULARSTRING (2 0, 3 1, 4 0), LINESTRING (2 0, 0 0))");
    GEOSSetSRID(input_, 4326);

    GEOSGeometry* cuts;
    GEOSGeometry* dangles;
    GEOSGeometry* invalidRings;

    result_ = GEOSPolygonize_full(input_, &cuts, &dangles, &invalidRings);
    ensure(result_);

    expected_ = fromWKT("GEOMETRYCOLLECTION (CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING (0 0, 1 1, 2 0), (2 0, 0 0))))");
    GEOSGeometry* expected_cuts = GEOSGeomFromWKT("GEOMETRYCOLLECTION EMPTY");
    GEOSGeometry* expected_dangles = GEOSGeomFromWKT("GEOMETRYCOLLECTION(CIRCULARSTRING (2 0, 3 1, 4 0))");
    GEOSGeometry* expected_invalidRings = GEOSGeomFromWKT("GEOMETRYCOLLECTION EMPTY");

    ensure_geometry_equals(result_, expected_);
    ensure_geometry_equals(cuts, expected_cuts);
    ensure_geometry_equals(dangles, expected_dangles);
    ensure_geometry_equals(invalidRings, expected_invalidRings);

    ensure_equals(GEOSGetSRID(result_), 4326);
    ensure_equals(GEOSGetSRID(cuts), 4326);
    ensure_equals(GEOSGetSRID(dangles), 4326);
    ensure_equals(GEOSGetSRID(invalidRings), 4326);

    GEOSGeom_destroy(cuts);
    GEOSGeom_destroy(dangles);
    GEOSGeom_destroy(invalidRings);

    GEOSGeom_destroy(expected_cuts);
    GEOSGeom_destroy(expected_dangles);
    GEOSGeom_destroy(expected_invalidRings);
}

} // namespace tut