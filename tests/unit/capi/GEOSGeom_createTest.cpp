//
// Test Suite for C-API GEOSGeom_createPolygon

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosgeom_create_data {
    GEOSGeometry* geom1_;
    GEOSContextHandle_t handle_;

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

    test_capigeosgeom_create_data()
        : geom1_(nullptr), handle_(initGEOS_r(notice, notice))
    {
    }

    ~test_capigeosgeom_create_data()
    {
        GEOSGeom_destroy(geom1_);
        geom1_ = nullptr;
        finishGEOS_r(handle_);
    }

};

typedef test_group<test_capigeosgeom_create_data> group;
typedef group::object object;

group test_capigeosgeom_create_group("capi::GEOSGeom_create");

//
// Test Cases
//

// EMPTY point
template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeom_createEmptyPoint_r(handle_);
    ensure(0 != GEOSisEmpty_r(handle_, geom1_));
    ensure_equals(GEOSGeomTypeId_r(handle_, geom1_), GEOS_POINT);
    GEOSGeom_destroy(geom1_);
    geom1_ = nullptr;
}

// EMPTY linestring
template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeom_createEmptyLineString_r(handle_);
    ensure(0 != GEOSisEmpty_r(handle_, geom1_));
    ensure_equals(GEOSGeomTypeId_r(handle_, geom1_), GEOS_LINESTRING);
    GEOSGeom_destroy(geom1_);
    geom1_ = nullptr;
}


// EMPTY polygon
template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeom_createEmptyPolygon_r(handle_);
    ensure(0 != GEOSisEmpty_r(handle_, geom1_));
    ensure_equals(GEOSGeomTypeId_r(handle_, geom1_), GEOS_POLYGON);
    GEOSGeom_destroy(geom1_);
    geom1_ = nullptr;
}

// EMPTY multipoint
template<>
template<>
void object::test<4>
()
{
    geom1_ = GEOSGeom_createEmptyCollection_r(handle_, GEOS_MULTIPOINT);
    ensure(0 != GEOSisEmpty_r(handle_, geom1_));
    ensure_equals(GEOSGeomTypeId_r(handle_, geom1_), GEOS_MULTIPOINT);
    GEOSGeom_destroy(geom1_);
    geom1_ = nullptr;
}

// EMPTY multilinestring
template<>
template<>
void object::test<5>
()
{
    geom1_ = GEOSGeom_createEmptyCollection_r(handle_, GEOS_MULTILINESTRING);
    ensure(0 != GEOSisEmpty_r(handle_, geom1_));
    ensure_equals(GEOSGeomTypeId_r(handle_, geom1_), GEOS_MULTILINESTRING);
    GEOSGeom_destroy(geom1_);
    geom1_ = nullptr;
}

// EMPTY multipolygon
template<>
template<>
void object::test<6>
()
{
    geom1_ = GEOSGeom_createEmptyCollection_r(handle_, GEOS_MULTIPOLYGON);
    ensure(0 != GEOSisEmpty_r(handle_, geom1_));
    ensure_equals(GEOSGeomTypeId_r(handle_, geom1_), GEOS_MULTIPOLYGON);
    GEOSGeom_destroy(geom1_);
    geom1_ = nullptr;
}

// EMPTY collection
template<>
template<>
void object::test<7>
()
{
    geom1_ = GEOSGeom_createEmptyCollection_r(handle_, GEOS_GEOMETRYCOLLECTION);
    ensure(0 != GEOSisEmpty_r(handle_, geom1_));
    ensure_equals(GEOSGeomTypeId_r(handle_, geom1_), GEOS_GEOMETRYCOLLECTION);
    GEOSGeom_destroy(geom1_);
    geom1_ = nullptr;
}


} // namespace tut

