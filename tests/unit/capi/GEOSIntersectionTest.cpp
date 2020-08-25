//
// Test Suite for C-API GEOSintersection

#include "capi_test_utils.h"


namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosintersection_data : public capitest::utility
{

    GEOSWKTWriter* wktw_;
    GEOSGeometry* geom1_;
    GEOSGeometry* geom2_;
    GEOSGeometry* geom3_;
    GEOSGeometry* expected_;

    test_capigeosintersection_data()
        : geom1_(nullptr), geom2_(nullptr), geom3_(nullptr)
    {
        wktw_ = GEOSWKTWriter_create();
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setOutputDimension(wktw_, 3);
        geom1_ = nullptr;
        geom2_ = nullptr;
        geom3_ = nullptr;
        expected_ = nullptr;
    }

    ~test_capigeosintersection_data()
    {
        GEOSWKTWriter_destroy(wktw_);
        GEOSGeom_destroy(geom1_);
        GEOSGeom_destroy(geom2_);
        GEOSGeom_destroy(geom3_);
        if (expected_) GEOSGeom_destroy(expected_);
        geom1_ = nullptr;
        geom2_ = nullptr;
        geom3_ = nullptr;
        expected_ = nullptr;
    }

};

typedef test_group<test_capigeosintersection_data> group;
typedef group::object object;

group test_capigeosintersection_group("capi::GEOSIntersection");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON EMPTY");
    geom2_ = GEOSGeomFromWKT("POLYGON EMPTY");

    ensure(nullptr != geom1_);
    ensure(nullptr != geom2_);

    geom3_ = GEOSIntersection(geom1_, geom2_);
    ensure(nullptr != geom3_);
    ensure_geometry_equals(geom3_, "POLYGON EMPTY");
}

template<>
template<>
void object::test<2>
()
{
    geom1_ = GEOSGeomFromWKT("POLYGON((1 1,1 5,5 5,5 1,1 1))");
    geom2_ = GEOSGeomFromWKT("POINT(2 2)");

    ensure(nullptr != geom1_);
    ensure(nullptr != geom2_);

    geom3_ = GEOSIntersection(geom1_, geom2_);
    ensure(nullptr != geom3_);
    ensure_geometry_equals(geom3_, "POINT (2 2)");
}

template<>
template<>
void object::test<3>
()
{
    geom1_ = GEOSGeomFromWKT("MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)))");
    geom2_ = GEOSGeomFromWKT("POLYGON((-1 1,-1 2,2 2,2 1,-1 1))");
    expected_ = GEOSGeomFromWKT("POLYGON ((0 1, 0 2, 2 2, 2 1, 0 1))");

    ensure(nullptr != geom1_);
    ensure(nullptr != geom2_);

    geom3_ = GEOSIntersection(geom1_, geom2_);

    ensure(nullptr != geom3_);
    ensure_geometry_equals(geom3_, expected_, 0.1);
}

/* See http://trac.osgeo.org/geos/ticket/719 */
template<>
template<>
void object::test<4>
()
{
    geom1_ = GEOSGeomFromWKT("MULTIPOLYGON(((0 0,5 10,10 0,0 0),(1 1,1 2,2 2,2 1,1 1),(100 100,100 102,102 102,102 100,100 100)))");
    geom2_ = GEOSGeomFromWKT("POLYGON((0 1,0 2,10 2,10 1,0 1))");

    ensure(nullptr != geom1_);
    ensure(nullptr != geom2_);

    geom3_ = GEOSIntersection(geom1_, geom2_);
    GEOSNormalize(geom3_);

    ensure(nullptr != geom3_);

    expected_ = GEOSGeomFromWKT("GEOMETRYCOLLECTION (LINESTRING (1 2, 2 2), LINESTRING (2 1, 1 1), POLYGON ((0.5 1, 1 2, 1 1, 0.5 1)), POLYGON ((9 2, 9.5 1, 2 1, 2 2, 9 2)))");
    GEOSNormalize(expected_);
    ensure(GEOSEqualsExact(expected_, geom3_, 0.00001));
}

} // namespace tut

