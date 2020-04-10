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
    doMaximumInscribedCircleTest(std::string wktIn, unsigned int numSegments, geos::geom::Coordinate& centreOut,
                                 double radiusOut)
    {
        geom = reader.read(wktIn);
        MaximumInscribedCircle mic(geom.get(), numSegments);
        double actualRadius = mic.getRadius();
        geos::geom::Coordinate actualCenter = mic.getCenter();
        
        if(!centreOut.isNull()) {
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
    // Trivial point
    Coordinate c(10, 10);
    doMaximumInscribedCircleTest(
        "POINT (10 10)",
        2,
        c,
        0
    );
}

template<>
template<>
void object::test<2>
()
{
    // Trivial square
    Coordinate c(1, 1);
    doMaximumInscribedCircleTest(
        "POLYGON ((0.0 0.0, 2.0 0.0, 2.0 2.0, 0.0 2.0, 0.0 0.0))",
        2,
        c,
        1
    );
}

template<>
template<>
void object::test<3>
()
{
    // Square with the center cut out. True centroid lies inside hole.
    Coordinate c(5, 12.5);
    doMaximumInscribedCircleTest(
        "POLYGON ((0.0 0.0, 0.0 30.0, 30.0 30.0, 30.0 0.0, 0.0 0.0), \
                  (10.0 10.0, 10.0 20.0, 20.0 20.0, 20.0 10.0, 10.0 10.0))",
        2,
        c,
        5
    );
}

template<>
template<>
void object::test<4>
()
{
    // Polygon with no holes, but centroid does not intersect polygon
    Coordinate c(1.25, 3.5);
    doMaximumInscribedCircleTest(
        "POLYGON ((0.0 0.0, 1.0 0.0, 1.0 3.0, 2.0 3.0, 2.0 0.0, 3.0 0.0, 3.0 4.0, 0.0 4.0, 0.0 0.0))",
        2,
        c,
        0.5
    );
}

template<>
template<>
void object::test<5>
()
{
    // Multipolygon where polygon of smallest area actually contains largest inscribed circle.
    // However, algorithm uses polygon of largest area in calculation. This polygon is same as in test 4.
    Coordinate c(1.25, 3.5);
    doMaximumInscribedCircleTest(
        "MULTIPOLYGON (((0.0 0.0, 1.0 0.0, 1.0 3.0, 2.0 3.0, 2.0 0.0, 3.0 0.0, 3.0 4.0, 0.0 4.0, 0.0 0.0)), \
                       ((-3.0 -3.0, -1.0 -3.0, -1.0 -1.0, -3.0 -1.0, -3.0 -3.0)))",
        2,
        c,
        0.5
    );
}

template<>
template<>
void object::test<6>
()
{
    // Triangle that demonstrates more accurate results with increased segments.
    // Bisecting
    Coordinate c(3.75, 0.25);
    doMaximumInscribedCircleTest(
        "POLYGON ((0.0 0.0, 5.0 0.0, 5.0 1.0, 0.0 0.0))",
        2,
        c,
        0.25
    );
}

template<>
template<>
void object::test<7>
()
{
    // Triangle that demonstrates more accurate results with increased segments.
    // 10-secting
    Coordinate c(4.55, 0.45);
    doMaximumInscribedCircleTest(
        "POLYGON ((0.0 0.0, 5.0 0.0, 5.0 1.0, 0.0 0.0))",
        10,
        c,
        0.45
    );
}

template<>
template<>
void object::test<8>
()
{
    // Triangle that demonstrates more accurate results with increased segments.
    // 100-secting
    Coordinate c(4.54944, 0.450556);
    doMaximumInscribedCircleTest(
        "POLYGON ((0.0 0.0, 5.0 0.0, 5.0 1.0, 0.0 0.0))",
        100,
        c,
        0.450413
    );
}


} // namespace tut