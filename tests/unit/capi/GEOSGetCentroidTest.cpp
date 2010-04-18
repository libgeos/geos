// $Id$
// 
// Test Suite for C-API GEOSGetCentroid

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
    struct test_capigeosgetcentroid_data
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

        test_capigeosgetcentroid_data() 
            : geom1_(0), geom2_(0)
        {
            initGEOS(notice, notice);
        }       

        ~test_capigeosgetcentroid_data()
        {
            GEOSGeom_destroy(geom1_);
            GEOSGeom_destroy(geom2_);
            geom1_ = 0;
            geom2_ = 0;
            finishGEOS();
        }

    };

    typedef test_group<test_capigeosgetcentroid_data> group;
    typedef group::object object;

    group test_capigeosgetcentroid_group("capi::GEOSGetCentroid");

    //
    // Test Cases
    //

    /* More of a leak test */
    template<>
    template<>
    void object::test<1>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)");

        ensure ( ! GEOSisEmpty(geom1_) );

	geom2_ = GEOSGetCentroid(geom1_);

        ensure ( ! GEOSisEmpty(geom2_) );

    }   
    
    
} // namespace tut

