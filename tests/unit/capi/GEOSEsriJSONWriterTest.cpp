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

struct test_geosesrijsonwriter_data : public capitest::utility {
    GEOSEsriJSONWriter* writer_ = nullptr;
    char* json_ = nullptr;

    test_geosesrijsonwriter_data()
        : writer_(GEOSEsriJSONWriter_create())
    {
    }

    ~test_geosesrijsonwriter_data()
    {
        GEOSFree(json_);
        GEOSEsriJSONWriter_destroy(writer_);
    }
};

typedef test_group<test_geosesrijsonwriter_data> group;
typedef group::object object;

group test_geosesrijsonwriter_group("capi::GEOSEsriJSONWriter");

template<>
template<>
void object::test<1>()
{
    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure_equals(GEOSEsriJSONWriter_setTolerance(writer_, 0.001), 1);
    json_ = GEOSEsriJSONWriter_writeGeometry(writer_, input_);
    ensure(json_ != nullptr);
    ensure_equals(std::string(json_), std::string("{\"curvePaths\":[[[0,0],{\"c\":[[2,0],[1,1]]}]]}"));
    ensure_equals(GEOSEsriJSONWriter_getNumSegments(writer_), 1u);
    ensure_equals(GEOSEsriJSONWriter_getNumApproximated(writer_), 0u);
    ensure_equals(GEOSEsriJSONWriter_getMaxDeviation(writer_), 0.0);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("statistics of a clothoid transition, and the curvature option");

    input_ = fromWKT("CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), REFERENCEDIRECTIONS "
                     "(VECTOR (1 0), VECTOR (0 1))), SCALEFACTOR 50, STARTDISTANCE 0, ENDDISTANCE 100)");
    GEOSEsriJSONWriter_setTolerance(writer_, 0.001);
    json_ = GEOSEsriJSONWriter_writeGeometry(writer_, input_);
    ensure(json_ != nullptr);
    ensure(std::strstr(json_, "\"b\":[[") != nullptr);
    const size_t g2Segments = GEOSEsriJSONWriter_getNumSegments(writer_);
    ensure(g2Segments > 1);
    ensure_equals(GEOSEsriJSONWriter_getNumApproximated(writer_), g2Segments);
    const double dev = GEOSEsriJSONWriter_getMaxDeviation(writer_);
    ensure(dev > 0 && dev <= 0.001);

    // G1 only: a different, still bounded, result.
    GEOSEsriJSONWriter_setMatchCurvature(writer_, 0);
    char* g1 = GEOSEsriJSONWriter_writeGeometry(writer_, input_);
    ensure(g1 != nullptr);
    ensure(std::strcmp(g1, json_) != 0);
    ensure(GEOSEsriJSONWriter_getMaxDeviation(writer_) <= 0.001);
    GEOSFree(g1);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("errors return NULL or 0, and the statistics are zero after a failure");

    input_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    // No default tolerance.
    ensure(GEOSEsriJSONWriter_writeGeometry(writer_, input_) == nullptr);
    ensure_equals(GEOSEsriJSONWriter_setTolerance(writer_, 0.0), 0);
    ensure_equals(GEOSEsriJSONWriter_setTolerance(writer_, -1.0), 0);
    ensure_equals(GEOSEsriJSONWriter_setTolerance(writer_, 0.001), 1);
    json_ = GEOSEsriJSONWriter_writeGeometry(writer_, input_);
    ensure(json_ != nullptr);
    ensure_equals(GEOSEsriJSONWriter_getNumSegments(writer_), 1u);

    GEOSGeometry* coll = fromWKT("GEOMETRYCOLLECTION (POINT (1 1), LINESTRING (0 0, 1 1))");
    ensure(GEOSEsriJSONWriter_writeGeometry(writer_, coll) == nullptr);
    GEOSGeom_destroy(coll);
    ensure_equals(GEOSEsriJSONWriter_getNumSegments(writer_), 0u);
    ensure_equals(GEOSEsriJSONWriter_getMaxDeviation(writer_), 0.0);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("reentrant variants");

    GEOSContextHandle_t handle = GEOS_init_r();
    GEOSEsriJSONWriter* w = GEOSEsriJSONWriter_create_r(handle);
    GEOSGeometry* g = GEOSGeomFromWKT_r(handle, "CIRCULARSTRING (0 0, 1 1, 2 0)");
    ensure(g != nullptr);
    ensure_equals(GEOSEsriJSONWriter_setTolerance_r(handle, w, 0.001), 1);
    char* json = GEOSEsriJSONWriter_writeGeometry_r(handle, w, g);
    ensure(json != nullptr);
    ensure_equals(std::string(json), std::string("{\"curvePaths\":[[[0,0],{\"c\":[[2,0],[1,1]]}]]}"));
    ensure_equals(GEOSEsriJSONWriter_getNumSegments_r(handle, w), 1u);
    GEOSEsriJSONWriter_setMatchCurvature_r(handle, w, 1);
    GEOSFree_r(handle, json);
    GEOSGeom_destroy_r(handle, g);
    GEOSEsriJSONWriter_destroy_r(handle, w);
    GEOS_finish_r(handle);
}

} // namespace tut
