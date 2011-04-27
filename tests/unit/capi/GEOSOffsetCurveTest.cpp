// $Id$
// 
// Test Suite for C-API GEOSOffsetCurve

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
    struct test_capioffsetcurve_data
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

        test_capioffsetcurve_data()
            : geom1_(0), geom2_(0), wkt_(0)
        {
            initGEOS(notice, notice);
            wktw_ = GEOSWKTWriter_create();
            GEOSWKTWriter_setTrim(wktw_, 1);
        }       

        ~test_capioffsetcurve_data()
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

    typedef test_group<test_capioffsetcurve_data> group;
    typedef group::object object;

    group test_capioffsetcurve_group("capi::GEOSOffsetCurve");

    //
    // Test Cases
    //

    // Straight, left
    template<>
    template<>
    void object::test<1>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)");

        ensure( 0 != geom1_ );

        geom2_ = GEOSOffsetCurve(geom1_, 2, 2, GEOSBUF_JOIN_ROUND, 2, 1);

        ensure( 0 != geom2_ );

        wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

        ensure_equals(std::string(wkt_), std::string( "LINESTRING (0 2, 10 2)"));

    }

    // Straight, right
    template<>
    template<>
    void object::test<2>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)");

        ensure( 0 != geom1_ );

        geom2_ = GEOSOffsetCurve(geom1_, 2, 2, GEOSBUF_JOIN_ROUND, 2, 0);

        ensure( 0 != geom2_ );

        wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

        ensure_equals(std::string(wkt_), std::string(
"LINESTRING (10 -2, 0 -2)"
        ));
    }

    // Outside curve
    template<>
    template<>
    void object::test<3>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0, 10 10)");

        ensure( 0 != geom1_ );

        geom2_ = GEOSOffsetCurve(geom1_, 2, 1, GEOSBUF_JOIN_ROUND, 2, 0);

        ensure( 0 != geom2_ );

        wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

        ensure_equals(std::string(wkt_), std::string(
"LINESTRING (12 10, 12 0, 10 -2, 0 -2)"
        ));
    }

    // Inside curve
    template<>
    template<>
    void object::test<4>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0, 10 10)");

        ensure( 0 != geom1_ );

        geom2_ = GEOSOffsetCurve(geom1_, 2, 1, GEOSBUF_JOIN_ROUND, 2, 1);

        ensure( 0 != geom2_ );

        wkt_ = GEOSWKTWriter_write(wktw_, geom2_);

        ensure_equals(std::string(wkt_), std::string(
"LINESTRING (0 2, 8 2, 8 10)"
        ));
    }


} // namespace tut

