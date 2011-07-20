// 
// Test Suite for C-API GEOSDistance

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
    struct test_capigeosdistance_data
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

        test_capigeosdistance_data()
            : geom1_(0), geom2_(0), geom3_(0), w_(0)
        {
            initGEOS(notice, notice);
            w_ = GEOSWKTWriter_create();
            GEOSWKTWriter_setTrim(w_, 1);
        }       

        ~test_capigeosdistance_data()
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

    typedef test_group<test_capigeosdistance_data> group;
    typedef group::object object;

    group test_capigeosdistance_group("capi::GEOSDistance");

    //
    // Test Cases
    //

    /// See http://trac.osgeo.org/geos/ticket/377
    template<>
    template<>
    void object::test<1>()
    {
        geom1_ = GEOSGeomFromWKT("POINT(10 10)");
        geom2_ = GEOSGeomFromWKT("POINT(3 6)");

        double dist;
        int ret = GEOSDistance(geom1_, geom2_, &dist);

        ensure_equals(ret, 1);
        ensure_distance(dist, 8.06225774829855, 1e-12);
    }
    

} // namespace tut

