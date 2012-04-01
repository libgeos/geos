// $Id$
// 
// Test Suite for C-API GEOSisValidDetail

#include <tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cctype>
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
    struct test_capiisvaliddetail_data
    {
        GEOSWKTWriter* wktw_;
        GEOSGeometry* geom_;
        GEOSGeometry* loc_;
        char* reason_;

        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capiisvaliddetail_data()
            : geom_(0), loc_(0), reason_(0)
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

        void strToUpper(std::string &str)
        {
            using std::toupper;
            using std::string;
            
            for(string::size_type i = 0, len = str.size(); i < len; ++i)
                str[i] = static_cast<string::value_type>(toupper(str[i]));
        }

        ~test_capiisvaliddetail_data()
        {
            GEOSGeom_destroy(geom_);
            GEOSGeom_destroy(loc_);
            GEOSFree(reason_);
            GEOSWKTWriter_destroy(wktw_);
            finishGEOS();
        }

    };

    typedef test_group<test_capiisvaliddetail_data> group;
    typedef group::object object;

    group test_capiisvaliddetail_group("capi::GEOSisValidDetail");

    //
    // Test Cases
    //


    // Flag values
    template<>
    template<>
    void object::test<1>()
    {
      ensure_equals(GEOSVALID_ALLOW_SELFTOUCHING_RING_FORMING_HOLE, 1);
    }

    // Valid case
    template<>
    template<>
    void object::test<2>()
    {
      // Looks invalid (self-intersecting) but isn't
      // (is non-simple though)
      geom_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0, 5 -5, 5 5)");
      int r = GEOSisValidDetail(geom_, 0, &reason_, &loc_);
      ensure_equals(r, 1); // valid
      ensure_equals(reason_, (void*)0);
      ensure_equals(loc_, (void*)0);
    }

    // Invalid coordinate
    template<>
    template<>
    void object::test<3>()
    {
      geom_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0, NaN -5)");
      ensure(0 != geom_);
      int r = GEOSisValidDetail(geom_, 0, &reason_, &loc_);
      std::string wkt = toWKT(loc_);
      strToUpper(wkt);
      ensure_equals(r, 0); // invalid
      ensure_equals(std::string(reason_), std::string("Invalid Coordinate"));
      ensure_equals(wkt, "POINT (NAN -5)");
    }

    // Self intersecting ring forming hole
    template<>
    template<>
    void object::test<4>()
    {
      geom_ = GEOSGeomFromWKT("POLYGON((0 1, -10 10, 10 10, 0 1, 4 6, -4 6, 0 1))");
      int r = GEOSisValidDetail(geom_, 0, &reason_, &loc_);
      ensure_equals(r, 0); // invalid
      ensure_equals(std::string(reason_), std::string("Ring Self-intersection"));
      ensure_equals(toWKT(loc_), "POINT (0 1)");
    }

    // Self intersecting ring forming hole (with ESRI flag)
    template<>
    template<>
    void object::test<5>()
    {
      geom_ = GEOSGeomFromWKT("POLYGON((0 1, -10 10, 10 10, 0 1, 4 6, -4 6, 0 1))");
      int flags = GEOSVALID_ALLOW_SELFTOUCHING_RING_FORMING_HOLE;

      int r = GEOSisValidDetail(geom_, flags, &reason_, &loc_);
      ensure_equals(r, 1); // valid
      ensure_equals(reason_, (void*)0);
      ensure_equals(loc_, (void*)0);
    }

    // Check it is possible to not request details
    template<>
    template<>
    void object::test<6>()
    {
      geom_ = GEOSGeomFromWKT("POLYGON((0 1, -10 10, 10 10, 0 1, 4 6, -4 6, 0 1))");
      int r = GEOSisValidDetail(geom_, 0, 0, 0);
      ensure_equals(r, 0); // invalid
    }

} // namespace tut

