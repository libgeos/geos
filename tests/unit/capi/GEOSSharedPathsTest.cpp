//
// Test Suite for C-API GEOSSharedPaths

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeossharedpaths_data {
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
    GEOSGeometry* geom3_;
    GEOSWKTWriter* w_;

    static void
    notice(const char* fmt, ...)
    {
        std::fprintf(stdout, "NOTICE: ");

        va_list ap;
        va_start(ap, fmt);
        std::vfprintf(stdout, fmt, ap);
        va_end(ap);

        std::fprintf(stdout, "\n");
    }

    test_capigeossharedpaths_data()
        : geom1_(nullptr), geom2_(nullptr), geom3_(nullptr), w_(nullptr)
    {
        initGEOS(notice, notice);
        w_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(w_, 1);
    }

    ~test_capigeossharedpaths_data()
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

typedef test_group<test_capigeossharedpaths_data> group;
typedef group::object object;

group test_capigeossharedpaths_group("capi::GEOSSharedPaths");

//
// Test Cases
//

/// Illegal case (point-poly)
template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");
    geom2_ = GEOSGeomFromWKT("POINT(0.5 0)");
    geom3_ = GEOSSharedPaths(geom1_, geom2_);

    ensure(!geom3_);
}

/// Line to line sharing
template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING (-30 -20, 50 60, 50 70, 50 0)");
    geom2_ = GEOSGeomFromWKT("LINESTRING (-29 -20, 50 60, 50 70, 51 0)");
    geom3_ = GEOSSharedPaths(geom1_, geom2_);

    char* wkt_c = GEOSWKTWriter_write(w_, geom3_);
    std::string out(wkt_c);
    free(wkt_c);

    ensure_equals(out,
                  "GEOMETRYCOLLECTION (MULTILINESTRING ((50 60, 50 70)), MULTILINESTRING EMPTY)"
                 );
}

/// http://trac.osgeo.org/postgis/ticket/670#comment:3
template<>
template<>
void object::test<3>
()
{
    // NOTE: in ticket #670 both geoms were in SRID=4326

    geom1_ = GEOSGeomFromWKT(
                 "POINT(-11.1111111 40)"
             );
    geom2_ = GEOSGeomFromWKT(
                 "POLYGON((-8.1111111 60,-8.16875525879031 59.4147290339516,-8.33947250246614 58.8519497029047,-8.61670226309236 58.3332893009412,-8.98979075644036 57.8786796564404,-9.44440040094119 57.5055911630924,-9.96306080290473 57.2283614024661,-10.5258401339516 57.0576441587903,-11.1111111 57,-11.6963820660484 57.0576441587903,-12.2591613970953 57.2283614024661,-12.7778217990588 57.5055911630924,-13.2324314435596 57.8786796564404,-13.6055199369076 58.3332893009412,-13.8827496975339 58.8519497029047,-14.0534669412097 59.4147290339516,-14.1111111 60,-14.0534669412097 60.5852709660484,-13.8827496975339 61.1480502970953,-13.6055199369076 61.6667106990588,-13.2324314435597 62.1213203435596,-12.7778217990588 62.4944088369076,-12.2591613970953 62.7716385975339,-11.6963820660484 62.9423558412097,-11.1111111 63,-10.5258401339516 62.9423558412097,-9.96306080290474 62.7716385975339,-9.4444004009412 62.4944088369076,-8.98979075644036 62.1213203435596,-8.61670226309237 61.6667106990588,-8.33947250246614 61.1480502970953,-8.16875525879031 60.5852709660484,-8.1111111 60))"
             );

    ensure(nullptr != geom1_);
    ensure(nullptr != geom2_);

    geom3_ = GEOSSharedPaths(geom1_, geom2_);

    ensure(!geom3_);
}

} // namespace tut

