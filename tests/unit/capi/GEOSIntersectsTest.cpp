// 
// Test Suite for C-API GEOSIntersects

#include <tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace tut
{
    //
    // Test Group
    //

    // Common data used in test cases.
    struct test_capigeosintersects_data
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

        test_capigeosintersects_data()
            : geom1_(0), geom2_(0)
        {
            initGEOS(notice, notice);
        }       

        ~test_capigeosintersects_data()
        {
            GEOSGeom_destroy(geom1_);
            GEOSGeom_destroy(geom2_);
            geom1_ = 0;
            geom2_ = 0;
            finishGEOS();
        }

    };

    typedef test_group<test_capigeosintersects_data> group;
    typedef group::object object;

    group test_capigeosintersects_group("capi::GEOSIntersects");

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

        char const r1 = GEOSIntersects(geom1_, geom2_);

        ensure_equals(r1, 0);

        char const r2 = GEOSIntersects(geom2_, geom1_);

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

        char const r1 = GEOSIntersects(geom1_, geom2_);

        ensure_equals(int(r1), 1);

        char const r2 = GEOSIntersects(geom2_, geom1_);

        ensure_equals(int(r2), 1);
    }
    
    template<>
    template<>
    void object::test<3>()
    {
        geom1_ = GEOSGeomFromWKT("MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)))");
        geom2_ = GEOSGeomFromWKT("POLYGON((1 1,1 2,2 2,2 1,1 1))");
        
        ensure( 0 != geom1_ );
        ensure( 0 != geom2_ );

        char const r1 = GEOSIntersects(geom1_, geom2_);

        ensure_equals(int(r1), 1);
        
        char const r2 = GEOSIntersects(geom2_, geom1_);

        ensure_equals(int(r2), 1);
    }

    // This is a test for bug #357 (GEOSIntersects with nan coords)
    template<>
    template<>
    void object::test<4>()
    {
        GEOSCoordSequence* cs = GEOSCoordSeq_create(5, 2);

        double nan = std::numeric_limits<double>::quiet_NaN();        
        GEOSCoordSeq_setX(cs, 0, 1); GEOSCoordSeq_setY(cs, 0, 1);
        for (unsigned int i=1; i<4; ++i) {
            GEOSCoordSeq_setX(cs, i, nan);
            GEOSCoordSeq_setY(cs, i, nan);
        }
        GEOSCoordSeq_setX(cs, 4, 1); GEOSCoordSeq_setY(cs, 4, 1);

        geom1_ = GEOSGeom_createPolygon(GEOSGeom_createLinearRing(cs),
                                        NULL, 0);

        char const r1 = GEOSIntersects(geom1_, geom1_);

        ensure_equals(int(r1), 2);
        
    }

    // This is a test for bug #357 (GEOSIntersects with inf coords)
    template<>
    template<>
    void object::test<5>()
    {
        const char *hex = "0103000020E61000000100000005000000737979F3DDCC2CC0F92154F9E7534540000000000000F07F000000000000F07F8F806E993F7E55C0304B29FFEA8554400634E8D1DD424540B5FEE6A37FCD4540737979F3DDCC2CC0F92154F9E7534540";

        geom1_ = GEOSGeomFromHEX_buf((unsigned char*)hex, std::strlen(hex));
        
        ensure( 0 != geom1_ );

        char const r1 = GEOSIntersects(geom1_, geom1_);

        ensure_equals(int(r1), 2);
        
    }
 
} // namespace tut

