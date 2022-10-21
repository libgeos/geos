#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosisvalidreason_data : public capitest::utility {};

typedef test_group<test_geosisvalidreason_data> group;
typedef group::object object;

group test_geosisvalidreason("capi::GEOSisValidReason");

template<>
template<>
void object::test<1>()
{
    GEOSGeometry* input = GEOSGeomFromWKT("LINESTRING (1 2, 4 5, 9 -2)");

    char* reason = GEOSisValidReason(input);

    ensure_equals(std::string(reason), "Valid Geometry");

    GEOSGeom_destroy(input);
    GEOSFree(reason);
}

template<>
template<>
void object::test<2>()
{
    GEOSGeometry* input = GEOSGeomFromWKT("POLYGON ((0 0, 1 0, 0 1, 1 1, 0 0))");

    char* reason = GEOSisValidReason(input);

    ensure_equals(std::string(reason), "Self-intersection[0.5 0.5]");

    GEOSGeom_destroy(input);
    GEOSFree(reason);
}

template<>
template<>
void object::test<3>()
{
    std::string wkb = "01060000C00100000001030000C00100000003000000E3D9107E234F5041A3DB66BC97A30F4122ACEF440DAF9440FFFFFFFFFFFFEFFFE3D9107E234F5041A3DB66BC97A30F4122ACEF440DAF9440FFFFFFFFFFFFEFFFE3D9107E234F5041A3DB66BC97A30F4122ACEF440DAF9440FFFFFFFFFFFFEFFF";
    input_ = GEOSGeomFromHEX_buf((unsigned char*) wkb.c_str(), wkb.size());

    char* reason = GEOSisValidReason(input_);
    ensure_equals(std::string(reason), "Too few points in geometry component[4275341.96977851 259186.966993061]");

    GEOSFree(reason);
}


} // namespace tut

