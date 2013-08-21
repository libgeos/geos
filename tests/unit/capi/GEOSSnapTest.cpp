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
    struct test_capigeossnap_data
    {
        GEOSGeometry* geom1_;
        GEOSGeometry* geom2_;
        GEOSGeometry* geom3_;
        GEOSWKTWriter* w_;

        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capigeossnap_data()
            : geom1_(0), geom2_(0), geom3_(0), w_(0)
        {
            initGEOS(notice, notice);
            w_ = GEOSWKTWriter_create();
            GEOSWKTWriter_setTrim(w_, 1);
            GEOSWKTWriter_setRoundingPrecision(w_, 8);
        }       

        ~test_capigeossnap_data()
        {
            GEOSGeom_destroy(geom1_);
            GEOSGeom_destroy(geom2_);
            GEOSGeom_destroy(geom3_);
            GEOSWKTWriter_destroy(w_);
            geom1_ = 0;
            geom2_ = 0;
            geom3_ = 0;
            finishGEOS();
        }

    };

    typedef test_group<test_capigeossnap_data> group;
    typedef group::object object;

    group test_capigeossnap_group("capi::GEOSSnap");

    //
    // Test Cases
    //

    /// Polygon snapped to point
    template<>
    template<>
    void object::test<1>()
    {
        geom1_ = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");
        geom2_ = GEOSGeomFromWKT("POINT(0.5 0)");
        geom3_ = GEOSSnap(geom1_, geom2_, 1);

        char* wkt_c = GEOSWKTWriter_write(w_, geom3_);
        std::string out(wkt_c); 
        free(wkt_c);

        ensure_equals(out, "POLYGON ((0.5 0, 10 0, 10 10, 0 10, 0.5 0))");
    }
    
    /// Line snapped to line (vertex)
    template<>
    template<>
    void object::test<2>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING (-30 -20, 50 60, 50 0)");
        geom2_ = GEOSGeomFromWKT("LINESTRING (-29 -20, 40 60, 51 0)");
        geom3_ = GEOSSnap(geom1_, geom2_, 2);

        char* wkt_c = GEOSWKTWriter_write(w_, geom3_);
        std::string out(wkt_c); 
        free(wkt_c);

        ensure_equals(out, "LINESTRING (-29 -20, 50 60, 51 0)");
    }

    /// Line snapped to line (segment)
    template<>
    template<>
    void object::test<3>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING (-20 -20, 50 50, 100 100)");
        geom2_ = GEOSGeomFromWKT("LINESTRING (-10 -9, 40 20, 80 79)");
        geom3_ = GEOSSnap(geom1_, geom2_, 2);

        char* wkt_c = GEOSWKTWriter_write(w_, geom3_);
        std::string out(wkt_c); 
        free(wkt_c);

        ensure_equals(out,
          "LINESTRING (-20 -20, -10 -9, 50 50, 80 79, 100 100)"
        );
    }

    /// Another single segment
    template<>
    template<>
    void object::test<4>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)");
        geom2_ = GEOSGeomFromWKT("LINESTRING(0 0, 9 0)");
        geom3_ = GEOSSnap(geom1_, geom2_, 2);

        char* wkt_c = GEOSWKTWriter_write(w_, geom3_);
        std::string out(wkt_c); 
        free(wkt_c);

        ensure_equals(out, "LINESTRING (0 0, 9 0)");
    }

    /// See http://trac.osgeo.org/geos/ticket/501
    template<>
    template<>
    void object::test<5>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0)");
        geom2_ = GEOSGeomFromWKT("LINESTRING(0 0, 9 0, 10 0, 11 0)");
        geom3_ = GEOSSnap(geom1_, geom2_, 2);

        char* wkt_c = GEOSWKTWriter_write(w_, geom3_);
        std::string out(wkt_c); 
        free(wkt_c);

        //ensure_equals(out, "LINESTRING (0 0, 9 0, 10 0)");
        ensure_equals(out, "LINESTRING (0 0, 9 0, 10 0, 11 0)");
    }

    /// Test snapping of equidistant segments to outlyers snap point
    template<>
    template<>
    void object::test<6>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 3,4 1,0 1)");
        geom2_ = GEOSGeomFromWKT("MULTIPOINT(5 0,4 1)");
        geom3_ = GEOSSnap(geom1_, geom2_, 2);

        char* wkt_c = GEOSWKTWriter_write(w_, geom3_);
        std::string out(wkt_c); 
        free(wkt_c);

        ensure_equals(out, "LINESTRING (0 3, 4 1, 5 0, 0 1)");
        //ensure_equals(out, "LINESTRING (0 3, 4 1, 0 1)");
    }

    /// Test snapping of equidistant segments to outlyers snap point
    /// Same as the above but with the snap points order reversed
    template<>
    template<>
    void object::test<7>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 3,4 1,0 1)");
        geom2_ = GEOSGeomFromWKT("MULTIPOINT(4 1,5 0)");
        geom3_ = GEOSSnap(geom1_, geom2_, 2);

        char* wkt_c = GEOSWKTWriter_write(w_, geom3_);
        std::string out(wkt_c); 
        free(wkt_c);

        ensure_equals(out, "LINESTRING (0 3, 4 1, 5 0, 0 1)");
        //ensure_equals(out, "LINESTRING (0 3, 4 1, 0 1)");
    }

    /// Test snapping of closed ring to outlyers snap point
    template<>
    template<>
    void object::test<8>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 0,10 0,10 10,0 10,0 0)");
        geom2_ = GEOSGeomFromWKT("MULTIPOINT(0 0,-1 0)");
        geom3_ = GEOSSnap(geom1_, geom2_, 3);

        char* wkt_c = GEOSWKTWriter_write(w_, geom3_);
        std::string out(wkt_c); 
        free(wkt_c);

        ensure_equals(out, "LINESTRING (-1 0, 0 0, 10 0, 10 10, 0 10, -1 0)");
    }

    template<>
    template<>
    void object::test<9>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 2,5 2,9 2,5 0)");
        geom2_ = GEOSGeomFromWKT("POINT(5 0)");
        geom3_ = GEOSSnap(geom1_, geom2_, 3);

        char* wkt_c = GEOSWKTWriter_write(w_, geom3_);
        std::string out(wkt_c); 
        free(wkt_c);

        ensure_equals(out, "LINESTRING (0 2, 5 2, 9 2, 5 0)");
    }

    // See http://trac.osgeo.org/geos/ticket/649
    template<>
    template<>
    void object::test<10>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(-71.1317 42.2511,-71.1317 42.2509)");
        geom2_ = GEOSGeomFromWKT("MULTIPOINT(-71.1261 42.2703,-71.1257 42.2703,-71.1261 42.2702)");
        geom3_ = GEOSSnap(geom1_, geom2_, 0.5);

        char* wkt_c = GEOSWKTWriter_write(w_, geom3_);
        std::string out(wkt_c); 
        free(wkt_c);

        ensure_equals(out, "LINESTRING (-71.1257 42.2703, -71.1261 42.2703, -71.1261 42.2702, -71.1317 42.2509)");
    }

} // namespace tut

