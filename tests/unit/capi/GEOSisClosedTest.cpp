//
// Test Suite for C-API GEOSisClosed

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capiisclosed_data {
    GEOSGeometry* geom_;

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

    test_capiisclosed_data() : geom_(nullptr)
    {
        initGEOS(notice, notice);
    }

    ~test_capiisclosed_data()
    {
        GEOSGeom_destroy(geom_);
        finishGEOS();
    }

};

typedef test_group<test_capiisclosed_data> group;
typedef group::object object;

group test_capiisclosed_group("capi::GEOSisClosed");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom_ = GEOSGeomFromWKT("LINESTRING(0 0, 1 0, 1 1)");
    int r = GEOSisClosed(geom_);
    ensure_equals(r, 0);
}

template<>
template<>
void object::test<2>
()
{
    geom_ = GEOSGeomFromWKT("LINESTRING(0 0, 0 1, 1 1, 0 0)");
    int r = GEOSisClosed(geom_);
    ensure_equals(r, 1);
}

template<>
template<>
void object::test<3>
()
{
    geom_ = GEOSGeomFromWKT("MULTILINESTRING ((1 1, 1 2, 2 2, 1 1), (0 0, 0 1, 1 1))");
    int r = GEOSisClosed(geom_);
    ensure_equals(r, 0);
}

template<>
template<>
void object::test<4>
()
{
    geom_ = GEOSGeomFromWKT("MULTILINESTRING ((1 1, 1 2, 2 2, 1 1), (0 0, 0 1, 1 1, 0 0))");
    int r = GEOSisClosed(geom_);
    ensure_equals(r, 1);
}

} // namespace tut
