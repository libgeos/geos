//
// Test Suite for C-API GEOSPolygonize*

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeospolygonize_data {
    static void
    notice(const char* fmt, ...)
    {
        std::fprintf(stdout, "NOTICE: ");

        va_list ap;
        va_start(ap, fmt);
        std::vfprintf(stdout, fmt, ap);
        va_end(ap);

        std::fprintf(stdout, "\n");
    }

    test_capigeospolygonize_data()
    {
        initGEOS(notice, notice);
    }

    ~test_capigeospolygonize_data()
    {
        finishGEOS();
    }

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
    constexpr int size = 2;
    GEOSGeometry* geoms[size] = { nullptr };

    geoms[0] = GEOSGeomFromWKT("LINESTRING(1 3, 3 3, 3 1, 1 1, 1 3)");
    geoms[1] = GEOSGeomFromWKT("LINESTRING(1 3, 3 3, 3 1, 1 1, 1 3)");

    GEOSGeometry* g = GEOSPolygonizer_getCutEdges(geoms, size);

    ensure(nullptr != g);
    ensure_equals(GEOSGetNumGeometries(g), size);

    GEOSGeom_destroy(g);

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
    constexpr int size = 2;
    GEOSGeometry* geoms[size];
    geoms[0] = GEOSGeomFromWKT("LINESTRING (100 100, 100 300, 300 300, 300 100, 100 100)");
    geoms[1] = GEOSGeomFromWKT("LINESTRING (150 150, 150 250, 250 250, 250 150, 150 150)");

    // GEOSPolygonize gives us a collection of two polygons
    GEOSGeometry* g = GEOSPolygonize(geoms, size);
    ensure(nullptr != g);
    ensure_equals(GEOSGetNumGeometries(g), 2);
    ensure_equals(GEOSGeomTypeId(g), GEOS_GEOMETRYCOLLECTION);
    GEOSGeom_destroy(g);

    // GEOSPolygonize_valid gives us a single polygon with a hole
    g = GEOSPolygonize_valid(geoms, 2);

    ensure(nullptr != g);
    ensure_equals(GEOSGetNumGeometries(g), 1);
    ensure_equals(GEOSGeomTypeId(g), GEOS_POLYGON);
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
    constexpr int size = 1;
    GEOSGeometry* geoms[size];
    geoms[0] = GEOSGeomFromWKT("LINESTRING (0 0, 1 1)");

    GEOSGeometry* g = GEOSPolygonize_valid(geoms, size);

    ensure(nullptr != g);
    ensure_equals(GEOSGetNumGeometries(g), 0);
    ensure_equals(GEOSGeomTypeId(g), GEOS_GEOMETRYCOLLECTION);
    GEOSGeom_destroy(g);

    for(auto& input : geoms) {
        GEOSGeom_destroy(input);
    }
}

template<>
template<>
void object::test<5>
()
{
    constexpr int size = 2;
    GEOSGeometry* geoms[size];
    geoms[0] = GEOSGeomFromWKT("LINESTRING (0 0, 1 0, 1 1, 0 1, 0 0)");
    geoms[1] = GEOSGeomFromWKT("LINESTRING (1 1, 2 1, 2 2, 1 2, 1 1)");

    GEOSGeometry* g = GEOSPolygonize_valid(geoms, size);

    ensure(nullptr != g);
    ensure_equals(GEOSGetNumGeometries(g), 2);
    ensure_equals(GEOSGeomTypeId(g), GEOS_MULTIPOLYGON);
    GEOSGeom_destroy(g);

    for(auto& input : geoms) {
        GEOSGeom_destroy(input);
    }
}

} // namespace tut

