// Test Suite for C-API MakeValid

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
struct test_capimakevalid_data {
    GEOSWKTWriter* wktw_;
    GEOSGeometry* geom1_ = nullptr;
    GEOSGeometry* geom2_ = nullptr;

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

    std::string
    toWKT(GEOSGeometry* g)
    {
        char* wkt = GEOSWKTWriter_write(wktw_, g);
        std::string ret(wkt);
        GEOSFree(wkt);
        return ret;
    }

    test_capimakevalid_data()
    {
        initGEOS(notice, notice);
        wktw_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setOutputDimension(wktw_, 3);
    }

    ~test_capimakevalid_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSWKTWriter_destroy(wktw_);
        finishGEOS();
    }

};

typedef test_group<test_capimakevalid_data> group;
typedef group::object object;

group test_capimakevalid_group("capi::GEOSMakeValid");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((0 0,1 1,0 1,1 0,0 0))");
    geom2_ = GEOSMakeValid(geom1_);
    ensure_equals(toWKT(geom2_), std::string("MULTIPOLYGON (((0 0, 0.5 0.5, 1 0, 0 0)), ((0.5 0.5, 0 1, 1 1, 0.5 0.5)))"));
}
} // namespace tut
