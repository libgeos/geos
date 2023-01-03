#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeomtype_data : public capitest::utility {};

typedef test_group<test_geosgeomtype_data> group;
typedef group::object object;

group test_geosgeomtype("capi::GEOSGeomType");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("POINT (1 2)");
    char* type1 = GEOSGeomType(geom1_);
    ensure_equals(std::string(type1), "Point");
    GEOSFree(type1);

    geom2_ = fromWKT("LINESTRING (1 2, 3 4)");
    char* type2 = GEOSGeomType(geom2_);
    ensure_equals(std::string(type2), "LineString");
    GEOSFree(type2);
}

} // namespace tut

