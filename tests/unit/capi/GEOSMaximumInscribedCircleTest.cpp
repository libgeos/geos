// Test Suite for C-API GEOSMaximumInscribedCircle

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace tut{
//
// Test Group
//

// Common data used in test cases.
struct test_capimaximuminscribedcircle_data {
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
    GEOSGeometry* geom3_;

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

    test_capimaximuminscribedcircle_data()
        : geom1_(nullptr), geom2_(nullptr), geom3_(nullptr)
    {
        initGEOS(notice, notice);
    }

    ~test_capimaximuminscribedcircle_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSGeom_destroy(geom3_);
        geom1_ = nullptr;
        geom2_ = nullptr;
        geom3_ = nullptr;
        finishGEOS();
    }
};

typedef test_group<test_capimaximuminscribedcircle_data> group;
typedef group::object object;

group test_capimaximuminscribedcircle_group("capi::GEOSMaximumInscribedCircle");

//
// Test Cases
//

// Single point
template<>
template<>
void object::test<1>()
{
    geom1_ = GEOSGeomFromWKT("POINT(10 10)");
    ensure(nullptr != geom1_);
    geom2_ = GEOSMaximumInscribedCircle(geom1_, 2, NULL, NULL);
    ensure(nullptr != geom2_);
    geom3_ = GEOSGeomFromWKT("POINT(10 10)");
    ensure(nullptr != geom3_);

    bool eq = GEOSEqualsExact(geom2_, geom3_, 0.0001) != 0;
    ensure(eq);
}

} // namespace tut