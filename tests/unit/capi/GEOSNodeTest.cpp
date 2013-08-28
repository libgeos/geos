// 
// Test Suite for C-API GEOSNode

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
    struct test_capigeosnode_data
    {
        GEOSGeometry* geom1_;
        GEOSGeometry* geom2_;
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

        test_capigeosnode_data()
            : geom1_(0), geom2_(0), w_(0)
        {
            initGEOS(notice, notice);
            w_ = GEOSWKTWriter_create();
            GEOSWKTWriter_setTrim(w_, 1);
        }       

        ~test_capigeosnode_data()
        {
            GEOSGeom_destroy(geom1_);
            GEOSGeom_destroy(geom2_);
            GEOSWKTWriter_destroy(w_);
            geom1_ = 0;
            geom2_ = 0;
            finishGEOS();
        }

    };

    typedef test_group<test_capigeosnode_data> group;
    typedef group::object object;

    group test_capigeosnode_group("capi::GEOSNode");

    //
    // Test Cases
    //

    /// Self-intersecting line
    template<>
    template<>
    void object::test<1>()
    {
        geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 10, 10 0, 0 10)");
        geom2_ = GEOSNode(geom1_);
        ensure(0 != geom2_);

        GEOSNormalize(geom2_);
        char* wkt_c = GEOSWKTWriter_write(w_, geom2_);
        std::string out(wkt_c); 
        free(wkt_c);

        ensure_equals(out,
          "MULTILINESTRING ((5 5, 10 0, 10 10, 5 5), (0 10, 5 5), (0 0, 5 5))"
        );
    }

    /// Overlapping lines
    template<>
    template<>
    void object::test<2>()
    {
        geom1_ = GEOSGeomFromWKT("MULTILINESTRING((0 0, 2 0, 4 0),(5 0, 3 0, 1 0))");
        geom2_ = GEOSNode(geom1_);
        ensure(0 != geom2_);

        GEOSNormalize(geom2_);
        char* wkt_c = GEOSWKTWriter_write(w_, geom2_);
        std::string out(wkt_c); 
        free(wkt_c);

        ensure_equals(out,
          "MULTILINESTRING ((4 0, 5 0), (3 0, 4 0), (2 0, 3 0), (1 0, 2 0), (0 0, 1 0))"
        );
    }

    /// Equal lines
    template<>
    template<>
    void object::test<3>()
    {
        geom1_ = GEOSGeomFromWKT("MULTILINESTRING((0 0, 2 0, 4 0),(0 0, 2 0, 4 0))");
        geom2_ = GEOSNode(geom1_);
        ensure(0 != geom2_);

        GEOSNormalize(geom2_);
        char* wkt_c = GEOSWKTWriter_write(w_, geom2_);
        std::string out(wkt_c); 
        free(wkt_c);

        ensure_equals(out,
          "MULTILINESTRING ((2 0, 4 0), (0 0, 2 0))"
        );
    }
    

} // namespace tut

