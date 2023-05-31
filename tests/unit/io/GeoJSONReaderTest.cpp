//
// Test Suite for geos::io::GeoJSONReader

// tut
#include <tut/tut.hpp>
// geos
#include <geos/io/ParseException.h>
#include <geos/io/GeoJSONReader.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Coordinate.h>
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

// Read a GeoJSON Point
template<>
template<>
void object::test<1>
()
{
    std::string geojson { "{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "POINT (-117 33)");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON LineString
template<>
template<>
void object::test<2>
()
{
    std::string geojson { "{\"type\":\"LineString\",\"coordinates\":[[102.0,0.0],[103.0,1.0],[104.0,0.0],[105.0,1.0]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "LINESTRING (102 0, 103 1, 104 0, 105 1)");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON Polygon with only an outer ring
template<>
template<>
void object::test<3>
()
{
    std::string geojson { "{\"type\":\"Polygon\",\"coordinates\":[[[30,10],[40,40],[20,40],[10,20],[30,10]]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10))");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON Point with an outer ring and an inner ring
template<>
template<>
void object::test<4>
()
{
    std::string geojson { "{\"type\":\"Polygon\",\"coordinates\":[[[35,10],[45,45],[15,40],[10,20],[35,10]],[[20,30],[35,35],[30,20],[20,30]]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "POLYGON ((35 10, 45 45, 15 40, 10 20, 35 10), (20 30, 35 35, 30 20, 20 30))");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON MultiPoint
template<>
template<>
void object::test<5>
()
{
    std::string geojson { "{\"type\":\"MultiPoint\",\"coordinates\":[[10, 40], [40, 30], [20, 20], [30, 10]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "MULTIPOINT ((10 40), (40 30), (20 20), (30 10))");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON MultiLineString
template<>
template<>
void object::test<6>
()
{
    std::string geojson { "{\"type\":\"MultiLineString\",\"coordinates\":[[[10, 10], [20, 20], [10, 40]],[[40, 40], [30, 30], [40, 20], [30, 10]]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "MULTILINESTRING ((10 10, 20 20, 10 40), (40 40, 30 30, 40 20, 30 10))");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON MultiPolygon
template<>
template<>
void object::test<7>
()
{
    std::string geojson { "{\"type\": \"MultiPolygon\", \"coordinates\": [[[[40, 40], [20, 45], [45, 30], [40, 40]]], [[[20, 35], [10, 30], [10, 10], [30, 5], [45, 20], [20, 35]], [[30, 20], [20, 15], [20, 25], [30, 20]]]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "MULTIPOLYGON (((40 40, 20 45, 45 30, 40 40)), ((20 35, 10 30, 10 10, 30 5, 45 20, 20 35), (30 20, 20 15, 20 25, 30 20)))");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON GeometryCollection
template<>
template<>
void object::test<8>
()
{
    std::string geojson { "{\"type\": \"GeometryCollection\",\"geometries\": [{\"type\": \"Point\",\"coordinates\": [40, 10]},{\"type\": \"LineString\",\"coordinates\": [[10, 10], [20, 20], [10, 40]]},{\"type\": \"Polygon\",\"coordinates\": [[[40, 40], [20, 45], [45, 30], [40, 40]]]}]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "GEOMETRYCOLLECTION (POINT (40 10), LINESTRING (10 10, 20 20, 10 40), POLYGON ((40 40, 20 45, 45 30, 40 40)))");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON Feature with a Point and no properties
template<>
template<>
void object::test<9>
()
{
    std::string geojson { "{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "POINT (-117 33)");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON FeatureCollection with two Feature with Points and no properties
template<>
template<>
void object::test<10>
()
{
    std::string geojson { "{\"type\":\"FeatureCollection\",\"features\":[{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}},{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-122.0,45.0]}}]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "GEOMETRYCOLLECTION (POINT (-117 33), POINT (-122 45))");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON empty Point
template<>
template<>
void object::test<11>
()
{
    std::string geojson { "{\"type\":\"Point\",\"coordinates\":[]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "POINT EMPTY");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON empty LineString
template<>
template<>
void object::test<12>
()
{
    std::string geojson { "{\"type\":\"LineString\",\"coordinates\":[]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "LINESTRING EMPTY");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON empty Polygon
template<>
template<>
void object::test<13>
()
{
    std::string geojson { "{\"type\":\"Polygon\",\"coordinates\":[]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "POLYGON EMPTY");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON empty MultiPoint
template<>
template<>
void object::test<14>
()
{
    std::string geojson { "{\"type\":\"MultiPoint\",\"coordinates\":[]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "MULTIPOINT EMPTY");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON empty MultiLineString
template<>
template<>
void object::test<15>
()
{
    std::string geojson { "{\"type\":\"MultiLineString\",\"coordinates\":[]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "MULTILINESTRING EMPTY");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON empty MultiPolygon
template<>
template<>
void object::test<16>
()
{
    std::string geojson { "{\"type\": \"MultiPolygon\", \"coordinates\": []}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "MULTIPOLYGON EMPTY");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read an empty GeometryCollection
template<>
template<>
void object::test<17>
()
{
    std::string geojson { "{\"type\": \"GeometryCollection\",\"geometries\": []}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "GEOMETRYCOLLECTION EMPTY");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a Simple Feature
template<>
template<>
void object::test<18>
()
{
    std::string geojson { "{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}, \"properties\": {\"id\": 1, \"name\": \"one\", \"required\": true} }" };
    geos::io::GeoJSONFeatureCollection features(geojsonreader.readFeatures(geojson));
    ensure_equals(features.getFeatures().size(), static_cast<size_t>(1));
    ensure_equals(static_cast<size_t>(features.getFeatures()[0].getGeometry()->getCoordinateDimension()), 2u);
    ensure_equals(features.getFeatures()[0].getGeometry()->toText(), "POINT (-117 33)");
    ensure_equals(features.getFeatures()[0].getProperties().at("id").getNumber(), 1.0);
    ensure_equals(features.getFeatures()[0].getProperties().at("name").getString(), "one");
    ensure_equals(features.getFeatures()[0].getProperties().at("required").getBoolean(), true);
}

// Read a Complicated Feature
template<>
template<>
void object::test<19>
()
{
    std::string geojson { "{\"type\":\"Feature\",\"geometry\":{\"type\":\"Point\",\"coordinates\":[-117.0,33.0]}, \"properties\": {\"id\": 1, \"name\": \"one\", \"items\": [1,2,3,4], \"nested\": {\"id\":2, \"name\":\"two\"}}}" };
    geos::io::GeoJSONFeatureCollection features(geojsonreader.readFeatures(geojson));
    ensure_equals(features.getFeatures().size(), static_cast<size_t>(1));
    ensure_equals(static_cast<size_t>(features.getFeatures()[0].getGeometry()->getCoordinateDimension()), 2u);
    ensure_equals(features.getFeatures()[0].getGeometry()->toText(), "POINT (-117 33)");
    ensure_equals(features.getFeatures()[0].getProperties().at("id").getNumber(), 1.0);
    ensure_equals(features.getFeatures()[0].getProperties().at("name").getString(), "one");
    std::vector<geos::io::GeoJSONValue> values = features.getFeatures()[0].getProperties().at("items").getArray();
    ensure_equals(values.size(), static_cast<size_t>(4));
    ensure_equals(values[0].getNumber(), 1.0);
    ensure_equals(values[1].getNumber(), 2.0);
    ensure_equals(values[2].getNumber(), 3.0);
    ensure_equals(values[3].getNumber(), 4.0);
}

// Read a FeatureCollection
template<>
template<>
void object::test<20>
()
{
    std::string geojson { "{\"type\":\"FeatureCollection\",\"features\":["
        "{\"type\":\"Feature\",\"properties\":{\"id\": 1},\"geometry\":{\"type\":\"Polygon\",\"coordinates\":[[[87.890,64.923],[76.992,55.178],[102.656,46.558],[115.312,60.413],[94.570,58.447],[87.890,64.923]]]}},"
        "{\"type\":\"Feature\",\"properties\":{\"id\": 2},\"geometry\":{\"type\":\"LineString\",\"coordinates\":[[1.406,48.690],[41.835,34.016],[22.5,13.923]]}},"
        "{\"type\":\"Feature\",\"properties\":{\"id\": 3},\"geometry\":{\"type\":\"Point\",\"coordinates\":[-28.125,39.095]}}"
     "]}" };
    geos::io::GeoJSONFeatureCollection features(geojsonreader.readFeatures(geojson));
    ensure_equals(features.getFeatures().size(), static_cast<size_t>(3));
    ensure_equals(static_cast<size_t>(features.getFeatures()[0].getGeometry()->getCoordinateDimension()), 2u);
    ensure_equals(features.getFeatures()[0].getGeometry()->toText(), "POLYGON ((87.89 64.923, 76.992 55.178, 102.656 46.558, 115.312 60.413, 94.57 58.447, 87.89 64.923))");
    ensure_equals(features.getFeatures()[0].getProperties().at("id").getNumber(), 1.0);
    ensure_equals(features.getFeatures()[1].getGeometry()->toText(), "LINESTRING (1.406 48.69, 41.835 34.016, 22.5 13.923)");
    ensure_equals(features.getFeatures()[1].getProperties().at("id").getNumber(), 2.0);
    ensure_equals(features.getFeatures()[2].getGeometry()->toText(), "POINT (-28.125 39.095)");
    ensure_equals(features.getFeatures()[2].getProperties().at("id").getNumber(), 3.0);
}

// Read a GeoJSON Polygon with an empty ring
template<>
template<>
void object::test<21>
()
{
    std::string geojson { "{\"type\":\"Polygon\",\"coordinates\":[[]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "POLYGON EMPTY");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON Point with only one coordinate
template<>
template<>
void object::test<22>
()
{
    std::string errorMessage;    
    std::string geojson { "{\"type\":\"Point\",\"coordinates\":[-117.0]}" };
    bool error = false;
    try {
        GeomPtr geom(geojsonreader.read(geojson));
    } catch (geos::io::ParseException& e) {
        error = true;
        errorMessage = e.what();
    }
    ensure(error == true);
    ensure_equals(errorMessage, "ParseException: Expected two coordinates found one");
}

// Throw ParseException for bad GeoJSON
template<>
template<>
void object::test<23>
()
{
    std::string errorMessage;
    std::string geojson { "<gml>NOT_GEO_JSON</gml>" };
    bool error = false;
    try {
        GeomPtr geom(geojsonreader.read(geojson));
    } catch (geos::io::ParseException& e) {
        error = true;
        errorMessage = e.what();
    }
    ensure(error == true);
    ensure(errorMessage.find("ParseException: Error parsing JSON") != std::string::npos);
}

// Throw error when LINESTRING has only one coordinate
template<>
template<>
void object::test<24>
()
{
    std::string errorMessage;
    bool error = false;
    try {    
        std::string geojson { "{\"type\":\"LineString\",\"coordinates\":[[1,2],[2]]}" };
        GeomPtr geom(geojsonreader.read(geojson));
    } catch (geos::io::ParseException& e) {
        error = true;
        errorMessage = e.what();
    }
    ensure(error == true);
    ensure_equals(errorMessage, "ParseException: Expected two coordinates found one");
}

// Throw error when geometry type is unsupported
template<>
template<>
void object::test<25>
()
{
    std::string errorMessage;
    bool error = false;
    try {    
        std::string geojson { "{\"type\":\"Line\",\"coordinates\":[[1,2],[2,3]]}" };
        GeomPtr geom(geojsonreader.read(geojson));
    } catch (geos::io::ParseException& e) {
        error = true;
        errorMessage = e.what();
    }
    ensure(error == true);
    ensure_equals(errorMessage, "ParseException: Unknown geometry type!");
}

// Throw error when LINESTRING has only one coordinate
template<>
template<>
void object::test<26>
()
{
    std::string errorMessage;
    bool error = false;
    try {    
        std::string geojson { "{\"type\":\"LineString\",\"coordinates\":[[1]]}" };
        GeomPtr geom(geojsonreader.read(geojson));
    } catch (geos::io::ParseException& e) {
        error = true;
        errorMessage = e.what();
    }
    ensure(error == true);
    ensure_equals(errorMessage, "ParseException: Expected two coordinates found one");
}

// Read a GeoJSON empty Polygon with empty shell and empty inner rings
template<>
template<>
void object::test<27>
()
{
    std::string geojson { "{\"type\":\"Polygon\",\"coordinates\":[[],[]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "POLYGON EMPTY");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON empty MultiLineString with empty LineStrings
template<>
template<>
void object::test<28>
()
{
    std::string geojson { "{\"type\":\"MultiLineString\",\"coordinates\":[[],[],[]]}" };
    GeomPtr geom(geojsonreader.read(geojson));
    ensure_equals(geom->toText(), "MULTILINESTRING EMPTY");
    ensure_equals(static_cast<size_t>(geom->getCoordinateDimension()), 2u);
}

// Read a GeoJSON Point with too many coordinates
template<>
template<>
void object::test<29>
()
{
    std::string errorMessage;    
    std::string geojson { "{\"type\":\"Point\",\"coordinates\":[1,2,3,4,5,6]}" };
    bool error = false;
    try {
        GeomPtr geom(geojsonreader.read(geojson));
    } catch (geos::io::ParseException& e) {
        error = true;
        errorMessage = e.what();
    }
    ensure(error == true);
    ensure_equals(errorMessage, "ParseException: Expected two coordinates found more than two");
}

// Throw ParseException for bad GeoJSON
template<>
template<>
void object::test<30>
()
{
    std::string errorMessage;
    std::string geojson { "{ \"missing\": \"type\" }" };
    bool error = false;
    try {
        GeomPtr geom(geojsonreader.read(geojson));
    } catch (geos::io::ParseException& e) {
        error = true;
        errorMessage = e.what();
    }
    ensure(error == true);
    ensure(errorMessage.find("ParseException: Error parsing JSON") != std::string::npos);
}

}

