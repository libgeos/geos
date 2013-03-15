// 
// Test Suite for C-API GEOSUnaryUnion

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
    struct test_capiunaryunion_data
    {
        GEOSWKTWriter* wktw_;
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

        test_capiunaryunion_data()
            : geom1_(0), geom2_(0)
        {
            initGEOS(notice, notice);
            wktw_ = GEOSWKTWriter_create();
            GEOSWKTWriter_setTrim(wktw_, 1);
            GEOSWKTWriter_setOutputDimension(wktw_, 3);
        }       

        std::string toWKT(GEOSGeometry* g)
        {
          char* wkt = GEOSWKTWriter_write(wktw_, g);
          std::string ret (wkt);
          GEOSFree(wkt);
          return ret;
        }

        ~test_capiunaryunion_data()
        {
            GEOSGeom_destroy(geom1_);
            GEOSGeom_destroy(geom2_);
            GEOSWKTWriter_destroy(wktw_);
            geom1_ = 0;
            geom2_ = 0;
            finishGEOS();
        }

    };

    typedef test_group<test_capiunaryunion_data> group;
    typedef group::object object;

    group test_capiunaryunion_group("capi::GEOSUnaryUnion");

    //
    // Test Cases
    //


    // Self-union an empty point
    template<>
    template<>
    void object::test<1>()
    {
        geom1_ = GEOSGeomFromWKT("POINT EMPTY");
        ensure( 0 != geom1_ );

        geom2_ = GEOSUnaryUnion(geom1_);
        ensure( 0 != geom2_ );

        ensure_equals(toWKT(geom2_), std::string("GEOMETRYCOLLECTION EMPTY"));
    }

    // Self-union a 2d point
    template<>
    template<>
    void object::test<2>()
    {
        geom1_ = GEOSGeomFromWKT("POINT (6 3)");
        ensure( 0 != geom1_ );

        geom2_ = GEOSUnaryUnion(geom1_);
        ensure( 0 != geom2_ );

        ensure_equals(toWKT(geom2_), std::string("POINT (6 3)"));
    }

    // Self-union a 3d point
    template<>
    template<>
    void object::test<3>()
    {
        geom1_ = GEOSGeomFromWKT("POINT (4 5 6)");
        ensure( 0 != geom1_ );

        geom2_ = GEOSUnaryUnion(geom1_);
        ensure( 0 != geom2_ );

        ensure_equals(toWKT(geom2_), std::string("POINT Z (4 5 6)"));
    }

    // Self-union a multipoint with duplicated points
    template<>
    template<>
    void object::test<4>()
    {
        geom1_ = GEOSGeomFromWKT("MULTIPOINT (4 5, 6 7, 4 5, 6 5, 6 7)");
        ensure( 0 != geom1_ );

        geom2_ = GEOSUnaryUnion(geom1_);
        ensure( 0 != geom2_ );

        ensure_equals(toWKT(geom2_), std::string("MULTIPOINT (4 5, 6 5, 6 7)"));
    }

    // Self-union a collection of puntal and lineal geometries
    template<>
    template<>
    void object::test<5>()
    {
        geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION (POINT(4 5), MULTIPOINT(6 7, 6 5, 6 7), LINESTRING(0 5, 10 5), LINESTRING(4 -10, 4 10))");
        ensure( 0 != geom1_ );

        geom2_ = GEOSUnaryUnion(geom1_);
        ensure( 0 != geom2_ );

        ensure_equals(toWKT(geom2_), std::string("GEOMETRYCOLLECTION (POINT (6 7), LINESTRING (4 -10, 4 5), LINESTRING (4 5, 4 10), LINESTRING (0 5, 4 5), LINESTRING (4 5, 10 5))"));
    }

    // Self-union a collection of puntal and polygonal geometries
    template<>
    template<>
    void object::test<6>()
    {
        geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION (POINT(4 5), MULTIPOINT(6 7, 6 5, 6 7), POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(5 6, 7 6, 7 8, 5 8, 5 6)))");
        ensure( 0 != geom1_ );

        geom2_ = GEOSUnaryUnion(geom1_);
        ensure( 0 != geom2_ );

        ensure_equals(toWKT(geom2_), std::string(
"GEOMETRYCOLLECTION (POINT (6 7), POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (5 6, 7 6, 7 8, 5 8, 5 6)))"
));
    }

    // Self-union a collection of lineal and polygonal geometries
    template<>
    template<>
    void object::test<7>()
    {
        geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION (MULTILINESTRING((5 7, 12 7), (4 5, 6 5), (5.5 7.5, 6.5 7.5)), POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(5 6, 7 6, 7 8, 5 8, 5 6)))");
        ensure( 0 != geom1_ );

        geom2_ = GEOSUnaryUnion(geom1_);
        ensure( 0 != geom2_ );

        ensure_equals(toWKT(geom2_), std::string(
"GEOMETRYCOLLECTION (LINESTRING (5 7, 7 7), LINESTRING (10 7, 12 7), LINESTRING (5.5 7.5, 6.5 7.5), POLYGON ((10 7, 10 0, 0 0, 0 10, 10 10, 10 7), (5 6, 7 6, 7 7, 7 8, 5 8, 5 7, 5 6)))"
));
    }

    // Self-union a collection of puntal, lineal and polygonal geometries
    template<>
    template<>
    void object::test<8>()
    {
        geom1_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION (MULTILINESTRING((5 7, 12 7), (4 5, 6 5), (5.5 7.5, 6.5 7.5)), POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(5 6, 7 6, 7 8, 5 8, 5 6)), MULTIPOINT(6 6.5, 6 1, 12 2, 6 1))");
        ensure( 0 != geom1_ );

        geom2_ = GEOSUnaryUnion(geom1_);
        ensure( 0 != geom2_ );

        ensure_equals(toWKT(geom2_), std::string(
"GEOMETRYCOLLECTION (POINT (6 6.5), POINT (12 2), LINESTRING (5 7, 7 7), LINESTRING (10 7, 12 7), LINESTRING (5.5 7.5, 6.5 7.5), POLYGON ((10 7, 10 0, 0 0, 0 10, 10 10, 10 7), (5 6, 7 6, 7 7, 7 8, 5 8, 5 7, 5 6)))"
));
    }

    // Self-union a geometry with NaN coordinates
    // See http://trac.osgeo.org/geos/ticket/530
    template<>
    template<>
    void object::test<9>()
    {
        const char* hexwkb = "010200000002000000000000000000F8FF000000000000F8FF0000000000000000000000000000F03F";
        geom1_ = GEOSGeomFromHEX_buf((const unsigned char*)hexwkb, std::strlen(hexwkb));
        ensure( 0 != geom1_ );

        geom2_ = GEOSUnaryUnion(geom1_);
        ensure( 0 == geom2_ );

    }

} // namespace tut

