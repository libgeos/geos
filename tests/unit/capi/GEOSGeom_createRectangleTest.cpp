// Test Suite for C-API GEOSGeom_createRectangle

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "capi_test_utils.h"

namespace tut {
struct test_capiGEOSGeom_createRectangle : public capitest::utility {};

typedef test_group<test_capiGEOSGeom_createRectangle> group;
typedef group::object object;

group test_capiGEOSGeom_createRectangle_group("capi::GEOSGeom_createRectangle");

template <>
template <>
void object::test<1>() {
    GEOSGeometry* expected = GEOSGeomFromWKT("POLYGON ((0 0, 1 0, 1 2, 0 2, 0 0))");
    GEOSGeometry* geom = GEOSGeom_createRectangle(0, 0, 1, 2);

    ensure(geom != nullptr);
    ensure_equals(GEOSEqualsExact(geom, expected, 0), 1);

    GEOSGeom_destroy(expected);
    GEOSGeom_destroy(geom);
}

// verify that rectangle without width or height raises error
template <>
template <>
void object::test<2>() {
    ensure(GEOSGeom_createRectangle(0, 0, 0, 0) == nullptr);
    ensure(GEOSGeom_createRectangle(0, 0, 1, 0) == nullptr);
    ensure(GEOSGeom_createRectangle(0, 0, 0, 1) == nullptr);
}

// verify that mismatched bounds raise error
template <>
template <>
void object::test<3>() {
    ensure(GEOSGeom_createRectangle(1, 0, 0, 0) == nullptr);
    ensure(GEOSGeom_createRectangle(0, 1, 0, 0) == nullptr);
}

}  // namespace tut
