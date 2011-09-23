// $Id: GEOSPolygonizer_getCutEdgesTest.cpp 2424 2009-04-29 23:52:36Z mloskot $
// 
// Test Suite for C-API GEOSPolygonizeGetCutEdges

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
    struct test_capigeospolygonizegetcutedges_data
    {
        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capigeospolygonizegetcutedges_data()
        {
            initGEOS(notice, notice);
        }       

        ~test_capigeospolygonizegetcutedges_data()
        {
            finishGEOS();
        }

    };

    typedef test_group<test_capigeospolygonizegetcutedges_data> group;
    typedef group::object object;

    group test_capigeospolygonizegetcutedges_group("capi::GEOSPolygonizeGetCutEdges");

    //
    // Test Cases
    //

    template<>
    template<>
    void object::test<1>()
    {
        const int size = 2;
        GEOSGeometry* geoms[size] = { 0 };

        geoms[0] = GEOSGeomFromWKT("LINESTRING(1 3, 3 3, 3 1, 1 1, 1 3)");
        geoms[1] = GEOSGeomFromWKT("LINESTRING(1 3, 3 3, 3 1, 1 1, 1 3)");

        GEOSGeometry* g = GEOSPolygonizer_getCutEdges(geoms, size);

        ensure(0 != g);
        ensure_equals(GEOSGetNumGeometries(g), size);

        GEOSGeom_destroy(g);
        GEOSGeom_destroy(geoms[0]);
        GEOSGeom_destroy(geoms[1]);
    }

    template<>
    template<>
    void object::test<2>()
    {
        const int size = 6;
        GEOSGeometry* geoms[size] = { 0 };

        // Example from JTS Developer's Guide, Chapter 6 - Polygonization
        geoms[0] = GEOSGeomFromWKT("LINESTRING(0 0, 10 10)"); // isolated edge
        geoms[1] = GEOSGeomFromWKT("LINESTRING(185 221, 100 100)"); // dangling edge
        geoms[2] = GEOSGeomFromWKT("LINESTRING(185 221, 88 275, 180 316)");
        geoms[3] = GEOSGeomFromWKT("LINESTRING(185 221, 292 281, 180 316)");
        geoms[4] = GEOSGeomFromWKT("LINESTRING(189 98, 83 187, 185 221)");
        geoms[5] = GEOSGeomFromWKT("LINESTRING(189 98, 325 168, 185 221)");

        GEOSGeometry* g = GEOSPolygonizer_getCutEdges(geoms, size);

        ensure(0 != g);
        ensure_equals(GEOSGetNumGeometries(g), 0);

        GEOSGeom_destroy(g);

        for (int i = 0; i < size; ++i)
        {
            if (0 != geoms[i])
                GEOSGeom_destroy(geoms[i]);
        }
    }
    
} // namespace tut

