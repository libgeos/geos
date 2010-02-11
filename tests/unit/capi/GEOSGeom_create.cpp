// $Id: GEOSGeomToWKTTest.cpp 2424 2009-04-29 23:52:36Z mloskot $
// 
// Test Suite for C-API GEOSGeom_createPolygon

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
    struct test_capigeosgeom_create_data
    {
        GEOSGeometry* geom1_;
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

        test_capigeosgeom_create_data()
            : geom1_(0), handle_(initGEOS_r(notice, notice))
        {
        }       

        ~test_capigeosgeom_create_data()
        {
            GEOSGeom_destroy(geom1_); geom1_ = 0;
            finishGEOS_r(handle_);
        }

    };

    typedef test_group<test_capigeosgeom_create_data> group;
    typedef group::object object;

    group test_capigeosgeom_create_group("capi::GEOSGeom_create");

    //
    // Test Cases
    //

    // EMPTY polygon
    template<>
    template<>
    void object::test<1>()
    {
        geom1_ = GEOSGeom_createEmptyPolygon_r(handle_);
        ensure(GEOSisEmpty_r(handle_, geom1_));
	ensure_equals(GEOSGeomTypeId_r(handle_, geom1_), GEOS_POLYGON);
	GEOSGeom_destroy(geom1_); geom1_=0;
    }
    

} // namespace tut

