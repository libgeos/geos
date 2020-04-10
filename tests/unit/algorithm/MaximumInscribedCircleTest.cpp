// Test Suite for geos::algorithm::MaximumInscribedCircle
// Based on MinimumBoundingCircleTest.cpp

#include <tut/tut.hpp>
// geos
#include <geos/algorithm/MaximumInscribedCircle.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/io/WKTReader.h>

// std
#include <string>
#include <cstdlib>

using namespace std; // Remove when we remove cout statements.

namespace tut {
//
//Test Group
//

struct test_maximuminscribedcircle_data {
    typedef geos::geom::Coordinate Coordinate;
    typedef geos::geom::Geometry Geometry;
    typedef geos::geom::GeometryFactory GeometryFactory;
    typedef geos::algorithm::MaximumInscribedCircle MaximumInscribedCircle;

    geos::io::WKTReader reader;
    std::unique_ptr<Geometry> geom;
    std::unique_ptr<Geometry> geomOut;
    GeometryFactory::Ptr geomFact = GeometryFactory::create();

    test_maximuminscribedcircle_data()
    {}

    void
    doMaximumInscribedCircleTest(std::string wktIn, geos::geom::Coordinate& centreOut,
                                 double radiusOut)
    {
        geom = reader.read(wktIn);
        MaximumInscribedCircle mic(geom.get(), 2);
        double actualRadius = mic.getRadius();
        geos::geom::Coordinate actualCenter = mic.getCenter();

        if(centreOut.isNull()) {
            ensure(centreOut.distance(actualCenter) < 0.0001);
        }
        if(radiusOut >= 0) {
            ensure(fabs(radiusOut - actualRadius) < 0.0001);
        }
    }
};

typedef test_group<test_maximuminscribedcircle_data> group;
typedef group::object object;

group test_maximuminscribedcircle_group("geos::algorithm::MaximumInscribedCircle");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    Coordinate c(10, 10);
    doMaximumInscribedCircleTest(
        "POINT (10 10)",
        c,
        0
    );
}

template<>
template<>
void object::test<2>
()
{
    Coordinate c(5, 12.5);
    doMaximumInscribedCircleTest(
        "POLYGON ((0.0 0.0, 0.0 30.0, 30.0 30.0, 30.0 0.0, 0.0 0.0), (10.0 10.0, 10.0 20.0, 20.0 20.0, 20.0 10.0, 10.0 10.0))",
        c,
        5
    );
}

} // namespace tut