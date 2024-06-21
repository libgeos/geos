#include <tut/tut.hpp>
#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeom_createcurvepolygon_data : public capitest::utility {};

typedef test_group<test_geosgeom_createcurvepolygon_data> group;
typedef group::object object;

group test_geosgeom_createcurvepolygon("capi::GEOSGeom_createCurvePolygon");

template<>
template<>
void object::test<1>
()
{
    GEOSGeometry* shell = fromWKT("CIRCULARSTRING(0 0, 4 0, 4 4, 0 4, 0 0)");
    GEOSGeometry* hole = fromWKT("LINESTRING (1 1, 3 3, 3 1, 1 1)");
    GEOSGeometry* holes[] = {hole};

    result_ = GEOSGeom_createCurvePolygon(shell, holes, 1);
    expected_ = fromWKT("CURVEPOLYGON( CIRCULARSTRING(0 0, 4 0, 4 4, 0 4, 0 0), (1 1, 3 3, 3 1, 1 1) )");

    ensure_geometry_equals_identical(result_, expected_);
}

// Ownership transferred on error
template<>
template<>
void object::test<2>
()
{
    GEOSGeometry* shell = fromWKT("CIRCULARSTRING(0 0, 4 0, 4 4, 0 4, 0 0)");
    GEOSGeometry* hole1 = fromWKT("POINT (3 3)");
    GEOSGeometry* hole2 = fromWKT("LINESTRING (1 1, 3 3, 3 1, 1 1)");
    GEOSGeometry* holes[] = {hole1, hole2};

    result_ = GEOSGeom_createCurvePolygon(shell, holes, 2);
    ensure(result_ == nullptr);
}

template<>
template<>
void object::test<3>
()
{
    result_ = GEOSGeom_createEmptyCurvePolygon();
    ensure(GEOSisEmpty(result_));
    ensure(!GEOSHasZ(result_));
    ensure(!GEOSHasM(result_));
}

} // namespace tut

