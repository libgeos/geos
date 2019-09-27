//
// Test Suite for C-API GEOSGeom_setPrecision_r

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosgeomsetprecision_data {
    GEOSWKTWriter* wktw_;
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
    GEOSGeometry* geom3_;

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

    GEOSGeometry*
    fromWKT(const char* wkt)
    {
        GEOSGeometry* g = GEOSGeomFromWKT(wkt);
        ensure(g != 0);
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

    test_capigeosgeomsetprecision_data()
        : geom1_(nullptr), geom2_(nullptr), geom3_(nullptr)
    {
        initGEOS(notice, notice);
        wktw_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setRoundingPrecision(wktw_, 10);
        //GEOSWKTWriter_setOutputDimension(wktw_, 3);
    }

    ~test_capigeosgeomsetprecision_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        if(geom3_) {
            GEOSGeom_destroy(geom3_);
        }
        GEOSWKTWriter_destroy(wktw_);

        geom1_ = nullptr;
        geom2_ = nullptr;
        geom3_ = nullptr;
        finishGEOS();
    }

};

typedef test_group<test_capigeosgeomsetprecision_data> group;
typedef group::object object;

group test_capigeosgeomsetprecision_group("capi::GEOSGeom_setPrecision");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = fromWKT("POLYGON EMPTY");
    ensure(geom1_ != 0);
    double scale = GEOSGeom_getPrecision(geom1_);
    ensure_equals(scale, 0.0);
    geom3_ = GEOSGeom_setPrecision(geom1_, 2.0, 0);
    ensure(geom3_ != 0);
    ensure_equals(toWKT(geom3_), std::string("POLYGON EMPTY"));
    scale = GEOSGeom_getPrecision(geom3_);
    ensure_equals(scale, 2.0);
}

template<>
template<>
void object::test<2>
()
{
    geom1_ = fromWKT("LINESTRING(-3 6, 9 1)");
    ensure(geom1_ != 0);
    geom3_ = GEOSGeom_setPrecision(geom1_, 2.0, 0);
    ensure(geom3_ != 0);
    ensure_equals(toWKT(geom3_), std::string("LINESTRING (-2 6, 10 2)"));
}

// See effects of precision reduction on intersection operation
template<>
template<>
void object::test<3>
()
{
    geom1_ = fromWKT("LINESTRING(2 10, 4 30)");
    ensure(geom1_ != 0);
    geom2_ = fromWKT("LINESTRING(4 10, 2 30)");
    ensure(geom2_ != 0);
    geom3_ = GEOSIntersection(geom1_, geom2_);
    ensure(geom3_ != 0);
    ensure_equals(toWKT(geom3_), std::string("POINT (3 20)"));

    GEOSGeometry* g;

    // Both inputs with precision grid of 2.0
    g = GEOSGeom_setPrecision(geom1_, 2.0, 0);
    ensure(g != 0);
    GEOSGeom_destroy(geom1_);
    geom1_ = g;
    g = GEOSGeom_setPrecision(geom2_, 2.0, 0);
    ensure(g != 0);
    GEOSGeom_destroy(geom2_);
    geom2_ = g;
    GEOSGeom_destroy(geom3_);
    geom3_ = GEOSIntersection(geom1_, geom2_);
    ensure(geom3_ != 0);
    ensure_equals(toWKT(geom3_), std::string("POINT (4 20)"));

    // One input with precision grid of 0.5, the other of 2.0
    g = GEOSGeom_setPrecision(geom1_, 0.5, 0);
    ensure(g != 0);
    GEOSGeom_destroy(geom1_);
    geom1_ = g;
    GEOSGeom_destroy(geom3_);
    geom3_ = GEOSIntersection(geom1_, geom2_);
    ensure(geom3_ != 0);
    ensure_equals(toWKT(geom3_), std::string("POINT (3 20)"));
    double scale = GEOSGeom_getPrecision(geom1_);
    ensure_equals(scale, 0.5);
    scale = GEOSGeom_getPrecision(geom2_);
    ensure_equals(scale, 2.0);
}

// Retain (or not) topology
template<>
template<>
void object::test<4>
()
{
    geom1_ = fromWKT("POLYGON((10 10,20 10,16 15,20 20, 10 20, 14 15, 10 10))");
    ensure(geom1_ != 0);
    geom2_ = GEOSGeom_setPrecision(geom1_, 5.0, 0);
    ensure_equals(toWKT(geom2_), std::string(
                      "MULTIPOLYGON (((10 10, 15 15, 20 10, 10 10)), ((15 15, 10 20, 20 20, 15 15)))"
                  ));
    geom3_ = GEOSGeom_setPrecision(geom1_, 5.0, GEOS_PREC_NO_TOPO);
    ensure_equals(toWKT(geom3_), std::string(
                      "POLYGON ((10 10, 20 10, 15 15, 20 20, 10 20, 15 15, 10 10))"
                  ));
}

// Retain (or not) collapsed elements
template<>
template<>
void object::test<5>
()
{
    geom1_ = fromWKT("LINESTRING(1 0, 2 0)");
    ensure(geom1_ != 0);
    geom2_ = GEOSGeom_setPrecision(geom1_, 5.0, 0);
    ensure_equals(toWKT(geom2_), std::string(
                      "LINESTRING EMPTY"
                  ));
    geom3_ = GEOSGeom_setPrecision(geom1_, 5.0, GEOS_PREC_KEEP_COLLAPSED);
    ensure_equals(toWKT(geom3_), std::string(
                      "LINESTRING (0 0, 0 0)"
                  ));
}

} // namespace tut

