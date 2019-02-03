//
// Test Suite for C-API GEOSRelateBoundaryNodeRule

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
struct test_capigeosrelateboundarynoderule_data {
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
    char* pat_;

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

    test_capigeosrelateboundarynoderule_data()
        : geom1_(nullptr), geom2_(nullptr), pat_(nullptr)
    {
        initGEOS(notice, notice);
    }

    ~test_capigeosrelateboundarynoderule_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSFree(pat_);
        finishGEOS();
    }

};

typedef test_group<test_capigeosrelateboundarynoderule_data> group;
typedef group::object object;

group test_capigeosrelateboundarynoderule_group("capi::GEOSRelateBoundaryNodeRule");

//
// Test Cases
//

// Closed line touching open line on endpoint with OGC rule
template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0, 10 10, 0 0)");
    geom2_ = GEOSGeomFromWKT("LINESTRING(0 0, 0 -10)");
    pat_ = GEOSRelateBoundaryNodeRule(geom1_, geom2_, GEOSRELATE_BNR_OGC);
    ensure_equals(std::string(pat_), std::string("F01FFF102"));
}

// Closed line touching open line on endpoint with MOD2 rule
template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0, 10 10, 0 0)");
    geom2_ = GEOSGeomFromWKT("LINESTRING(0 0, 0 -10)");
    pat_ = GEOSRelateBoundaryNodeRule(geom1_, geom2_, GEOSRELATE_BNR_MOD2);
    ensure_equals(std::string(pat_), std::string("F01FFF102"));
}

// Closed line touching open line on endpoint with ENDPOINT rule
template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 0, 10 0, 10 10, 0 0)");
    geom2_ = GEOSGeomFromWKT("LINESTRING(0 0, 0 -10)");
    pat_ = GEOSRelateBoundaryNodeRule(geom1_, geom2_,
                                      GEOSRELATE_BNR_ENDPOINT);
    ensure_equals(std::string(pat_), std::string("FF1F0F102"));
}

// Noded multiline touching line on node , MOD2 rule
template<>
template<>
void object::test<4>
()
{
    geom1_ = GEOSGeomFromWKT("MULTILINESTRING((0 0, 10 0),(10 0, 10 10))");
    geom2_ = GEOSGeomFromWKT("LINESTRING(10 0, 10 -10)");
    pat_ = GEOSRelateBoundaryNodeRule(geom1_, geom2_,
                                      GEOSRELATE_BNR_MOD2);
    ensure_equals(std::string(pat_), std::string("F01FF0102"));
}

// Noded multiline touching line on node , ENDPOINT rule
template<>
template<>
void object::test<5>
()
{
    geom1_ = GEOSGeomFromWKT("MULTILINESTRING((0 0, 10 0),(10 0, 10 10))");
    geom2_ = GEOSGeomFromWKT("LINESTRING(10 0, 10 -10)");
    pat_ = GEOSRelateBoundaryNodeRule(geom1_, geom2_,
                                      GEOSRELATE_BNR_ENDPOINT);
    ensure_equals(std::string(pat_), std::string("FF1F00102"));
}

// Noded multiline touching line on node , MULTIVALENT ENDPOINT rule
// NOTE: the single line has no boundary !
template<>
template<>
void object::test<6>
()
{
    geom1_ = GEOSGeomFromWKT("MULTILINESTRING((0 0, 10 0),(10 0, 10 10))");
    geom2_ = GEOSGeomFromWKT("LINESTRING(10 0, 10 -10)");
    pat_ = GEOSRelateBoundaryNodeRule(geom1_, geom2_,
                                      GEOSRELATE_BNR_MULTIVALENT_ENDPOINT);
    ensure_equals(std::string(pat_), std::string("0F1FFF1F2"));
}

// Noded multiline touching line on node , MONOVALENT ENDPOINT rule
template<>
template<>
void object::test<7>
()
{
    geom1_ = GEOSGeomFromWKT("MULTILINESTRING((0 0, 10 0),(10 0, 10 10))");
    geom2_ = GEOSGeomFromWKT("LINESTRING(10 0, 10 -10)");
    pat_ = GEOSRelateBoundaryNodeRule(geom1_, geom2_,
                                      GEOSRELATE_BNR_MONOVALENT_ENDPOINT);
    ensure_equals(std::string(pat_), std::string("F01FF0102"));
}

// Invalid/unknown rule
template<>
template<>
void object::test<8>
()
{
    geom1_ = GEOSGeomFromWKT("MULTILINESTRING((0 0, 10 0),(10 0, 10 10))");
    geom2_ = GEOSGeomFromWKT("LINESTRING(10 0, 10 -10)");
    pat_ = GEOSRelateBoundaryNodeRule(geom1_, geom2_, 5);
    ensure(nullptr == pat_);
}



} // namespace tut

