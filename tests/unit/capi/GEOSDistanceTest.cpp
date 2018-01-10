//
// Test Suite for C-API GEOSDistance

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <math.h>
#ifndef M_PI
#define M_PI       3.14159265358979323846
#endif

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
            : geom1_(nullptr), geom2_(nullptr), geom3_(nullptr), w_(nullptr)
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
            geom1_ = nullptr;
            geom2_ = nullptr;
            geom3_ = nullptr;
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

    GEOSGeometry* random_polygon(double x, double y, double r, size_t num_points)
    {
        std::vector<double> angle(num_points);
        std::vector<double> radius(num_points);


        for (size_t i = 0; i < num_points; i++) {
            angle[i] = 2 * M_PI * std::rand() / RAND_MAX;
            radius[i] = r*std::rand() / RAND_MAX;
        }

        std::sort(angle.begin(), angle.end());

        GEOSCoordSequence* seq_1 = GEOSCoordSeq_create(static_cast<unsigned int>(num_points), 2);
        for (unsigned int i = 0; i < num_points; i++)
        {
            auto idx = i == (num_points - 1) ? 0 : i;

            GEOSCoordSeq_setX(seq_1, i, x + radius[idx] * cos(angle[idx]));
            GEOSCoordSeq_setY(seq_1, i, y + radius[idx] * sin(angle[idx]));
        }

        return GEOSGeom_createPolygon(GEOSGeom_createLinearRing(seq_1), nullptr, 0);
    }

    /* Generate two complex polygons and verify that GEOSDistance and GEOSDistanceIndexed
     * return identical results.
     */
    template<>
    template<>
    void object::test<2>()
    {
        std::srand(12345);

        GEOSGeometry* g1 = random_polygon(-3, -8, 7, 1000);
        GEOSGeometry* g2 = random_polygon(14, 22, 6, 500);

        double d_raw, d_indexed;
        ensure(GEOSDistance(g1, g2, &d_raw));
        ensure(GEOSDistanceIndexed(g1, g2, &d_indexed));

        ensure_equals(d_indexed, d_raw);

        GEOSGeom_destroy(g1);
        GEOSGeom_destroy(g2);
    }


} // namespace tut

