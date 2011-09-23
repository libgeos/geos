// $Id: GEOSGeomFromWKBTest.cpp 2809 2009-12-06 01:05:24Z mloskot $
// 
// Test Suite for C-API GEOSGeomFromWKB

#include <tut.hpp>
#include <utility.h> // wkb_hex_decoder
// geos
#include <geos_c.h>
// std
#include <string>
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
    struct test_capigeosgeomfromwkb_data
    {
        GEOSGeometry* geom1_;
        GEOSGeometry* geom2_;
        GEOSWKTReader* reader_;

        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capigeosgeomfromwkb_data()
            : geom1_(0), geom2_(0), reader_(0)
        {
            initGEOS(notice, notice);
            reader_ = GEOSWKTReader_create();
        }       

        ~test_capigeosgeomfromwkb_data()
        {
            GEOSGeom_destroy(geom2_);
            geom2_ = 0;
            GEOSGeom_destroy(geom1_);
            geom1_ = 0;
            GEOSWKTReader_destroy(reader_);
            reader_ = 0;
            finishGEOS();
        }

        void test_wkb(std::string const& wkbhex, std::string const& wkt)
        {
            wkb_hex_decoder::binary_type wkb;
            wkb_hex_decoder::decode(wkbhex, wkb);

            geom1_ = GEOSGeomFromWKB_buf(&wkb[0], wkb.size());
            ensure("GEOSGeomFromWKB_buf failed to create geometry", 0 != geom1_ );

            // TODO: Update test to compare with WKT-based geometry
            (void)wkt;
            //       ATM, some XYZ and XYZM geometries fail
            //geom2_ = GEOSWKTReader_read(reader_, wkt.c_str());
            //ensure ( 0 != geom2_ );
            //char result = GEOSEquals(geom1_, geom2_);
            //ensure_equals(result, char(1));
        }
    };

    typedef test_group<test_capigeosgeomfromwkb_data> group;
    typedef group::object object;

    group test_capigeosgeomfromwkb_group("capi::GEOSGeomFromWKB");

    //
    // Test Cases
    //

    template<>
    template<>
    void object::test<1>()
    {
        // POINT(1.234 5.678)
        std::string wkt("POINT (1.234 5.678)");
        std::string wkb("01010000005839B4C876BEF33F83C0CAA145B61640");
        test_wkb(wkb, wkt);
    }

    template<>
    template<>
    void object::test<2>()
    {
        // SRID=4;POINT(0 0)
        std::string wkt("POINT(0 0)");
        std::string ewkb("01010000200400000000000000000000000000000000000000");
        test_wkb(ewkb, wkt);
    }
    
    template<>
    template<>
    void object::test<3>()
    {
        // SRID=32632;POINT(1.234 5.678)
        std::string wkt("POINT (1.234 5.678)");
        std::string ewkb("0101000020787F00005839B4C876BEF33F83C0CAA145B61640");
        test_wkb(ewkb, wkt);
    }

    template<>
    template<>
    void object::test<4>()
    {
        // POINT (1.234 5.678 15 79) -- XYZM
        std::string wkt("POINT (1.234 5.678 15 79)");
        std::string ewkb("01010000C05839B4C876BEF33F83C0CAA145B616400000000000002E400000000000C05340");
        test_wkb(ewkb, wkt);
    }

    template<>
    template<>
    void object::test<5>()
    {
        std::string wkt("MULTIPOINT (1.123 1.456, 2.123 2.456, 3.123 3.456)");
        std::string ewkb("01040000000300000001010000002b8716d9cef7f13fb29defa7c64bf73f010100000096438b6ce7fb0040d9cef753e3a50340010100000096438b6ce7fb0840d9cef753e3a50b40");
        test_wkb(ewkb, wkt);
    }

    // TODO: Does GEOSGeomFromWKB_buf accept EWKB or WKB only?
    //       The cases below test EWKB input and they are failing.
    //template<>
    //template<>
    //void object::test<6>()
    //{
    //    // SELECT st_geomfromewkt('MULTIPOINT((0 0 1 1), (3 2 2 1))') ;
    //    std::string wkt("MULTIPOINT((0 0 1 1), (3 2 2 1))");
    //    std::string ewkb("01040000C00200000001010000C000000000000000000000000000000000000000000000F03F000000000000F03F01010000C0000000000000084000000000000000400000000000000040000000000000F03F");
    //    test_wkb(ewkb, wkt);
    //}

} // namespace tut

