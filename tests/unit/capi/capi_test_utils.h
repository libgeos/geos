
#pragma once

#include <tut/tut.hpp>

#include <geos_c.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>


namespace capitest {

    struct utility {

        utility()
        {
            initGEOS(notice, notice);
        }

        ~utility()
        {
            finishGEOS();
        }

        static void notice(const char* fmt, ...)
        {
            std::fprintf(stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);

            std::fprintf(stdout, "\n");
        }

        void
        ensure_geometry_equals(GEOSGeometry* g1, GEOSGeometry* g2, double tolerance)
        {
            GEOSNormalize(g1);
            GEOSNormalize(g2);
            bool rslt = GEOSEqualsExact(g1, g2, tolerance);
            if (!rslt)
            {
                char* wkt1 = GEOSGeomToWKT(g1);
                char* wkt2 = GEOSGeomToWKT(g2);
                std::fprintf(stdout, "\n%s != %s\n", wkt1, wkt2);
                GEOSFree(wkt1);
                GEOSFree(wkt2);
            }
            tut::ensure_equals("GEOSEqualsExact(g1, g2, tolerance)", rslt, 1);
            return;
        }

        void
        ensure_geometry_equals(GEOSGeometry* g1, GEOSGeometry* g2)
        {
            return ensure_geometry_equals(g1, g2, 1e-12);
        }

        void
        ensure_geometry_equals(GEOSGeometry* g1, const char* g2str)
        {
            GEOSGeometry* g2 = GEOSGeomFromWKT(g2str);
            tut::ensure(g2 != 0);
            GEOSNormalize(g1);
            GEOSNormalize(g2);
            bool rslt = GEOSEqualsExact(g1, g2, 1e-12);
            if (!rslt)
            {
                char* wkt1 = GEOSGeomToWKT(g1);
                char* wkt2 = GEOSGeomToWKT(g2);
                std::fprintf(stdout, "\n%s != %s\n", wkt1, wkt2);
                GEOSFree(wkt1);
                GEOSFree(wkt2);
            }
            GEOSGeom_destroy(g2);
            tut::ensure_equals("GEOSEqualsExact(g1, g2, 1e-12)", rslt, 1);
            return;
        }

    };

}

