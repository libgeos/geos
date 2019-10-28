//
// Test Suite for C-API GEOSGeom_getXMin et al

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace tut {
struct test_capigeosgeom_extent {
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

    test_capigeosgeom_extent()
    {
        initGEOS(notice, notice);
    }

    ~test_capigeosgeom_extent()
    {
        finishGEOS();
    }

};

typedef test_group<test_capigeosgeom_extent> group;
typedef group::object object;

group test_capigeosgeom_extent_group("capi::GEOSGeom_extent");

template<>
template<>
void object::test<1>
()
{
    GEOSGeometry* g = GEOSGeomFromWKT("LINESTRING (3 8, -12 -4)");

    double xmin, ymin, xmax, ymax;

    ensure(GEOSGeom_getXMin(g, &xmin) != 0);
    ensure(GEOSGeom_getYMin(g, &ymin) != 0);
    ensure(GEOSGeom_getXMax(g, &xmax) != 0);
    ensure(GEOSGeom_getYMax(g, &ymax) != 0);

    ensure_equals(xmin, -12);
    ensure_equals(xmax, 3);
    ensure_equals(ymin, -4);
    ensure_equals(ymax, 8);

    GEOSGeom_destroy(g);
}

template<>
template<>
void object::test<2>
()
{
    GEOSGeometry* g = GEOSGeomFromWKT("POLYGON EMPTY");

    double d;

    ensure_equals(GEOSGeom_getXMax(g, &d), 0);
    ensure_equals(GEOSGeom_getYMax(g, &d), 0);
    ensure_equals(GEOSGeom_getXMin(g, &d), 0);
    ensure_equals(GEOSGeom_getYMin(g, &d), 0);

    GEOSGeom_destroy(g);
}

} // namespace tut

