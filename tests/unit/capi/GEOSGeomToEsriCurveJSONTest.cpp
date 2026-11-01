#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

#include <cstring>
#include <string>

namespace tut {
//
// Test Group
//

struct test_geosgeomtoesricurvejson_data : public capitest::utility {
    char* json_ = nullptr;

    ~test_geosgeomtoesricurvejson_data()
    {
        GEOSFree(json_);
    }
};

typedef test_group<test_geosgeomtoesricurvejson_data> group;
typedef group::object object;

group test_geosgeomtoesricurvejson_group("capi::GEOSGeomToEsriCurveJSON");

template<>
template<>
void object::test<1>()
{
    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    json_ = GEOSGeomToEsriCurveJSON(input_, 0.001);
    ensure(json_ != nullptr);
    ensure_equals(std::string(json_), std::string("{\"curvePaths\":[[[0,0],{\"c\":[[2,0],[1,1]]}]]}"));
}

template<>
template<>
void object::test<2>()
{
    set_test_name("statistics of a clothoid transition");

    input_ = fromWKT("CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), REFERENCEDIRECTIONS "
                     "(VECTOR (1 0), VECTOR (0 1))), SCALEFACTOR 50, STARTDISTANCE 0, ENDDISTANCE 25)");
    size_t segments = 0, approximated = 0;
    double deviation = -1;
    json_ = GEOSGeomToEsriCurveJSONWithStats(input_, 0.001, &segments, &approximated, &deviation);
    ensure(json_ != nullptr);
    ensure(std::strstr(json_, "\"b\":[[") != nullptr);
    ensure(segments >= 1 && segments <= 2);
    ensure_equals(approximated, segments);
    ensure(deviation > 0 && deviation <= 0.001);

    // The output pointers are optional.
    GEOSFree(json_);
    json_ = GEOSGeomToEsriCurveJSONWithStats(input_, 0.001, nullptr, nullptr, nullptr);
    ensure(json_ != nullptr);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("errors return NULL");

    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure(GEOSGeomToEsriCurveJSON(input_, 0.0) == nullptr);
    ensure(GEOSGeomToEsriCurveJSON(input_, -1.0) == nullptr);
    GEOSGeometry* coll = fromWKT("GEOMETRYCOLLECTION (POINT (1 1), LINESTRING (0 0, 1 1))");
    ensure(GEOSGeomToEsriCurveJSON(coll, 0.001) == nullptr);
    GEOSGeom_destroy(coll);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("reentrant variant");

    GEOSContextHandle_t handle = GEOS_init_r();
    GEOSGeometry* g = GEOSGeomFromWKT_r(handle, "CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure(g != nullptr);
    char* json = GEOSGeomToEsriCurveJSON_r(handle, g, 0.001);
    ensure(json != nullptr);
    ensure_equals(std::string(json), std::string("{\"curvePaths\":[[[0,0],{\"c\":[[2,0],[1,1]]}]]}"));
    ensure(GEOSGeomToEsriCurveJSON_r(handle, g, 0.0) == nullptr);
    GEOSFree_r(handle, json);
    GEOSGeom_destroy_r(handle, g);
    GEOS_finish_r(handle);
}

} // namespace tut
