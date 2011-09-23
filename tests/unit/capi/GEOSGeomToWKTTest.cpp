// $Id: GEOSGeomToWKTTest.cpp 3075 2010-06-24 14:40:01Z warmerdam $
// 
// Test Suite for C-API GEOSGeomToWKT

#include <tut.hpp>
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
    struct test_capigeosgeomtowkt_data
    {
        GEOSGeometry* geom1_;

        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capigeosgeomtowkt_data()
            : geom1_(0)
        {
            initGEOS(notice, notice);
        }       

        ~test_capigeosgeomtowkt_data()
        {
            GEOSGeom_destroy(geom1_);
            geom1_ = 0;
            finishGEOS();
        }

        void test_wkt(std::string const& wkt)
        {
            geom1_ = GEOSGeomFromWKT(wkt.c_str());
            ensure ( 0 != geom1_ );

            char* wkt_c = GEOSGeomToWKT(geom1_);
            std::string out(wkt_c); 
        free(wkt_c);

            ensure_equals(out, wkt);
        }
        
        void test_wkt(std::string const& wkt, std::string::size_type n)
        {
            geom1_ = GEOSGeomFromWKT(wkt.c_str());
            ensure ( 0 != geom1_ );

            char* wkt_c = GEOSGeomToWKT(geom1_);
            std::string out(wkt_c); 
        free(wkt_c);

            ensure_equals(out.substr(0, n), wkt.substr(0, n));
        }

        void test_writer_wkt(GEOSWKTWriter *writer, std::string const& wkt)
        {
            geom1_ = GEOSGeomFromWKT(wkt.c_str());
            ensure ( 0 != geom1_ );

            char* wkt_c = GEOSWKTWriter_write(writer,geom1_);
            std::string out(wkt_c); 
            free(wkt_c);

            ensure_equals(out, wkt);
        }
    };

    typedef test_group<test_capigeosgeomtowkt_data> group;
    typedef group::object object;

    group test_capigeosgeomtowkt_group("capi::GEOSGeomToWKT");

    //
    // Test Cases
    //

    template<>
    template<>
    void object::test<1>()
    {
        test_wkt("POINT EMPTY");
    }
    
    template<>
    template<>
    void object::test<2>()
    {
        test_wkt("LINESTRING EMPTY");
    }
    
    template<>
    template<>
    void object::test<3>()
    {
        test_wkt("POLYGON EMPTY");
    }
    
    template<>
    template<>
    void object::test<4>()
    {
        test_wkt("MULTIPOINT EMPTY");
    }
    
    template<>
    template<>
    void object::test<5>()
    {
        test_wkt("MULTILINESTRING EMPTY");
    }

    template<>
    template<>
    void object::test<6>()
    {
        test_wkt("MULTIPOLYGON EMPTY");
    }

    // Comparing string based on float-point numbers does not make sense,
    // so make poor-man comparison of WKT type tag.

    template<>
    template<>
    void object::test<7>()
    {
        test_wkt("POINT (1.234 5.678)", 7);
    }

    template<>
    template<>
    void object::test<8>()
    {
        test_wkt("LINESTRING (0 0, 5 5, 10 5, 10 10)", 13);
    }
    
    template<>
    template<>
    void object::test<9>()
    {
        test_wkt("POLYGON ((0 10, 5 5, 10 5, 15 10, 10 15, 5 15, 0 10))", 11);
    }
    
    template<>
    template<>
    void object::test<10>()
    {
        test_wkt("MULTIPOINT (0 0, 5 5, 10 10, 15 15, 20 20)", 13);
    }
    
    template<>
    template<>
    void object::test<11>()
    {
        test_wkt("MULTILINESTRING ((0 0, 10 0, 10 10, 0 10, 10 20),(2 2, 2 6, 6 4, 20 2))", 19);
    }

    template<>
    template<>
    void object::test<12>()
    {
        test_wkt("MULTIPOLYGON (((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 2 6, 6 4, 2 2)),((60 60, 60 50, 70 40, 60 60)))", 17);
    }

    // Test the WKTWriter API instead of the quicky function.

    template<>
    template<>
    void object::test<13>()
    {
        GEOSWKTWriter *writer = GEOSWKTWriter_create();

        ensure( "getOutputDimension_1", 
                GEOSWKTWriter_getOutputDimension(writer) == 2 );

        GEOSWKTWriter_setTrim( writer, 1 );
        GEOSWKTWriter_setOutputDimension( writer, 3 );
        ensure( "getOutputDimension_2", 
                GEOSWKTWriter_getOutputDimension(writer) == 3 );

        test_writer_wkt(writer, "POINT Z (10 13 3)");
        GEOSWKTWriter_destroy( writer );
    }

} // namespace tut

