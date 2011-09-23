// 
// Test Suite for C-API GEOSGeom_extractUniquePoints_r

#include <tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace tut
{
    //
    // Test Group
    //

    // Common data used in test cases.
    struct test_capigeosextractuniquepoints_data
    {
        GEOSGeometry* geom1_;
        GEOSGeometry* geom2_;
        GEOSGeometry* geom3_;
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

        test_capigeosextractuniquepoints_data()
                : geom1_(0), geom2_(0), geom3_(0)
        {
            handle_ = initGEOS_r(notice, notice);
        }       

        ~test_capigeosextractuniquepoints_data()
        {
            GEOSGeom_destroy_r(handle_, geom1_);
            GEOSGeom_destroy_r(handle_, geom2_);
            if( geom3_ )
                GEOSGeom_destroy_r(handle_, geom3_);

            geom1_ = 0;
            geom2_ = 0;
            geom3_ = 0;
            finishGEOS_r(handle_);
        }

    };

    typedef test_group<test_capigeosextractuniquepoints_data> group;
    typedef group::object object;

    group test_capigeosextractuniquepoints_group("capi::GEOSGeom_extractUniquePoints");

    //
    // Test Cases
    //

    template<>
    template<>
    void object::test<1>()
    {
        geom1_ = GEOSGeomFromWKT_r(handle_, "POLYGON EMPTY");
        /* ensure_equals(GEOSGetNumGeometries_r(handle_, geom2_), 0); */
        geom3_ = GEOSGeom_extractUniquePoints_r(handle_, geom1_);
        ensure(0 != GEOSisEmpty_r(handle_, geom3_));
    }

    template<>
    template<>
    void object::test<2>()
    {
        geom1_ = GEOSGeomFromWKT_r(handle_, "MULTIPOINT(0 0, 0 0, 1 1)");
        geom2_ = GEOSGeomFromWKT_r(handle_, "MULTIPOINT(0 0, 1 1)");
        /* ensure_equals(GEOSGetNumGeometries_r(handle_, geom2_), 0); */
        geom3_ = GEOSGeom_extractUniquePoints_r(handle_, geom1_);
        ensure(0 != GEOSEquals_r(handle_, geom3_, geom2_));
    }

    template<>
    template<>
    void object::test<3>()
    {
        geom1_ = GEOSGeomFromWKT_r(handle_, "GEOMETRYCOLLECTION(MULTIPOINT(0 0, 0 0, 1 1),LINESTRING(1 1, 2 2, 2 2, 0 0),POLYGON((5 5, 0 0, 0 2, 2 2, 5 5)))");
        geom2_ = GEOSGeomFromWKT_r(handle_, "MULTIPOINT(0 0, 1 1, 2 2, 5 5, 0 2)");
        geom3_ = GEOSGeom_extractUniquePoints_r(handle_, geom1_);
        /* ensure_equals(GEOSGetNumGeometries_r(handle_, geom2_), 0); */
        ensure(0 != GEOSEquals_r(handle_, geom3_, geom2_));
    }

 
} // namespace tut

