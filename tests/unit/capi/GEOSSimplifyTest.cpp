// 
// Test Suite for C-API GEOSSimplify

#include <tut.hpp>
// geos
#include <geos_c.h>
// std
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
    struct test_capigeossimplify_data
    {
        GEOSGeometry* geom1_;
        GEOSGeometry* geom2_;

        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capigeossimplify_data()
            : geom1_(0), geom2_(0)
        {
            initGEOS(notice, notice);
        }       

        ~test_capigeossimplify_data()
        {
            GEOSGeom_destroy(geom1_);
            GEOSGeom_destroy(geom2_);
            geom1_ = 0;
            geom2_ = 0;
            finishGEOS();
        }

    };

    typedef test_group<test_capigeossimplify_data> group;
    typedef group::object object;

    group test_capigeossimplify_group("capi::GEOSSimplify");

    //
    // Test Cases
    //

    // Test GEOSSimplify
    template<>
    template<>
    void object::test<1>()
    {
        geom1_ = GEOSGeomFromWKT("POLYGON EMPTY");

        ensure ( 0 != GEOSisEmpty(geom1_) );

        geom2_ = GEOSSimplify(geom1_, 43.2);

        ensure ( 0 != GEOSisEmpty(geom2_) );
    }
    
} // namespace tut

