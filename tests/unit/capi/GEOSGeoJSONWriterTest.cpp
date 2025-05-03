#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeojsonwriter_data : public capitest::utility {

    test_geosgeojsonwriter_data() :
        geojsonwriter_(GEOSGeoJSONWriter_create()),
        geojson_(nullptr)
    {}

    ~test_geosgeojsonwriter_data() {
        GEOSGeoJSONWriter_destroy(geojsonwriter_);
        GEOSFree(geojson_);
    }

    void
    test_writer_geojson(std::string const& geojson)
    {
        test_writer_geojson(geojson, geojson);
    }

    void
    test_writer_geojson(std::string const& geojson, std::string const& expected)
    {
        GEOSGeoJSONReader* reader = GEOSGeoJSONReader_create();
        GEOSGeometry* geom1 = GEOSGeoJSONReader_readGeometry(reader, &geojson[0]);
        ensure(nullptr != geom1);

        char* wkt_c = GEOSGeoJSONWriter_writeGeometry(geojsonwriter_, geom1, -1);
        std::string out(wkt_c);
        free(wkt_c);
        GEOSGeom_destroy(geom1);
        GEOSGeoJSONReader_destroy(reader);

        ensure_equals(out, expected);
    }

    GEOSGeoJSONWriter* geojsonwriter_;
    char* geojson_;
};

typedef test_group<test_geosgeojsonwriter_data> group;
typedef group::object object;

group test_geosgeojsonwriter("capi::GEOSGeoJSONWriter");

// Check default output dimension 3
template<>
template<>
void object::test<1>()
{
    ensure_equals(GEOSGeoJSONWriter_getOutputDimension(geojsonwriter_), 3);

    test_writer_geojson("{\"type\":\"Point\",\"coordinates\":[10.0,13.0]}");
    test_writer_geojson("{\"type\":\"Point\",\"coordinates\":[10.0,13.0,3.0]}");
}

// Check writer with output dimension 2
template<>
template<>
void object::test<2>()
{
    GEOSGeoJSONWriter_setOutputDimension(geojsonwriter_, 2);
    ensure_equals("getOutputDimension_2", GEOSGeoJSONWriter_getOutputDimension(geojsonwriter_), 2);

    test_writer_geojson("{\"type\":\"Point\",\"coordinates\":[10.0,13.0]}");
    test_writer_geojson("{\"type\":\"Point\",\"coordinates\":[10.0,13.0,3.0]}", "{\"type\":\"Point\",\"coordinates\":[10.0,13.0]}");
}

} // namespace tut

