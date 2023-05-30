//
// Test Suite for C-API GEOSClipByRect

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
struct test_capigeosclipbyrect_data {
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

    void
    isEqual(GEOSGeom g, const char* exp_wkt)
    {
        geom3_ = GEOSGeomFromWKT(exp_wkt);
        bool eq = GEOSEquals(geom3_, g) != 0;
        if(! eq) {
            std::printf("EXP: %s\n", exp_wkt);
            char* obt_wkt = GEOSWKTWriter_write(w_, g);
            std::printf("OBT: %s\n", obt_wkt);
            free(obt_wkt);
        }
        ensure(eq);
    }

    test_capigeosclipbyrect_data()
        : geom1_(nullptr), geom2_(nullptr), geom3_(nullptr), w_(nullptr)
    {
        initGEOS(notice, notice);
        w_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(w_, 1);
        GEOSWKTWriter_setRoundingPrecision(w_, 8);
    }

    ~test_capigeosclipbyrect_data()
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

typedef test_group<test_capigeosclipbyrect_data> group;
typedef group::object object;

group test_capigeosclipbyrect_group("capi::GEOSClipByRect");

//
// Test Cases
//

/// Point outside
template<> template<> void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POINT(0 0)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POINT EMPTY");
}

/// Point inside
template<> template<> void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("POINT(15 15)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POINT(15 15)");
}

/// Point on boundary
template<> template<> void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("POINT(15 10)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POINT EMPTY");
}

/// Line outside
template<> template<> void object::test<4>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, -5 5)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "LINESTRING EMPTY");
}

/// Line inside
template<> template<> void object::test<5>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(15 15, 16 15)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "LINESTRING(15 15, 16 15)");
}

/// Line on boundary
template<> template<> void object::test<6>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(10 15, 10 10, 15 10)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "LINESTRING EMPTY");
}

/// Line splitting rectangle
template<> template<> void object::test<7>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(10 5, 25 20)");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "LINESTRING (15 10, 20 15)");
}

/// Polygon shell (CCW) fully on rectangle boundary
template<> template<> void object::test<8>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((10 10, 20 10, 20 20, 10 20, 10 10))");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POLYGON((10 10, 20 10, 20 20, 10 20, 10 10))");
}

/// Polygon shell (CW) fully on rectangle boundary
template<> template<> void object::test<9>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((10 10, 10 20, 20 20, 20 10, 10 10))");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POLYGON((10 10, 20 10, 20 20, 10 20, 10 10))");
}

/// Polygon hole (CCW) fully on rectangle boundary
template<> template<> void object::test<10>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((0 0, 0 30, 30 30, 30 0, 0 0),(10 10, 20 10, 20 20, 10 20, 10 10))");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POLYGON EMPTY");
}

/// Polygon hole (CW) fully on rectangle boundary
template<> template<> void object::test<11>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((0 0, 0 30, 30 30, 30 0, 0 0),(10 10, 10 20, 20 20, 20 10, 10 10))");
    geom2_ = GEOSClipByRect(geom1_, 10, 10, 20, 20);
    isEqual(geom2_, "POLYGON EMPTY");
}

/// Polygon fully within rectangle
template<> template<> void object::test<12>
()
{
    const char* wkt = "POLYGON((1 1, 1 30, 30 30, 30 1, 1 1),(10 10, 20 10, 20 20, 10 20, 10 10))";
    geom1_ = GEOSGeomFromWKT(wkt);
    geom2_ = GEOSClipByRect(geom1_, 0, 0, 40, 40);
    isEqual(geom2_, wkt);
}

/// Polygon overlapping rectangle
template<> template<> void object::test<13>
()
{
    const char* wkt = "POLYGON((0 0, 0 30, 30 30, 30 0, 0 0),(10 10, 20 10, 20 20, 10 20, 10 10))";
    geom1_ = GEOSGeomFromWKT(wkt);
    geom2_ = GEOSClipByRect(geom1_, 5, 5, 15, 15);
    isEqual(geom2_, "POLYGON ((5 5, 5 15, 10 15, 10 10, 15 10, 15 5, 5 5))");
}

/// Empty combinations - always return GEOMETRYCOLLECTION EMPTY
template<> template<> void object::test<14>
()
{
    std::vector<const char*> variants{
        "POINT EMPTY",
        "LINESTRING EMPTY",
        "POLYGON EMPTY",
        "MULTIPOINT EMPTY",
        "MULTILINESTRING EMPTY",
        "MULTIPOLYGON EMPTY",
        "GEOMETRYCOLLECTION EMPTY",
        "LINEARRING EMPTY",
    };
    for (const auto& wkt : variants) {
        GEOSGeom geom1 = GEOSGeomFromWKT(wkt);
        GEOSGeom geom2 = GEOSClipByRect(geom1, 0, 0, 1, 1);
        char* obt_wkt = GEOSWKTWriter_write(w_, geom2);
        std::string obt_wkt2(obt_wkt);
        GEOSFree(obt_wkt);
        GEOSGeom_destroy(geom1);
        GEOSGeom_destroy(geom2);
        ensure_equals(obt_wkt2, "GEOMETRYCOLLECTION EMPTY");
    }
}


} // namespace tut

