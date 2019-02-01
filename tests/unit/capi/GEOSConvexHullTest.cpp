//
// Test Suite for C-API GEOSConvexHull

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
struct test_capigeosconvexhull_data {
    GEOSGeometry* input_;
    GEOSGeometry* expected_;

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

    test_capigeosconvexhull_data()
        : input_(nullptr), expected_(nullptr)
    {
        initGEOS(notice, notice);
    }

    ~test_capigeosconvexhull_data()
    {
        GEOSGeom_destroy(input_);
        GEOSGeom_destroy(expected_);
        input_ = nullptr;
        expected_ = nullptr;
        finishGEOS();
    }

};

typedef test_group<test_capigeosconvexhull_data> group;
typedef group::object object;

group test_capigeosconvexhull_group("capi::GEOSConvexHull");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    input_ = GEOSGeomFromWKT("MULTIPOINT (130 240, 130 240, 130 240, 570 240, 570 240, 570 240, 650 240)");
    ensure(nullptr != input_);

    expected_ = GEOSGeomFromWKT("LINESTRING (130 240, 650 240, 130 240)");
    ensure(nullptr != expected_);

    GEOSGeometry* output = GEOSConvexHull(input_);
    ensure(nullptr != output);
    ensure(0 == GEOSisEmpty(output));
    // TODO
    //ensure( 0 != GEOSEquals(output, expected_));
    GEOSGeom_destroy(output);
}

} // namespace tut

