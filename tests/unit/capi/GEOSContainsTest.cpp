// 
// Test Suite for C-API GEOSContains

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
    struct test_capigeoscontains_data
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

        test_capigeoscontains_data()
            : geom1_(0), geom2_(0)
        {
            initGEOS(notice, notice);
        }       

        ~test_capigeoscontains_data()
        {
            GEOSGeom_destroy(geom1_);
            GEOSGeom_destroy(geom2_);
            geom1_ = 0;
            geom2_ = 0;
            finishGEOS();
        }

    };

    typedef test_group<test_capigeoscontains_data> group;
    typedef group::object object;

    group test_capigeoscontains_group("capi::GEOSContains");

    //
    // Test Cases
    //

    template<>
    template<>
    void object::test<1>()
    {
        geom1_ = GEOSGeomFromWKT("POLYGON EMPTY");
        geom2_ = GEOSGeomFromWKT("POLYGON EMPTY");

        ensure( 0 != geom1_ );
        ensure( 0 != geom2_ );

        char const r1 = GEOSContains(geom1_, geom2_);

        ensure_equals(r1, 0);

        char const r2 = GEOSContains(geom2_, geom1_);

        ensure_equals(r2, 0);
    }

    template<>
    template<>
    void object::test<2>()
    {
        geom1_ = GEOSGeomFromWKT("POLYGON((1 1,1 5,5 5,5 1,1 1))");
        geom2_ = GEOSGeomFromWKT("POINT(2 2)");
        
        ensure( 0 != geom1_ );
        ensure( 0 != geom2_ );

        char const r1 = GEOSContains(geom1_, geom2_);

        ensure_equals(int(r1), 1);

        char const r2 = GEOSContains(geom2_, geom1_);

        ensure_equals(int(r2), 0);
    }
    
    template<>
    template<>
    void object::test<3>()
    {
        geom1_ = GEOSGeomFromWKT("MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)))");
        geom2_ = GEOSGeomFromWKT("POLYGON((1 1,1 2,2 2,2 1,1 1))");
        
        ensure( 0 != geom1_ );
        ensure( 0 != geom2_ );

        char const r1 = GEOSContains(geom1_, geom2_);

        ensure_equals(int(r1), 1);
        
        char const r2 = GEOSContains(geom2_, geom1_);

        ensure_equals(int(r2), 0);
    }
 
} // namespace tut

