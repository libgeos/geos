// $Id: computeOrientationTest.cpp 2344 2009-04-09 21:46:30Z mloskot $
// 
// Test Suite for CGAlgorithms::computeOrientation() function
// Ported from JTS junit/algorithm/ComputeOrientationTest.java

#include <tut.hpp>
// geos
#include <geos/algorithm/CGAlgorithms.h>
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

namespace tut
{
    //
    // Test Group
    //

    struct test_computeorientation_data
    {
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
    void object::test<1>()
    {
        const std::string wkt("LINESTRING ( 0 0, 0 1, 1 1)");
        Geometry::AutoPtr geom(reader_.read(wkt));

        CoordinateSequence::AutoPtr pts(geom->getCoordinates());

        int const a = CGAlgorithms::computeOrientation(pts->getAt(0), pts->getAt(1), pts->getAt(2));
        int const b = CGAlgorithms::computeOrientation(pts->getAt(0), pts->getAt(1), pts->getAt(2));
        int const c = CGAlgorithms::computeOrientation(pts->getAt(0), pts->getAt(1), pts->getAt(2));

        ensure_equals( a, b );
        ensure_equals( a, c );
    }

    // 2 - Test CCW orientation
    template<>
    template<>
    void object::test<2>()
    {    
        Coordinate c1(1.0000000000004998, -7.989685402102996);
        Coordinate c2(10.0, -7.004368924503866);
        Coordinate c3(1.0000000000005, -7.989685402102996);

        CoordinateArraySequence pts;
        pts.add(c1);
        pts.add(c2);
        pts.add(c3);

        int const a = CGAlgorithms::computeOrientation(pts[0], pts[1], pts[2]);
        int const b = CGAlgorithms::computeOrientation(pts[0], pts[1], pts[2]);
        int const c = CGAlgorithms::computeOrientation(pts[0], pts[1], pts[2]);

        ensure_equals( a, b );
        ensure_equals( a, c );
    }

} // namespace tut
