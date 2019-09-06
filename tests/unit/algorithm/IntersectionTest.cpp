/**********************************************************************
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.osgeo.org
 *
 * Copyright (C) 2019 Martin Davis <mtnclimb@gmail.com>
 * Copyright (C) 2011      Sandro Santilli <strk@kbt.io>
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Public Licence as published
 * by the Free Software Foundation.
 * See the COPYING file for more information.
 *
 **********************************************************************/

//
// Test Suite for geos::algorithm::InteriorPointArea

#include <tut/tut.hpp>
// geos
#include <geos/geom/Coordinate.h>
#include <geos/algorithm/Intersection.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/Geometry.h>
// std
#include <sstream>
#include <string>
#include <memory>

namespace tut {
//
// Test Group
//

// dummy data, not used
struct test_intersection_data {
    typedef geos::geom::Geometry Geometry;
    typedef geos::geom::Coordinate Coordinate;
    typedef geos::algorithm::Intersection Intersection;

    geos::io::WKTReader reader;
    std::unique_ptr<Geometry> geom;

    double MAX_ABS_ERROR = 1e-5;

    void checkIntersectionNull(double p1x, double p1y, double p2x, double p2y,
                               double q1x, double q1y, double q2x, double q2y) {
        Coordinate p1(p1x, p1y);
        Coordinate p2(p2x, p2y);
        Coordinate q1(q1x, q1y);
        Coordinate q2(q2x, q2y);
        Coordinate actual = Intersection::intersection(p1, p2, q1, q2);
        ensure("checkIntersectionNull", actual.isNull());
    }

    void checkIntersection(double p1x, double p1y, double p2x, double p2y,
                           double q1x, double q1y, double q2x, double q2y,
                           double expectedx, double expectedy) {
        Coordinate p1(p1x, p1y);
        Coordinate p2(p2x, p2y);
        Coordinate q1(q1x, q1y);
        Coordinate q2(q2x, q2y);
        Coordinate expected(expectedx, expectedy);
        Coordinate actual = Intersection::intersection(p1, p2, q1, q2);
        double dist = actual.distance(expected);
        // std::cout << "Expected: " << expected << "  Actual: " << actual << "  Dist = " << dist << std::endl;
        ensure("checkIntersection", dist <= MAX_ABS_ERROR);
    }

    test_intersection_data()
    {}

};

typedef test_group<test_intersection_data> group;
typedef group::object object;

group test_intersection_data("geos::algorithm::Intersection");


template<>
template<>
void object::test<1>
()
{
    // testSimple
    checkIntersection(
        0,0,  10,10,
        0,10, 10,0,
        5,5);

    // testCollinear
    checkIntersectionNull(
        0,0,  10,10,
        20,20, 30, 30 );

    // testParallel
    checkIntersectionNull(
        0,0,  10,10,
        10,0, 20,10 );

    // testAlmostCollinear
    checkIntersection(
        35613471.6165017, 4257145.306132293, 35613477.7705378, 4257160.528222711,
        35613477.77505724, 4257160.539653536, 35613479.85607389, 4257165.92369170,
        35613477.772841461, 4257160.5339209242 );

    // testAlmostCollinearCond
    checkIntersection(
        1.6165017, 45.306132293, 7.7705378, 60.528222711,
        7.77505724, 60.539653536, 9.85607389, 65.92369170,
        7.772841461, 60.5339209242 );

}

} // namespace tut

