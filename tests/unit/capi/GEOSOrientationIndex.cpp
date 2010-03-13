// $Id$
// 
// Test Suite for C-API GEOSOrientationIndex*

#include <tut.hpp>
// geos
#include <geos_c.h>
// std
#include <string>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // Common data used in test cases.
    struct test_capigeosorientationindex_data
    {
	GEOSContextHandle_t handle_;

        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capigeosorientationindex_data()
            : handle_(initGEOS_r(notice, notice))
        {
        }       

        ~test_capigeosorientationindex_data()
        {
            finishGEOS_r(handle_);
        }

    };

    typedef test_group<test_capigeosorientationindex_data> group;
    typedef group::object object;

    group test_capigeosorientationindex_group("capi::GEOSOrientationIndex");

    //
    // Test Cases
    //

    // Interior, collinear
    template<>
    template<>
    void object::test<1>()
    {
	int ret = GEOSOrientationIndex_r(handle_, 0, 0, 10, 0, 5, 0);
	ensure_equals(ret, 0);
    }

    // Boundary (last point), collinear
    template<>
    template<>
    void object::test<2>()
    {
	int ret = GEOSOrientationIndex_r(handle_, 0, 0, 10, 0, 10, 0);
	ensure_equals(ret, 0);
    }

    // Boundary (first point), collinear
    template<>
    template<>
    void object::test<3>()
    {
	int ret = GEOSOrientationIndex_r(handle_, 0, 0, 10, 0, 0, 0);
	ensure_equals(ret, 0);
    }

    // Exterior, before first point, collinear
    template<>
    template<>
    void object::test<4>()
    {
	int ret = GEOSOrientationIndex_r(handle_, 0, 0, 10, 0, -5, 0);
	ensure_equals(ret, 0);
    }

    // Exterior, after last point, collinear
    template<>
    template<>
    void object::test<5>()
    {
	int ret = GEOSOrientationIndex_r(handle_, 0, 0, 10, 0, 20, 0);
	ensure_equals(ret, 0);
    }

    // Exterior, in bounding box, turn left
    template<>
    template<>
    void object::test<6>()
    {
	int ret = GEOSOrientationIndex_r(handle_, 0, 0, 10, 10, 5, 6);
	ensure_equals(ret, 1);
    }

    // Exterior, outside bounding box, turn left
    template<>
    template<>
    void object::test<7>()
    {
	int ret = GEOSOrientationIndex_r(handle_, 0, 0, 10, 10, 5, 20);
	ensure_equals(ret, 1);
    }

    // Exterior, in bounding box, turn right
    template<>
    template<>
    void object::test<8>()
    {
	int ret = GEOSOrientationIndex_r(handle_, 0, 0, 10, 10, 5, 3);
	ensure_equals(ret, -1);
    }

    // Exterior, outside bounding box, turn left
    template<>
    template<>
    void object::test<9>()
    {
	int ret = GEOSOrientationIndex_r(handle_, 0, 0, 10, 10, 5, -2);
	ensure_equals(ret, -1);
    }

    // Exterior, outside bounding box, very close to collinear, turn left
    template<>
    template<>
    void object::test<10>()
    {
	int ret = GEOSOrientationIndex_r(handle_, 0, 0, 10, 10, 1000000, 1000001);
	ensure_equals(ret, 1);
    }

    // Exterior, outside bounding box, very close to collinear, turn right
    template<>
    template<>
    void object::test<11>()
    {
	int ret = GEOSOrientationIndex_r(handle_, 0, 0, 10, 10, 1000000,  999999);
	ensure_equals(ret, -1);
    }




} // namespace tut

