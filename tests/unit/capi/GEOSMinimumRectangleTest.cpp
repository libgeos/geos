//
// Test Suite for C-API GEOSMinimumRotatedRectangle

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosminimumrectangle_data {
    GEOSGeometry* input_;
    GEOSWKTWriter* wktw_;
    char* wkt_;

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

    test_capigeosminimumrectangle_data()
        : input_(nullptr), wkt_(nullptr)
    {
        initGEOS(notice, notice);
        wktw_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setRoundingPrecision(wktw_, 8);
    }

    ~test_capigeosminimumrectangle_data()
    {
        GEOSGeom_destroy(input_);
        input_ = nullptr;
        GEOSWKTWriter_destroy(wktw_);
        GEOSFree(wkt_);
        wkt_ = nullptr;
        finishGEOS();
    }

};

typedef test_group<test_capigeosminimumrectangle_data> group;
typedef group::object object;

group test_capigeosminimumrectangle_group("capi::GEOSMinimumRectangle");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    input_ = GEOSGeomFromWKT("POLYGON ((1 6, 6 11, 11 6, 6 1, 1 6))");
    ensure(nullptr != input_);

    GEOSGeometry* output = GEOSMinimumRotatedRectangle(input_);
    ensure(nullptr != output);
    ensure(0 == GEOSisEmpty(output));

    wkt_ = GEOSWKTWriter_write(wktw_, output);
    ensure_equals(std::string(wkt_), std::string("POLYGON ((6 1, 11 6, 6 11, 1 6, 6 1))"));

    GEOSGeom_destroy(output);
}

} // namespace tut
