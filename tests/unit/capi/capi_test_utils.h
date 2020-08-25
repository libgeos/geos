
#pragma once

#include <geos_c.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>


namespace capitest {

    struct test_handlers {
        static void notice(const char* fmt, ...)
        {
            std::fprintf(stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);

            std::fprintf(stdout, "\n");
        }

        bool
        capi_geometry_same(GEOSGeometry* g1, GEOSGeometry* g2, double tolerance)
        {
            GEOSNormalize(g1);
            GEOSNormalize(g2);
            return GEOSEqualsExact(g1, g2, tolerance);
        }

        bool
        capi_geometry_same(GEOSGeometry* g1, GEOSGeometry* g2)
        {
            return capi_geometry_same(g1, g2, 1e-12);
        }

        bool
        capi_geometry_same(GEOSGeometry* g1, const char* g2str)
        {
            GEOSGeometry* g2 = GEOSGeomFromWKT(g2str);
            tut::ensure(g2 != 0);
            bool rslt = capi_geometry_same(g1, g2);
            GEOSGeom_destroy(g2);
            return rslt;
        }

        test_handlers()
        {
            initGEOS(notice, notice);
        }

        ~test_handlers()
        {
            finishGEOS();
        }
    };

}

