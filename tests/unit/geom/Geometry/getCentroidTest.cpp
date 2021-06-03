//
// Test Suite for Geometry's equals() function

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>
#include <string>

using namespace geos;
using namespace geos::geom;

namespace tut {

//
// Test Group
//
struct test_geom_getcentroid_data {
    typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;
    geos::io::WKTReader reader_;

    test_geom_getcentroid_data()
        : reader_()
    {}

    void
    checkCentroid(std::string inputWkt, std::string expectedWKT)
    {
        GeomPtr inputGeom(reader_.read(inputWkt));
        GeomPtr exectedGeom(reader_.read(expectedWKT));
        GeomPtr resultGeom = inputGeom->getCentroid();
        ensure_equals_geometry(resultGeom.get(), exectedGeom.get());
    }

};

typedef test_group<test_geom_getcentroid_data> group;
typedef group::object object;

group test_geom_getcentroid_data("geos::geom::Geometry::getCentroid");

//
// Test Cases
//

template<>
template<>
void object::test<1> () {

    checkCentroid(
        "LINESTRING(100 100, 200 200)",
        "POINT(150 150)");
}


template<>
template<>
void object::test<2> () {

    checkCentroid(
        "POLYGON((0 0, 100 0, 100 100, 0 100, 0 0))",
        "POINT(50 50)");
}

template<>
template<>
void object::test<3> () {

    checkCentroid(
        "POLYGON EMPTY",
        "POINT EMPTY");
}

} // namespace tut

