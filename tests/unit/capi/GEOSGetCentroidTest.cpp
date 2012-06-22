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
#include <cstring>

namespace tut
{
    //
    // Test Group
    //

    // Common data used in test cases.
    struct test_capicentroid_data
    {
        GEOSGeometry* geom1_;
        GEOSGeometry* geom2_;
        GEOSWKTWriter* wktw_;
        char* wkt_;
        double area_;

        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capicentroid_data()
            : geom1_(0), geom2_(0), wkt_(0)
        {
            initGEOS(notice, notice);
            wktw_ = GEOSWKTWriter_create();
            GEOSWKTWriter_setTrim(wktw_, 1);
            GEOSWKTWriter_setRoundingPrecision(wktw_, 8);
        }       

        ~test_capicentroid_data()
        {
            GEOSGeom_destroy(geom1_);
            GEOSGeom_destroy(geom2_);
            GEOSWKTWriter_destroy(wktw_);
            GEOSFree(wkt_);
            geom1_ = 0;
            geom2_ = 0;
            wkt_ = 0;
            finishGEOS();
        }

    };

    typedef test_group<test_capicentroid_data> group;
    typedef group::object object;

    group test_capicentroid_group("capi::GEOSGetCentroid");

    //
    // Test Cases
    //

    // Single point
    template<>
    template<>
    void object::test<1>()
    {
        geom1_ = GEOSGeomFromWKT("POINT(10 0)");

        ensure( 0 != geom1_ );

        geom2_ = GEOSGetCentroid(geom1_);

        ensure( 0 != geom2_ );

        wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

        ensure_equals(std::string(wkt_), std::string( "POINT (10 0)"));

    }

    // line
    template<>
    template<>
    void object::test<2>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)");

        ensure( 0 != geom1_ );

        geom2_ = GEOSGetCentroid(geom1_);

        ensure( 0 != geom2_ );

        wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

        ensure_equals(std::string(wkt_), std::string( "POINT (5 0)"));

    }

    // polygon
    template<>
    template<>
    void object::test<3>()
    {
        geom1_ = GEOSGeomFromWKT("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");

        ensure( 0 != geom1_ );

        geom2_ = GEOSGetCentroid(geom1_);

        ensure( 0 != geom2_ );

        wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

        ensure_equals(std::string(wkt_), std::string( "POINT (5 5)"));

    }

    // Tiny triangle, see http://trac.osgeo.org/geos/ticket/559
    template<>
    template<>
    void object::test<4>()
    {
        geom1_ = GEOSGeomFromWKT(
"POLYGON(( \
56.528666666700 25.2101666667, \
56.529000000000 25.2105000000, \
56.528833333300 25.2103333333, \
56.528666666700 25.2101666667))");

        ensure( 0 != geom1_ );

        geom2_ = GEOSGetCentroid(geom1_);

        ensure( 0 != geom2_ );

        wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

        ensure_equals(std::string(wkt_), std::string( "POINT (56.528833 25.210333)" ) );

    }

    // Empty geometry -- see http://trac.osgeo.org/geos/ticket/560
    template<>
    template<>
    void object::test<5>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING EMPTY");

        ensure( 0 != geom1_ );

        geom2_ = GEOSGetCentroid(geom1_);

        ensure( 0 != geom2_ );

        wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

        ensure_equals(std::string(wkt_), std::string( "POINT EMPTY"));

    }

} // namespace tut

