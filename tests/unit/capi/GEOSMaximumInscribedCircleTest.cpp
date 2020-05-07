// $Id$
//
// Test Suite for C-API GEOSGetCentroid

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capimaximuminscribedcircle_data {
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
    GEOSWKTWriter* wktw_;
    char* wkt_;
    double area_;

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
        : geom1_(nullptr), geom2_(nullptr), wkt_(nullptr)
    {
        initGEOS(notice, notice);
        wktw_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setRoundingPrecision(wktw_, 8);
    }

    ~test_capimaximuminscribedcircle_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSWKTWriter_destroy(wktw_);
        GEOSFree(wkt_);
        geom1_ = nullptr;
        geom2_ = nullptr;
        wkt_ = nullptr;
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
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))");
    ensure(nullptr != geom1_);
    geom2_ = GEOSMaximumInscribedCircle(geom1_, 0.001);
    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

    ensure_equals(std::string(wkt_), std::string("LINESTRING (150 150, 150 200)"));
}

// Single point
template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("MULTIPOINT ((100 100), (100 200), (200 200), (200 100))");
    ensure(nullptr != geom1_);
    geom2_ = GEOSLargestEmptyCircle(geom1_, nullptr, 0.001);
    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

    ensure_equals(std::string(wkt_), std::string("LINESTRING (150 150, 100 100)"));
}


} // namespace tut

