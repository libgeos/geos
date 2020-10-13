// Test Suite for C-API LineString interpolate functions

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cmath>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capiinterpolate_data {
    GEOSGeometry* geom1_;

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

    test_capiinterpolate_data()
        : geom1_(nullptr)
    {
        initGEOS(notice, notice);
    }

    ~test_capiinterpolate_data()
    {
        GEOSGeom_destroy(geom1_);
        geom1_ = nullptr;
        finishGEOS();
    }

};

typedef test_group<test_capiinterpolate_data> group;
typedef group::object object;

group test_capiinterpolate_group("capi::GEOSInterpolate");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING EMPTY");
    GEOSGeometry* geom2 = GEOSInterpolate(geom1_, 1);
    ensure_equals(GEOSisEmpty(geom2), 1);
    GEOSGeom_destroy(geom2);
}

template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION EMPTY");
    GEOSGeometry* geom2 = GEOSInterpolate(geom1_, 1);
    ensure_equals(GEOSisEmpty(geom2), 1);
    GEOSGeom_destroy(geom2);
}

} // namespace tut
