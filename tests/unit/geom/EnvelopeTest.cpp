//
// Test Suite for geos::geom::Envelope class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>

namespace tut {
//
// Test Group
//

// dummy data, not used
struct test_envelope_data {};

typedef test_group<test_envelope_data> group;
typedef group::object object;

group test_envelope_group("geos::geom::Envelope");

//
// Test Cases
//

// 1 - Test of default constructor
template<>
template<>
void object::test<1>
()
{
    geos::geom::Envelope empty;

    ensure(empty.isNull());

    ensure_equals(empty.getWidth(), 0);
    ensure_equals(empty.getHeight(), 0);
}

// 2 - Test of overriden constructor
template<>
template<>
void object::test<2>
()
{
    geos::geom::Envelope box(0, 100, 0, 100);

    ensure(!box.isNull());

    ensure_equals(box.getMinX(), 0);
    ensure_equals(box.getMaxX(), 100);
    ensure_equals(box.getMinY(), 0);
    ensure_equals(box.getMaxY(), 100);

    ensure_equals(box.getMinX(), box.getMinY());
    ensure_equals(box.getMaxX(), box.getMaxY());

    ensure_equals(box.getWidth(), box.getHeight());
}

// 3 - Test of copy constructor
template<>
template<>
void object::test<3>
()
{
    geos::geom::Envelope box(0, 100, 0, 100);

    ensure(!box.isNull());
    ensure_equals(box.getWidth(), box.getHeight());

    geos::geom::Envelope copied(box);

    ensure(!copied.isNull());
    ensure(copied == box);
    ensure_equals(copied.getWidth(), copied.getHeight());
}

// 4 - Test of setToNull()
template<>
template<>
void object::test<4>
()
{
    geos::geom::Envelope e(100, 200, 100, 200);

    ensure(!e.isNull());
    e.setToNull();
    ensure(e.isNull());
}

// 5 - Test of equals()
template<>
template<>
void object::test<5>
()
{
    geos::geom::Envelope empty;
    geos::geom::Envelope zero(0, 0, 0, 0);
    geos::geom::Envelope zero2(0, 0, 0, 0);
    geos::geom::Envelope box(0, 100, 0, 100);

    ensure(empty.isNull());
    ensure(!zero.isNull());
    ensure(!zero2.isNull());
    ensure(!box.isNull());

    /* See http://trac.osgeo.org/geos/ticket/703 */
    ensure(empty.equals(&empty));

    ensure(!empty.equals(&zero));
    ensure(!zero.equals(&empty));

    ensure(zero.equals(&zero2));
    ensure(zero2.equals(&zero));

    ensure(!box.equals(&empty));
    ensure(!box.equals(&zero));
}

// 6 - Test of contains()
template<>
template<>
void object::test<6>
()
{
    geos::geom::Envelope empty;
    geos::geom::Envelope small(-1, 1, -1, 1);
    geos::geom::Envelope big(-5, 5, -5, 5);

    ensure(empty.isNull());
    ensure(!small.isNull());
    ensure(!big.isNull());

    // Test empty envelope by reference
    ensure("empty envelope is not empty!", !empty.contains(small));
    ensure("empty envelope is not empty!", !small.contains(empty));

    // Test empty envelope by pointer
    ensure("empty envelope is not empty!", !empty.contains(&small));
    ensure("empty envelope is not empty!", !small.contains(&empty));

    // Test non-empty envelope by reference
    ensure(!small.contains(big));
    ensure(big.contains(small));

    // Test raw point
    ensure(small.contains(0, 0));
    ensure(small.contains(-1, -1));
    ensure(!small.contains(5, 5));

    // Test coordinate
    geos::geom::Coordinate origin(0, 0, 0);

    ensure_equals(origin.x, 0);
    ensure_equals(origin.y, 0);
    ensure_equals(origin.z, 0);
    ensure(small.contains(origin));
}

// Test of intersects()
template<>
template<>
void object::test<7>
()
{
    geos::geom::Envelope empty;
    geos::geom::Envelope with_origin(-100, 100, -100, 100);
    geos::geom::Envelope moved(50, 150, 50, 150);

    ensure(empty.isNull());
    ensure(!with_origin.isNull());
    ensure(!moved.isNull());

    // Test empty envelope by reference
    ensure("empty envelope seems not empty!", !empty.intersects(with_origin));
    ensure("empty envelope seems not empty!", !with_origin.intersects(empty));

    // Test empty envelope by pointer
    ensure("empty envelope seems not empty!", !empty.intersects(&with_origin));
    ensure("empty envelope seems not empty!", !with_origin.intersects(&empty));

    // Test non-empty envelope by reference
    ensure(with_origin.intersects(moved));
    ensure(moved.intersects(with_origin));

    // Test intersection with raw point
    ensure(with_origin.intersects(0, 0));
    ensure(with_origin.intersects(-100, 100));
    ensure(!with_origin.intersects(-200, 200));

    // Test intersection with coordinate
    geos::geom::Coordinate origin(0, 0, 0);

    ensure_equals(origin.x, 0);
    ensure_equals(origin.y, 0);
    ensure_equals(origin.z, 0);
    ensure(with_origin.intersects(origin));

}

// Test of expand()
template<>
template<>
void object::test<8>
()
{
    geos::geom::Envelope empty;
    geos::geom::Envelope box(-5, 5, -5, 5); // Includes (0,0) point
    const geos::geom::Envelope exemplar(-5, 5, -5, 5);

    ensure(empty.isNull());
    ensure(!box.isNull());
    ensure(!exemplar.isNull());

    // Expand box envelope to include null envelope
    box.expandToInclude(&empty);
    ensure(box == exemplar);   // no change expected

    // Expand null envelope to include box envelope
    empty.expandToInclude(&box);
    ensure(empty == exemplar);
}

// Second test of expand()
template<>
template<>
void object::test<9>
()
{
    geos::geom::Envelope empty;
    geos::geom::Envelope box(100, 101, 100, 101); // Does not include (0,0) point
    const geos::geom::Envelope exemplar(100, 101, 100, 101);

    // Expand box envelope to include null envelope
    box.expandToInclude(&empty);
    ensure(box == exemplar);   // no change expected

    // Expand empty envelope to include bigger envelope
    empty.expandToInclude(&box);
    ensure(empty == exemplar);
}

// Test point-to-envelope distance
template<>
template<>
void object::test<10>
()
{
    using geos::geom::Coordinate;
    using geos::geom::Envelope;

    // Create a 5x5 grid of points and use them to test various
    // spatial arrangements of the envelope and test point

    //  0  1  2  3  4
    //  5  6  7  8  9
    // 10 11 12 13 14
    // 15 16 17 18 19
    // 20 21 22 23 24
    std::vector<Coordinate> c(25);

    // std::cout<<std::endl;
    for (size_t i = 0; i < c.size(); i++) {
        c[i].x = static_cast<double>(i % 5);
        c[i].y = static_cast<double>(5 - (i / 5));
        // std::cout<< c[i] << std::endl;
    }

    // point contained in envelope
    ensure_equals( Envelope::distanceToCoordinate(c[18], c[22], c[9]), 0);
    ensure_equals( Envelope::distanceToCoordinate(c[18], c[14], c[18]), 0);
    ensure_equals( Envelope::distanceToCoordinate(c[18], c[14], c[17]), 0);
    ensure_equals( Envelope::distanceToCoordinate(c[18], c[19], c[22]), 0);

    // envelope above point
    ensure_equals(Envelope::distanceToCoordinate(c[17], c[5], c[4]), 2);

    // envelope below point
    ensure_equals(Envelope::distanceToCoordinate(c[7], c[20], c[19]), 2);

    // envelope left of point
    ensure_equals(Envelope::distanceToCoordinate(c[13], c[20], c[11]), 2);

    // envelope right of point
    ensure_equals(Envelope::distanceToCoordinate(c[5], c[9], c[8]), 3);

    // envelope upper-left of point
    ensure_equals(Envelope::distanceToCoordinate(c[17], c[6], c[0]),
            c[17].distance(c[6]));

    // envelope upper-right of point
    ensure_equals(Envelope::distanceToCoordinate(c[21], c[9], c[13]),
            c[21].distance(c[13]));

    // envelope lower-left of point
    ensure_equals(Envelope::distanceToCoordinate(c[3], c[10], c[21]),
                  c[3].distance(c[11]));

    // envelope lower-right of point
    ensure_equals(Envelope::distanceToCoordinate(c[6], c[12], c[14]),
                  c[6].distance(c[12]));

}

// Test envelope distance
template<>
template<>
void object::test<11>
()
{
    using geos::geom::Coordinate;
    using geos::geom::Envelope;

    // b touches a
    Envelope a{{0, 0}, {5, 5}};
    Envelope b({5, 5}, {10, 10});
    ensure_equals(a.distance(b), 0);
    ensure_equals(a.distance(b), b.distance(a));

    // b within a
    a = Envelope({0, 0}, {10, 10});
    b = Envelope({3, 3}, {3, 3});
    ensure_equals(a.distance(b), 0);
    ensure_equals(a.distance(b), b.distance(a));

    // b overlaps a
    a = Envelope({0, 0}, {5, 5});
    b = Envelope({2, 2}, {8, 8});
    ensure_equals(a.distance(b), 0);
    ensure_equals(a.distance(b), b.distance(a));

    // b above a
    a = Envelope({2, 3}, {5, 7});
    b = Envelope({0, 10}, {10, 20});
    ensure_equals(a.distance(b), 3);
    ensure_equals(a.distance(b), b.distance(a));

    // b right of a
    a = Envelope({2, 3}, {5, 7});
    b = Envelope({9, 4}, {11, 12});
    ensure_equals(a.distance(b), 4);
    ensure_equals(a.distance(b), b.distance(a));

    // b above and right of a
    a = Envelope({0, 0}, {5, 7});
    b = Envelope({9, 13}, {12, 28});
    ensure_equals(a.distance(b), Coordinate(5, 7).distance(Coordinate(9, 13)));
    ensure_equals(a.distance(b), b.distance(a));

    // b below and right of a
    a = Envelope({10, 11}, {13, 28});
    b = Envelope({17, 3}, {20, 5});
    ensure_equals(a.distance(b), Coordinate(13, 11).distance(Coordinate(17, 5)));
    ensure_equals(a.distance(b), b.distance(a));
}

} // namespace tut

