#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeom_getcoordinatedimension_data : public capitest::utility {};

typedef test_group<test_geosgeom_getcoordinatedimension_data> group;
typedef group::object object;

group test_geosgeom_getcoordinatedimension("capi::GEOSGeom_getCoordinateDimension");

template<>
template<>
void object::test<1>
()
{
    input_ = fromWKT("POLYGON ((0 0, 1 0, 1 1, 0 0))");
    ensure_equals(GEOSGeom_getCoordinateDimension(input_), 2);
}

template<>
template<>
void object::test<2>
()
{
    input_ = fromWKT("POINT (4 2 7)");
    ensure_equals(GEOSGeom_getCoordinateDimension(input_), 3);
}

template<>
template<>
void object::test<3>
()
{
    input_ = fromWKT("LINESTRING (4 2 7 1, 8 2 9 5)");
    ensure_equals(GEOSGeom_getCoordinateDimension(input_), 4);
}

template<>
template<>
void object::test<4>
()
{
    input_ = fromWKT("CIRCULARSTRING Z (0 0 0, 1 1 1, 2 0 2)");
    ensure_equals(GEOSGeom_getCoordinateDimension(input_), 3);
}

} // namespace tut

