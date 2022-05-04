//
// Test Suite for C-API GEOSNode

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
struct test_capigeosnode_data {
    GEOSGeometry* geom1_ = nullptr;
    GEOSGeometry* geom2_ = nullptr;
    GEOSWKTWriter* w_ = nullptr;
    char* wkt_ = nullptr;

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

    test_capigeosnode_data()
        : geom1_(nullptr), geom2_(nullptr), w_(nullptr)
    {
        initGEOS(notice, notice);
        w_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(w_, 1);
    }

    ~test_capigeosnode_data()
    {
        if (geom1_) GEOSGeom_destroy(geom1_);
        if (geom1_) GEOSGeom_destroy(geom2_);
        if (w_) GEOSWKTWriter_destroy(w_);
        if (wkt_) GEOSFree(wkt_);
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
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 10, 10 0, 0 10)");
    geom2_ = GEOSNode(geom1_);
    ensure(nullptr != geom2_);

    GEOSNormalize(geom2_);
    wkt_ = GEOSWKTWriter_write(w_, geom2_);
    std::string out(wkt_);

    ensure_equals(out,
        "MULTILINESTRING ((5 5, 10 10, 10 0, 5 5), (0 10, 5 5), (0 0, 5 5))"
        );
}

/// Overlapping lines
template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("MULTILINESTRING((0 0, 2 0, 4 0),(5 0, 3 0, 1 0))");
    geom2_ = GEOSNode(geom1_);
    ensure(nullptr != geom2_);

    GEOSNormalize(geom2_);
    wkt_ = GEOSWKTWriter_write(w_, geom2_);
    std::string out(wkt_);

    ensure_equals(out,
        "MULTILINESTRING ((4 0, 5 0), (3 0, 4 0), (2 0, 3 0), (1 0, 2 0), (0 0, 1 0))"
        );
}

/// Equal lines
template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("MULTILINESTRING((0 0, 2 0, 4 0),(0 0, 2 0, 4 0))");
    geom2_ = GEOSNode(geom1_);
    ensure(nullptr != geom2_);

    GEOSNormalize(geom2_);
    wkt_ = GEOSWKTWriter_write(w_, geom2_);
    std::string out(wkt_);

    ensure_equals(out, "MULTILINESTRING ((2 0, 4 0), (0 0, 2 0))");
}

// https://gis.stackexchange.com/questions/345341/get-location-of-postgis-geos-topology-exception/345482#345482
template<>
template<>
void object::test<4>
()
{
    std::string wkb = "010500000002000000010200000003000000dc874d65fcc25ec176032c6b350c5341b336429ffec25ec1f962bbd"
                      "9480c5341fc849518ffc25ec15be20f5f500c5341010200000006000000fa9bbfd3fcc25ec1b978232f390c5341"
                      "b336429ffec25ec1f962bbd9480c5341a77e6be5fec25ec1357c21334d0c5341c3eba27bfec25ec11be5a4c34a0"
                      "c5341b61d8cacfcc25ec1bcf273143c0c5341fa9bbfd3fcc25ec1b978232f390c5341";

    geom1_ = GEOSGeomFromHEX_buf((const unsigned char*) wkb.c_str(), wkb.size());
    geom2_ = GEOSNode(geom1_);

    // Noding currently fails for this case.
    // ensure(geom2_);
}


// https://github.com/libgeos/geos/issues/601
template<>
template<>
void object::test<5>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING EMPTY");
    geom2_ = GEOSNode(geom1_);
    ensure(nullptr != geom2_);

    wkt_ = GEOSWKTWriter_write(w_, geom2_);
    std::string out(wkt_);

    ensure_equals(out, "LINESTRING EMPTY");
}

} // namespace tut

