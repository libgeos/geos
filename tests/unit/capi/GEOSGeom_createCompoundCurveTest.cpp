#include <tut/tut.hpp>
#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeom_createcompoundcurve_data : public capitest::utility {};

typedef test_group<test_geosgeom_createcompoundcurve_data> group;
typedef group::object object;

group test_geosgeom_createcompoundcurve("capi::GEOSGeom_createCompoundCurve");

template<>
template<>
void object::test<1>
()
{
    GEOSGeometry* c1 = fromWKT("LINESTRING (0 0, 1 1)");
    GEOSGeometry* c2 = fromWKT("CIRCULARSTRING (1 1, 2 0, 3 1)");
    GEOSGeometry* curves[] = {c1, c2};

    result_ = GEOSGeom_createCompoundCurve(curves, 2);
    expected_ = fromWKT("COMPOUNDCURVE((0 0, 1 1), CIRCULARSTRING (1 1, 2 0, 3 1))");

    ensure_geometry_equals_identical(result_, expected_);
}

// Make sure we take ownership of inputs even in case of failure
template<>
template<>
void object::test<2>
()
{
    GEOSGeometry* c1 = fromWKT("LINESTRING (0 0, 1 1)");
    GEOSGeometry* c2 = fromWKT("POINT (13 2)");
    GEOSGeometry* curves[] = {c1, c2};

    result_ = GEOSGeom_createCompoundCurve(curves, 2);
    ensure(result_ == nullptr);
}

template<>
template<>
void object::test<3>
()
{
    result_ = GEOSGeom_createEmptyCompoundCurve();
    ensure(GEOSisEmpty(result_));
    ensure(!GEOSHasZ(result_));
    ensure(!GEOSHasM(result_));
}

} // namespace tut

