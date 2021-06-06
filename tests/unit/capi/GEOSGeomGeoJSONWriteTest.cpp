//
// Test Suite for C-API GEOSGeoJSONReader_read

#include <tut/tut.hpp>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosgeomwritegeojson_data : public capitest::utility {
    GEOSGeoJSONWriter* writer_;

    test_capigeosgeomwritegeojson_data() : writer_(nullptr)
    {
        writer_ = GEOSGeoJSONWriter_create();
    }

    ~test_capigeosgeomwritegeojson_data()
    {
        GEOSGeoJSONWriter_destroy(writer_);
        writer_ = nullptr;
    }

    void
    test_geojson(std::string const& wkt, std::string const& expected, int type, int indent)
    {
        geom1_ = fromWKT(&wkt[0]);
        char* geojson_c = GEOSGeoJSONWriter_writeGeometry(writer_, geom1_, type, indent);
        ensure("GEOSGeoJSONWriter_writeGeometry failed to create GeoJSON", nullptr != geojson_c);

        std::string actual(geojson_c);
        free(geojson_c);
        ensure_equals(actual, expected);
    }
};

typedef test_group<test_capigeosgeomwritegeojson_data> group;
typedef group::object object;

group test_capigeosgeomwritegeojson_group("capi::GEOSGeomGeoJSONWrite");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    std::string wkt("POINT(-117.0 33.0)");
    std::string expected("{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}");
    test_geojson(wkt, expected, GEOSGEOJSON_GEOMETRY, -1);
}

template<>
template<>
void object::test<2>
()
{
    std::string wkt("POINT(-117.0 33.0)");
    std::string expected("{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}}");
    test_geojson(wkt, expected, GEOSGEOJSON_FEATURE, -1);
}

template<>
template<>
void object::test<3>
()
{
    std::string wkt("GEOMETRYCOLLECTION (POINT (-117.000 33.000), POINT (-122.000 45.000))");
    std::string expected("{\"type\":\"FeatureCollection\",\"features\":[{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}},{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-122.0,45.0]}}]}");
    test_geojson(wkt, expected, GEOSGEOJSON_FEATURE_COLLECTION, -1);
}
}
