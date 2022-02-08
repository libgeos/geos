//
// Test Suite for C-API GEOSGeom_transformXY

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "capi_test_utils.h"

namespace tut {
struct test_capi_geosgeom_transformxy : public capitest::utility {};

typedef test_group<test_capi_geosgeom_transformxy> group;
typedef group::object object;

group test_capi_geosgeom_transformxy_group("capi::GEOSGeom_transformXY");


// callback that doesn't update coordinates should return original values
template<>
template<>
void object::test<1>
()
{
    GEOSGeometry* geom = GEOSGeomFromWKT("POINT (1 1)");

    GEOSGeometry* out = GEOSGeom_transformXY(
        geom, [](double* x, double *y, void* userdata) {
            (void)(x); // make unused parameter warning go away
            (void)(y);  // make unused parameter warning go away
            (void)(userdata); // make unused parameter warning go away
            return 1;
        }, nullptr
    );

    ensure(out != nullptr);
    ensure_equals(GEOSEquals(out, geom), 1);

    GEOSGeom_destroy(geom);
    GEOSGeom_destroy(out);
}

// failed callback should return NULL
template<>
template<>
void object::test<2>
()
{
    GEOSGeometry* geom = GEOSGeomFromWKT("POINT (1 1)");

    GEOSGeometry* out = GEOSGeom_transformXY(
        geom, [](double* x, double *y, void* userdata) {
            (void)(x); // make unused parameter warning go away
            (void)(y);  // make unused parameter warning go away
            (void)(userdata); // make unused parameter warning go away
            return 0; // indicates error
        }, nullptr
    );

    ensure(out == nullptr);

    GEOSGeom_destroy(geom);
}

// callback should modify point coordinates
template<>
template<>
void object::test<3>
()
{
    GEOSGeometry* geom = GEOSGeomFromWKT("POINT (1 1)");
    GEOSGeometry* expected = GEOSGeomFromWKT("POINT (2 3)");

    GEOSGeometry* out = GEOSGeom_transformXY(
        geom, [](double* x, double *y, void* userdata) {
            (*x) *= 2;
            (*y) *= 3;
            (void)(userdata); // make unused parameter warning go away
            return 1;
        }, nullptr
    );

    ensure(out != nullptr);
    ensure_equals(GEOSEquals(out, expected), 1);

    GEOSGeom_destroy(geom);
    GEOSGeom_destroy(expected);
    GEOSGeom_destroy(out);
}

// callback should modify linestring coordinates
template<>
template<>
void object::test<4>
()
{
    GEOSGeometry* geom = GEOSGeomFromWKT("LINESTRING (1 1, 2 2)");
    GEOSGeometry* expected = GEOSGeomFromWKT("LINESTRING (2 3, 4 6)");

    GEOSGeometry* out = GEOSGeom_transformXY(
        geom, [](double* x, double *y, void* userdata) {
            (*x) *= 2;
            (*y) *= 3;
            (void)(userdata); // make unused parameter warning go away
            return 1;
        }, nullptr
    );

    ensure(out != nullptr);
    ensure_equals(GEOSEquals(out, expected), 1);

    GEOSGeom_destroy(geom);
    GEOSGeom_destroy(expected);
    GEOSGeom_destroy(out);
}

// callback should modify polygon coordinates
template<>
template<>
void object::test<5>
()
{
    GEOSGeometry* geom = GEOSGeomFromWKT("POLYGON ((1 1, 1 10, 10 10, 10 1, 1 1),  (2 2, 2 4, 4 4, 4 2, 2 2))");
    GEOSGeometry* expected = GEOSGeomFromWKT("POLYGON ((2 3, 2 30, 20 30, 20 3, 2 3), (4 6, 4 12, 8 12, 8 6, 4 6))");

    GEOSGeometry* out = GEOSGeom_transformXY(
        geom, [](double* x, double *y, void* userdata) {
            (*x) *= 2;
            (*y) *= 3;
            (void)(userdata); // make unused parameter warning go away
            return 1;
        }, nullptr
    );

    ensure(out != nullptr);
    ensure_equals(GEOSEquals(out, expected), 1);

    GEOSGeom_destroy(geom);
    GEOSGeom_destroy(expected);
    GEOSGeom_destroy(out);
}







} // namespace tut

