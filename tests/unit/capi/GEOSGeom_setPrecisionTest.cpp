
#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosgeomsetprecision_data : public capitest::utility {

    GEOSWKTWriter* wktw_ = nullptr;
    GEOSGeometry* geom1_ = nullptr;
    GEOSGeometry* geom2_ = nullptr;
    GEOSGeometry* geom3_ = nullptr;

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
    {
        wktw_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setRoundingPrecision(wktw_, 10);
        //GEOSWKTWriter_setOutputDimension(wktw_, 3);
    }

    ~test_capigeosgeomsetprecision_data()
    {
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        if(geom3_)
            GEOSGeom_destroy(geom3_);
        if (wktw_)
            GEOSWKTWriter_destroy(wktw_);

        geom1_ = geom2_ = geom3_ = 0;
        wktw_ = 0;
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
    ensure_equals(GEOSGeom_getPrecision(geom1_), 0.0);

    geom3_ = GEOSGeom_setPrecision(geom1_, 2.0, 0);
    ensure(geom3_ != 0);
    ensure_geometry_equals(geom3_, "POLYGON EMPTY");
    ensure_equals(GEOSGeom_getPrecision(geom3_), 2.0);
}

template<>
template<>
void object::test<2>
()
{
    geom1_ = fromWKT("LINESTRING(-3 6, 9 1)");
    geom3_ = GEOSGeom_setPrecision(geom1_, 2.0, 0);
    ensure(geom3_ != 0);
    ensure_geometry_equals(geom3_, "LINESTRING (-2 6, 10 2)");
}

// See effects of precision reduction on intersection operation
template<>
template<>
void object::test<3>
()
{
    geom1_ = fromWKT("LINESTRING(2 10, 4 30)");
    geom2_ = fromWKT("LINESTRING(4 10, 2 30)");

    geom3_ = GEOSIntersection(geom1_, geom2_);
    ensure(geom3_ != 0);
    ensure_geometry_equals(geom3_, "POINT (3 20)");

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
    ensure_geometry_equals(geom3_, "POINT (4 20)");

    // One input with precision grid of 0.5, the other of 2.0
    g = GEOSGeom_setPrecision(geom1_, 0.5, 0);
    ensure(g != 0);
    GEOSGeom_destroy(geom1_);
    geom1_ = g;
    GEOSGeom_destroy(geom3_);
    geom3_ = GEOSIntersection(geom1_, geom2_);
    ensure(geom3_ != 0);
    ensure_geometry_equals(geom3_, "POINT (3 20)");

    ensure_equals(GEOSGeom_getPrecision(geom1_), 0.5);
    ensure_equals(GEOSGeom_getPrecision(geom2_), 2.0);
}

// Retain (or not) topology
template<>
template<>
void object::test<4>
()
{
    geom1_ = fromWKT("POLYGON((10 10,20 10,16 15,20 20, 10 20, 14 15, 10 10))");

    geom2_ = GEOSGeom_setPrecision(geom1_, 5.0, 0);
    ensure_geometry_equals(geom2_,
        "MULTIPOLYGON (((10 10, 15 15, 20 10, 10 10)), ((15 15, 10 20, 20 20, 15 15)))");

    geom3_ = GEOSGeom_setPrecision(geom1_, 5.0, GEOS_PREC_NO_TOPO);
    ensure_geometry_equals(geom3_,
        "POLYGON ((10 10, 20 10, 15 15, 20 20, 10 20, 15 15, 10 10))");
}

// Retain (or not) collapsed elements
template<>
template<>
void object::test<5>
()
{
    geom1_ = fromWKT("LINESTRING(1 0, 2 0)");

    geom2_ = GEOSGeom_setPrecision(geom1_, 5.0, 0);
    ensure_geometry_equals(geom2_, "LINESTRING EMPTY");

    geom3_ = GEOSGeom_setPrecision(geom1_, 5.0, GEOS_PREC_KEEP_COLLAPSED);
    ensure_equals(toWKT(geom3_), "LINESTRING (0 0, 0 0)");
}

// Retain (or not) collapsed elements
template<>
template<>
void object::test<6> ()
{
    geom1_ = fromWKT("LINESTRING (0 0, 0.1 0.1)");
    geom2_ = GEOSGeom_setPrecision(geom1_, 1.0, 0);
    ensure_geometry_equals(geom2_, "LINESTRING EMPTY");
}

// Retain (or not) collapsed elements
template<>
template<>
void object::test<8> ()
{
    geom1_ = fromWKT("LINESTRING (0 0, 0.1 0.1)");
    geom2_ = GEOSGeom_setPrecision(geom1_, 1.0, GEOS_PREC_KEEP_COLLAPSED);
    ensure_geometry_equals(geom2_, "LINESTRING (0 0, 0 0)");
}

// Retain (or not) collapsed elements
template<>
template<>
void object::test<9> ()
{
    geom1_ = fromWKT("LINESTRING (0 0, 0.1 0.1)");
    geom2_ = GEOSGeom_setPrecision(geom1_, 1.0, GEOS_PREC_KEEP_COLLAPSED | GEOS_PREC_NO_TOPO);
    ensure_geometry_equals(geom2_, "LINESTRING (0 0, 0 0)");
}


// Collapse a linearRing / Trac #1135
template<>
template<>
void object::test<10> ()
{
    geom1_ = fromWKT("LINEARRING (0 0, 0.1 0, 0.1 0.1, 0 0.1, 0 0)");
    geom2_ = GEOSGeom_setPrecision(geom1_, 1.0, 0);
    ensure_geometry_equals(geom2_, "LINEARRING EMPTY");
}

template<>
template<>
void object::test<12>()
{
    geom1_ = fromWKT("POLYGON ((0 0, 0.1 0, 0.1 0.1, 0 0.1, 0 0))");
    geom2_ = GEOSGeom_setPrecision(geom1_, 1.0, 0);

    ensure_equals(GEOSGeom_getCoordinateDimension(geom2_), 2);
}

} // namespace tut

