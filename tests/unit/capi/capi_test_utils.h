
#pragma once

#include <tut/tut.hpp>

#include <geos_c.h>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cfenv>


namespace capitest {

    struct utility {
        GEOSWKTWriter* wktw_ = nullptr;
        GEOSGeometry* geom1_ = nullptr;
        GEOSGeometry* geom2_ = nullptr;
        GEOSGeometry* geom3_ = nullptr;
        GEOSGeometry* input_ = nullptr;
        GEOSGeometry* result_ = nullptr;
        GEOSGeometry* expected_ = nullptr;
        char* wkt_ = nullptr;

        utility()
        {
            initGEOS(notice, notice);
            wktw_ = GEOSWKTWriter_create();
            GEOSWKTWriter_setRoundingPrecision(wktw_, 10);

            std::feclearexcept(FE_ALL_EXCEPT);
        }

        ~utility()
        {
            if (wktw_)
                GEOSWKTWriter_destroy(wktw_);
            if (geom1_) {
                GEOSGeom_destroy(geom1_);
            }
            if (geom2_) {
                GEOSGeom_destroy(geom2_);
            }
            if (geom3_) {
                GEOSGeom_destroy(geom3_);
            }
            if (input_) {
                GEOSGeom_destroy(input_);
            }
            if (result_) {
                GEOSGeom_destroy(result_);
            }
            if (expected_) {
                GEOSGeom_destroy(expected_);
            }
            if (wkt_) {
                GEOSFree(wkt_);
            }
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

        GEOSGeometry*
        fromWKT(const char* wkt)
        {
            GEOSGeometry* g = GEOSGeomFromWKT(wkt);
            tut::ensure(g != nullptr);
            return g;
        }

        std::string
        toWKT(GEOSGeometry* g)
        {
            char* wkt = GEOSWKTWriter_write(wktw_, g);
            std::string ret(wkt);
            GEOSFree(wkt);
            return ret;
        }

        void
        ensure_geometry_equals(GEOSGeometry* g1, GEOSGeometry* g2, double tolerance)
        {
            GEOSNormalize(g1);
            GEOSNormalize(g2);
            char rslt = GEOSEqualsExact(g1, g2, tolerance);
            if (rslt != 1)
            {
                char* wkt1 = GEOSWKTWriter_write(wktw_, g1);
                char* wkt2 = GEOSGeomToWKT(g2);
                std::fprintf(stdout, "\n%s != %s\n", wkt1, wkt2);
                GEOSFree(wkt1);
                GEOSFree(wkt2);
            }
            tut::ensure_equals("GEOSEqualsExact(g1, g2, tolerance)", rslt, 1);
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
            tut::ensure(g2 != nullptr);
            GEOSNormalize(g1);
            GEOSNormalize(g2);
            char rslt = GEOSEqualsExact(g1, g2, 1e-12);
            if (rslt != 1)
            {
                char* wkt1 = GEOSWKTWriter_write(wktw_, g1);
                char* wkt2 = GEOSWKTWriter_write(wktw_, g2);
                std::fprintf(stdout, "\n%s != %s\n", wkt1, wkt2);
                GEOSFree(wkt1);
                GEOSFree(wkt2);
            }
            GEOSGeom_destroy(g2);
            tut::ensure_equals("GEOSEqualsExact(g1, g2, 1e-12)", rslt, 1);
        }



    };

}

