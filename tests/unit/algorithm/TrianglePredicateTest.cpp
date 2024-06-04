#include <tut/tut.hpp>

#include <geos/geom/Coordinate.h>
#include <geos/geom/Location.h>
#include <geos/triangulate/quadedge/TrianglePredicate.h>

using geos::geom::CoordinateXY;
using geos::geom::Location;
using geos::triangulate::quadedge::TrianglePredicate;

namespace tut {

struct test_trianglepredicate_data {};

typedef test_group<test_trianglepredicate_data> group;
typedef group::object object;

group test_trianglepredicate_group("geos::algorithm::TrianglePredicate");

// testInCircleNonRobust()
template<>
template<>
void object::test<1>
()
{
    const CoordinateXY p0 (-1, 0);
    const CoordinateXY p1 (0, 1);
    const CoordinateXY p2 (1, 0);

    ensure_equals(TrianglePredicate::isInCircleNonRobust(p0, p1, p2, CoordinateXY(0, 0)), Location::INTERIOR);
    ensure_equals(TrianglePredicate::isInCircleNonRobust(p0, p1, p2, CoordinateXY(0, -1)), Location::BOUNDARY);
    ensure_equals(TrianglePredicate::isInCircleNonRobust(p0, p1, p2, CoordinateXY(2, 0)), Location::EXTERIOR);
}

// testInCircleRobust()
template<>
template<>
void object::test<2>
()
{
    const CoordinateXY p0 (-1, 0);
    const CoordinateXY p1 (0, 1);
    const CoordinateXY p2 (1, 0);

    //ensure_equals(TrianglePredicate::isInCircleRobust(p0, p1, p2, CoordinateXY(0, 0)), Location::INTERIOR);
    ensure_equals(TrianglePredicate::isInCircleRobust(p0, p1, p2, CoordinateXY(0, -1)), Location::BOUNDARY);
    //ensure_equals(TrianglePredicate::isInCircleRobust(p0, p1, p2, CoordinateXY(2, 0)), Location::EXTERIOR);
}

}
