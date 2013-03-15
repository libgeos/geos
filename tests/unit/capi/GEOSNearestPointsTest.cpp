// $Id: GEOSNearestPointsTest.cpp 2424 2009-04-29 23:52:36Z mloskot $
// 
// Test Suite for C-API GEOSNearestPoints

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
    struct test_capigeosnearestpoints_data
    {
        GEOSGeometry *geom1_;
        GEOSGeometry *geom2_;

        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capigeosnearestpoints_data()
            : geom1_(0), geom2_(0)
        {
            initGEOS(notice, notice);
        }       

        ~test_capigeosnearestpoints_data()
        {
            GEOSGeom_destroy(geom1_);
            GEOSGeom_destroy(geom2_);
            geom1_ = 0;
            geom2_ = 0;
            finishGEOS();
        }

    };

    typedef test_group<test_capigeosnearestpoints_data> group;
    typedef group::object object;

    group test_capigeosnearestpoints_group("capi::GEOSNearestPoints");

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

        GEOSCoordSequence *coords_;
        coords_ = GEOSNearestPoints(geom1_, geom2_);

        ensure( 0 == coords_ );
    }

    template<>
    template<>
    void object::test<2>()
    {
        geom1_ = GEOSGeomFromWKT("POLYGON((1 1,1 5,5 5,5 1,1 1))");
        // geom2_ = GEOSGeomFromWKT("POINT(8 8)");
        geom2_ = GEOSGeomFromWKT("POLYGON((8 8, 9 9, 9 10, 8 8))");
        
        ensure( 0 != geom1_ );
        ensure( 0 != geom2_ );

        GEOSCoordSequence *coords_;
        coords_ = GEOSNearestPoints(geom1_, geom2_);

        ensure( 0 != coords_ );

        unsigned int size;
        GEOSCoordSeq_getSize(coords_, &size);
        ensure( 2 == size );

        double  x1, x2, y1, y2;

        /* Point in geom1_
         */
        GEOSCoordSeq_getOrdinate(coords_, 0, 0, &x1);
        GEOSCoordSeq_getOrdinate(coords_, 0, 1, &y1);

        /* Point in geom2_
         */
        GEOSCoordSeq_getOrdinate(coords_, 1, 0, &x2);
        GEOSCoordSeq_getOrdinate(coords_, 1, 1, &y2);

        ensure( 5 == x1 );
        ensure( 5 == y1 );
        ensure( 8 == x2 );
        ensure( 8 == y2 );

        GEOSCoordSeq_destroy(coords_);
    }
    
} // namespace tut

