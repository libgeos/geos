#include <tut/tut.hpp>
#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeom_createcircularstring_data : public capitest::utility {};

typedef test_group<test_geosgeom_createcircularstring_data> group;
typedef group::object object;

group test_geosgeom_createcircularstring("capi::GEOSGeom_createCircularString");

template<>
template<>
void object::test<1>
()
{
    GEOSCoordSequence* seq = GEOSCoordSeq_create(3, 2);
    GEOSCoordSeq_setXY(seq, 0, 1, 2);
    GEOSCoordSeq_setXY(seq, 1, 4, 5);
    GEOSCoordSeq_setXY(seq, 2, 9, -2);

    result_ = GEOSGeom_createCircularString(seq);
    expected_ = fromWKT("CIRCULARSTRING (1 2, 4 5, 9 -2)");

    ensure_geometry_equals_identical(result_, expected_);
}

template<>
template<>
void object::test<2>
()
{
    result_ = GEOSGeom_createEmptyCircularString();
    ensure(GEOSisEmpty(result_));
    ensure(!GEOSHasZ(result_));
    ensure(!GEOSHasM(result_));
}

} // namespace tut

