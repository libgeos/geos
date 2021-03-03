//
// Test Suite for geos::io::GeoJSONReader

// tut
#include <tut/tut.hpp>
// geos
#include <geos/io/GeoJSONReader.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
// std
#include <sstream>
#include <string>
#include <memory>

namespace tut {

//
// Test Group
//

// dummy data, not used
struct test_geojsonreader_data {
    geos::geom::PrecisionModel pm;
    geos::geom::GeometryFactory::Ptr gf;
    geos::io::GeoJSONReader geojsonreader;
    typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;

    test_geojsonreader_data()
        :
        pm(1000.0),
        gf(geos::geom::GeometryFactory::create(&pm)),
        geojsonreader(*(gf.get()))
    {}

};

typedef test_group<test_geojsonreader_data> group;
typedef group::object object;

group test_geojsonreader_group("geos::io::GeoJSONReader");

template<>
template<>
void object::test<1>
()
{
    std::string geojson { "{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("POINT (-117.000 33.000)", geom->toText());
}

template<>
template<>
void object::test<2>
()
{
    std::string geojson { "{\"type\":\"LineString\",\"coordinates\":[[102.0,0.0],[103.0,1.0],[104.0,0.0],[105.0,1.0]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("LINESTRING (102.000 0.000, 103.000 1.000, 104.000 0.000, 105.000 1.000)", geom->toText());
}

template<>
template<>
void object::test<3>
()
{
    std::string geojson { "{\"type\":\"Polygon\",\"coordinates\":[[[30,10],[40,40],[20,40],[10,20],[30,10]]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("POLYGON ((30.000 10.000, 40.000 40.000, 20.000 40.000, 10.000 20.000, 30.000 10.000))", geom->toText());
}

template<>
template<>
void object::test<4>
()
{
    std::string geojson { "{\"type\":\"Polygon\",\"coordinates\":[[[35,10],[45,45],[15,40],[10,20],[35,10]],[[20,30],[35,35],[30,20],[20,30]]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("POLYGON ((35.000 10.000, 45.000 45.000, 15.000 40.000, 10.000 20.000, 35.000 10.000), (20.000 30.000, 35.000 35.000, 30.000 20.000, 20.000 30.000))", geom->toText());
}

template<>
template<>
void object::test<5>
()
{
    std::string geojson { "{\"type\":\"MultiPoint\",\"coordinates\":[[10, 40], [40, 30], [20, 20], [30, 10]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("MULTIPOINT (10.000 40.000, 40.000 30.000, 20.000 20.000, 30.000 10.000)", geom->toText());
}

template<>
template<>
void object::test<6>
()
{
    std::string geojson { "{\"type\":\"MultiLineString\",\"coordinates\":[[[10, 10], [20, 20], [10, 40]],[[40, 40], [30, 30], [40, 20], [30, 10]]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("MULTILINESTRING ((10.000 10.000, 20.000 20.000, 10.000 40.000), (40.000 40.000, 30.000 30.000, 40.000 20.000, 30.000 10.000))", geom->toText());
}

template<>
template<>
void object::test<7>
()
{
    std::string geojson { "{\"type\": \"MultiPolygon\", \"coordinates\": [[[[40, 40], [20, 45], [45, 30], [40, 40]]], [[[20, 35], [10, 30], [10, 10], [30, 5], [45, 20], [20, 35]], [[30, 20], [20, 15], [20, 25], [30, 20]]]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("MULTIPOLYGON (((40.000 40.000, 20.000 45.000, 45.000 30.000, 40.000 40.000)), ((20.000 35.000, 10.000 30.000, 10.000 10.000, 30.000 5.000, 45.000 20.000, 20.000 35.000), (30.000 20.000, 20.000 15.000, 20.000 25.000, 30.000 20.000)))", geom->toText());
}

template<>
template<>
void object::test<8>
()
{
    std::string geojson { "{\"type\": \"GeometryCollection\",\"geometries\": [{\"type\": \"Point\",\"coordinates\": [40, 10]},{\"type\": \"LineString\",\"coordinates\": [[10, 10], [20, 20], [10, 40]]},{\"type\": \"Polygon\",\"coordinates\": [[[40, 40], [20, 45], [45, 30], [40, 40]]]}]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("GEOMETRYCOLLECTION (POINT (40.000 10.000), LINESTRING (10.000 10.000, 20.000 20.000, 10.000 40.000), POLYGON ((40.000 40.000, 20.000 45.000, 45.000 30.000, 40.000 40.000)))", geom->toText());
}

template<>
template<>
void object::test<9>
()
{
    std::string geojson { "{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("POINT (-117.000 33.000)", geom->toText());

}

template<>
template<>
void object::test<10>
()
{
    std::string geojson { "{\"type\":\"FeatureCollection\",\"features\":[{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}},{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-122.0,45.0]}}]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("GEOMETRYCOLLECTION (POINT (-117.000 33.000), POINT (-122.000 45.000))", geom->toText());

}

template<>
template<>
void object::test<11>
()
{
    std::string geojson { "{\"type\":\"Point\",\"coordinates\":[]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("POINT EMPTY", geom->toText());
}

template<>
template<>
void object::test<12>
()
{
    std::string geojson { "{\"type\":\"LineString\",\"coordinates\":[]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("LINESTRING EMPTY", geom->toText());
}

template<>
template<>
void object::test<13>
()
{
    std::string geojson { "{\"type\":\"Polygon\",\"coordinates\":[]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("POLYGON EMPTY", geom->toText());
}

template<>
template<>
void object::test<14>
()
{
    std::string geojson { "{\"type\":\"MultiPoint\",\"coordinates\":[]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("MULTIPOINT EMPTY", geom->toText());
}

template<>
template<>
void object::test<15>
()
{
    std::string geojson { "{\"type\":\"MultiLineString\",\"coordinates\":[]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("MULTILINESTRING EMPTY", geom->toText());
}

template<>
template<>
void object::test<16>
()
{
    std::string geojson { "{\"type\": \"MultiPolygon\", \"coordinates\": []}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("MULTIPOLYGON EMPTY", geom->toText());
}

// Read an empty GeometryCollection
template<>
template<>
void object::test<17>
()
{
    std::string geojson { "{\"type\": \"GeometryCollection\",\"geometries\": []}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals("GEOMETRYCOLLECTION EMPTY", geom->toText());
}

// Read a Feature
template<>
template<>
void object::test<18>
()
{
    std::string geojson { "{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}, \"properties\": {\"id\": 1, \"name\": \"one\", \"items\": [1,2,3,4]}}" };
    geos::io::GeoJSONFeatureCollection features(geojsonreader.readFeatures(geojson));
    ensure_equals(static_cast<size_t>(1), features.getFeatures().size());
    ensure_equals("POINT (-117.000 33.000)", features.getFeatures()[0].getGeometry()->toText());
    ensure_equals(1.0, features.getFeatures()[0].getProperties()["id"].numberValue);
    ensure_equals("one", features.getFeatures()[0].getProperties()["name"].stringValue);
    std::vector<geos::io::GeoJSONValue> values = features.getFeatures()[0].getProperties()["items"].arrayValue;
    ensure_equals(static_cast<size_t>(4), values.size());
    ensure_equals(1.0, values[0].numberValue);
    ensure_equals(2.0, values[1].numberValue);
    ensure_equals(3.0, values[2].numberValue);
    ensure_equals(4.0, values[3].numberValue);
}

// Read a FeatureCollection
template<>
template<>
void object::test<19>
()
{
    std::string geojson { "{\"type\":\"FeatureCollection\",\"features\":["
        "{\"type\":\"Feature\",\"properties\":{\"id\": 1},\"geometry\":{\"type\":\"Polygon\",\"coordinates\":[[[87.890,64.923],[76.992,55.178],[102.656,46.558],[115.312,60.413],[94.570,58.447],[87.890,64.923]]]}},"
        "{\"type\":\"Feature\",\"properties\":{\"id\": 2},\"geometry\":{\"type\":\"LineString\",\"coordinates\":[[1.406,48.690],[41.835,34.016],[22.5,13.923]]}},"
        "{\"type\":\"Feature\",\"properties\":{\"id\": 3},\"geometry\":{\"type\":\"Point\",\"coordinates\":[-28.125,39.095]}}"
     "]}" };
    geos::io::GeoJSONFeatureCollection features(geojsonreader.readFeatures(geojson));
    ensure_equals(static_cast<size_t>(3), features.getFeatures().size());
    ensure_equals("POLYGON ((87.890 64.923, 76.992 55.178, 102.656 46.558, 115.312 60.413, 94.570 58.447, 87.890 64.923))", features.getFeatures()[0].getGeometry()->toText());
    ensure_equals(1.0, features.getFeatures()[0].getProperties()["id"].numberValue);
    ensure_equals("LINESTRING (1.406 48.690, 41.835 34.016, 22.500 13.923)", features.getFeatures()[1].getGeometry()->toText());
    ensure_equals(2.0, features.getFeatures()[1].getProperties()["id"].numberValue);
    ensure_equals("POINT (-28.125 39.095)", features.getFeatures()[2].getGeometry()->toText());
    ensure_equals(3.0, features.getFeatures()[2].getProperties()["id"].numberValue);

}


}

