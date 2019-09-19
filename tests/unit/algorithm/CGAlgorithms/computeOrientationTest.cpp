//
// Test Suite for Oriengation::index() function
// Ported from JTS junit/algorithm/ComputeOrientationTest.java

#include <tut/tut.hpp>
// geos
#include <geos/algorithm/CGAlgorithmsDD.h>
#include <geos/algorithm/Orientation.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/io/WKTReader.h>
// std
#include <string>
#include <memory>

using namespace geos::geom;
using namespace geos::algorithm;

namespace tut {
//
// Test Group
//

struct test_computeorientation_data {
    geos::io::WKTReader reader_;

    test_computeorientation_data() {}
};

typedef test_group<test_computeorientation_data> group;
typedef group::object object;

group test_computeorientation_group("geos::algorithm::CGAlgorithms::computeOrientation");

//
// Test Cases
//

// 1 - Test CCW orientation
template<>
template<>
void object::test<1>
()
{
    const std::string wkt("LINESTRING ( 0 0, 0 1, 1 1)");
    Geometry::Ptr geom(reader_.read(wkt));

    CoordinateSequence::Ptr pts(geom->getCoordinates());

    int const a = Orientation::index(pts->getAt(0), pts->getAt(1), pts->getAt(2));
    int const b = Orientation::index(pts->getAt(0), pts->getAt(1), pts->getAt(2));
    int const c = Orientation::index(pts->getAt(0), pts->getAt(1), pts->getAt(2));

    ensure_equals(a, b);
    ensure_equals(a, c);
}

// 2 - Test CCW orientation
template<>
template<>
void object::test<2>
()
{
    Coordinate c1(1.0000000000004998, -7.989685402102996);
    Coordinate c2(10.0, -7.004368924503866);
    Coordinate c3(1.0000000000005, -7.989685402102996);

    CoordinateArraySequence pts;
    pts.add(c1);
    pts.add(c2);
    pts.add(c3);

    int const a = Orientation::index(pts[0], pts[1], pts[2]);
    int const b = Orientation::index(pts[0], pts[1], pts[2]);
    int const c = Orientation::index(pts[0], pts[1], pts[2]);

    ensure_equals(a, b);
    ensure_equals(a, c);
}

// 3 - Test orientation for the original JTS test case
// jts/java/src/com/vividsolutions/jts/algorithm/RobustDeterminant.java, r626
// http://sourceforge.net/p/jts-topo-suite/code/626/
template<>
template<>
void object::test<3>
()
{
    Coordinate p0(219.3649559090992, 140.84159161824724);
    Coordinate p1(168.9018919682399, -5.713787599646864);
    Coordinate p(186.80814046338352, 46.28973405831556);
    // CGAlgorithms::orientationIndex gives both the same!!!
    // First case of doubledouble robustness improvement
    int orient = Orientation::index(p0, p1, p);
    int orientInv = Orientation::index(p1, p0, p);
    ensure(orient != orientInv);
}

// 4 - make sure CGAlgorithmsDD::checkSignOfDet2x2 isn't
// busted
template<>
template<>
void object::test<4>
()
{
    ensure(0 == CGAlgorithmsDD::signOfDet2x2(1.0, 1.0, 2.0, 2.0));
    ensure(1 == CGAlgorithmsDD::signOfDet2x2(1.0, 1.0, 2.0, 3.0));
    ensure(-1 == CGAlgorithmsDD::signOfDet2x2(1.0, 1.0, 3.0, 2.0));
}

} // namespace tut
