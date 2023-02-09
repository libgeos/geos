//
// Test Suite for geos::geom::Envelope class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>

#include <array>
#include <cfenv>
#include <unordered_set>

namespace tut {
//
// Test Group
//

using geos::geom::CoordinateXY;
using geos::geom::Envelope;

// dummy data, not used
struct test_envelope_data {
    test_envelope_data()
    {
        std::feclearexcept(FE_ALL_EXCEPT);
    }

    static std::size_t
    absdiff(std::size_t a, std::size_t b)
    {
        auto x = std::minmax(a, b);
        return x.second - x.first;
    }

    static void
    check_intersects(const Envelope& e1, const Envelope& e2, bool expected)
    {
        ensure_equals(e1.intersects(e2), expected);
        ensure_equals(e1.intersects(&e2), expected);
        ensure_equals(e1.disjoint(e2), !expected);
        ensure_equals(e1.disjoint(&e2), !expected);

        ensure_equals(e2.intersects(e1), expected);
        ensure_equals(e2.intersects(&e1), expected);
        ensure_equals(e2.disjoint(e1), !expected);
        ensure_equals(e2.disjoint(&e1), !expected);

        CoordinateXY p0, p1, q0, q1;

        if (!e2.isNull()) {
            q0 = {e2.getMinX(), e2.getMaxY()};
            q1 = {e2.getMaxX(), e2.getMinY()};
            ensure_equals(e1.intersects(q0, q1), expected);
            ensure_equals(e1.intersects(q1, q0), expected);
        }

        if (!e1.isNull()) {
            p0 = {e1.getMinX(), e1.getMinY()};
            p1 = {e1.getMaxX(), e1.getMaxY()};

            ensure_equals(e2.intersects(p0, p1), expected);
            ensure_equals(e2.intersects(p1, p0), expected);
        }

        if (!e1.isNull() && !e2.isNull()) {
            ensure_equals(Envelope::intersects(p0, p1, q0, q1), expected);
            ensure_equals(Envelope::intersects(p1, p0, q0, q1), expected);
            ensure_equals(Envelope::intersects(p1, p0, q1, q0), expected);
            ensure_equals(Envelope::intersects(p1, p0, q0, q1), expected);
        }
    }

    static void
    check_intersects(const Envelope& e1, const CoordinateXY& q, bool expected)
    {
        ensure_equals(e1.intersects(q), expected);
        ensure_equals(e1.intersects(q.x, q.y), expected);
        ensure_equals(e1.contains(q), expected);
        ensure_equals(e1.contains(q.x, q.y), expected);
        ensure_equals(e1.covers(&q), expected);
        ensure_equals(e1.covers(q.x, q.y), expected);

        if (!e1.isNull()) {
            CoordinateXY p0{e1.getMinX(), e1.getMinY()};
            CoordinateXY p1{e1.getMaxX(), e1.getMaxY()};
            ensure_equals(Envelope::intersects(p0, p1, q), expected);
        }
    }

    static void
    ensure_no_fp_except()
    {
        ensure("FE_DIVBYZERO raised", !std::fetestexcept(FE_DIVBYZERO));
        //ensure("FE_INEXACT raised", !std::fetestexcept(FE_INEXACT));
        ensure("FE_INVALID raised", !std::fetestexcept(FE_INVALID));
        ensure("FE_OVERFLOW raised", !std::fetestexcept(FE_OVERFLOW));
        ensure("FE_UNDERFLOW raised", !std::fetestexcept(FE_UNDERFLOW));
    }
};

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

    ensure_no_fp_except();
}

// 2 - Test of overridden constructor
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

    ensure_no_fp_except();
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

    ensure_no_fp_except();
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

    ensure_no_fp_except();
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
    geos::geom::Envelope inf(0, 100, 0, std::numeric_limits<double>::infinity());

    ensure(empty.isNull());
    ensure(!zero.isNull());
    ensure(!zero2.isNull());
    ensure(!box.isNull());

    /* See http://trac.osgeo.org/geos/ticket/703 */
    ensure("empty envelopes are equal", empty.equals(&empty));

    ensure(!empty.equals(&zero));
    ensure(!zero.equals(&empty));

    ensure(zero.equals(&zero2));
    ensure(zero2.equals(&zero));

    ensure(!box.equals(&empty));
    ensure(!box.equals(&zero));

    ensure(!inf.isfinite());

    ensure_no_fp_except();
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
    ensure("empty envelope does not contain non-empty envelope", !empty.contains(small));
    ensure("non-empty envelope does not contain empty envelope", !small.contains(empty));
    ensure("empty envelope does not contain self", !empty.contains(empty));

    // Test empty envelope by pointer
    ensure("empty envelope does not contain non-empty envelope", !empty.contains(&small));
    ensure("non-empty envelope does not contain empty envelope", !small.contains(&empty));
    ensure("empty envelope does not contain self", !empty.contains(&empty));

    // Test non-empty envelope by reference
    ensure("small envelope does not contain big envelope", !small.contains(big));
    ensure("big envelope contains small envelope", big.contains(small));
    ensure("non-empty envelope contains itself", big.contains(big));

    // Test points
    check_intersects(small, {0, 0}, true);
    check_intersects(small, {-1, -1}, true);
    check_intersects(small, {5, 5}, false);

    check_intersects(empty, {0, 0}, false);

    ensure_no_fp_except();
}

// Test of intersects() and disjoint()
template<>
template<>
void object::test<7>
()
{
    constexpr std::size_t nrow = 3;
    constexpr std::size_t ncol = 3;

    std::array<std::array<Envelope, ncol>, nrow> envelopes;

    double xmin = 0.0;
    double xmax = 1.0;
    double ymin = 0.0;
    double ymax = 2.0;
    double dx = (xmax - xmin) / static_cast<double>(ncol);
    double dy = (ymax - ymin) / static_cast<double>(nrow);
    for (std::size_t i = 0; i < ncol; i++) {
        for (std::size_t j = 0; j < nrow; j++) {
            double x0 = xmin + static_cast<double>(i)*dx;
            double x1 = xmin + static_cast<double>(i + 1)*dx;
            double y0 = ymax - static_cast<double>(j + 1)*dy;
            double y1 =	ymax - static_cast<double>(j)*dy;

            envelopes[i][j] = Envelope(x0, x1, y0, y1);
        }
    }

    geos::geom::Envelope empty;

    // check intersection against empty
    for (std::size_t ia = 0; ia < ncol; ia++) {
        for (std::size_t ja = 0; ja < nrow; ja++) {
            check_intersects(envelopes[ia][ja], empty, false);
        }
    }

    // check intersection against adjacent
    for (std::size_t ia = 0; ia < ncol; ia++) {
        for (std::size_t ja = 0; ja < nrow; ja++) {
            for (std::size_t ib = 0; ib < ncol; ib++) {
                for (std::size_t jb = 0; jb < nrow; jb++) {
                    bool should_intersect = absdiff(ia, ib) <= 1 && absdiff(ja, jb) <= 1;

                    check_intersects(envelopes[ia][ja], envelopes[ib][jb], should_intersect);
                }
            }
        }
    }


    ensure_no_fp_except();
}

// Test of expandToInclude()
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
    ensure("expanding envelope to include null envelope has no effect",
           box == exemplar);   // no change expected

    // Expand null envelope to include box envelope
    empty.expandToInclude(&box);
    ensure("expanding null envelope to include non-null envelope makes null envelope not null",
           empty == exemplar);

    ensure_no_fp_except();
}

// Second test of expandToInclude()
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

    ensure_no_fp_except();
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

    for (std::size_t i = 0; i < c.size(); i++) {
        c[i].x = static_cast<double>(i % 5);
        c[i].y = static_cast<double>(5 - (i / 5));
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

    ensure_no_fp_except();
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

    ensure_no_fp_except();
}

// comparison of empty envelopes
template<>
template<>
void object::test<12>
()
{
    Envelope empty1;
    Envelope empty2;

    ensure(!(empty1 < empty2));
    ensure(!(empty2 < empty1));

    ensure_no_fp_except();
}

// Envelope::intersects(Coordinate, Coordinate)
template<>
template<>
void object::test<13>
()
{
    Envelope empty;

    ensure(!empty.intersects({1, 1}, {2, 2}));

    ensure_no_fp_except();
}

// Test of expandBy
template<>
template<>
void object::test<14>
()
{
    // expanding null envelope is still null
    Envelope empty;
    empty.expandBy(10, 10);
    ensure(empty.isNull());

    // expanding a regular envelope gives expected result
    Envelope e(0, 1, -2, 2);
    e.expandBy(2, 1);
    ensure_equals(e.getMinX(), -2);
    ensure_equals(e.getMaxX(), 3);
    ensure_equals(e.getMinY(), -3);
    ensure_equals(e.getMaxY(), 3);

    // expanding envelope by negative amount shrinks it
    e.expandBy(-2, -1);
    ensure_equals(e, Envelope(0, 1, -2, 2));

    // shrinking it until it disappears makes it null
    e.expandBy(-100, -100);
    ensure(e.isNull());

    ensure_no_fp_except();
}

// Test of intersection
template<>
template<>
void object::test<15>
()
{
    Envelope a(0, 1, 0, 1);
    Envelope b(1, 2, 1, 1);
    Envelope c;
    Envelope d(100, 200, 100, 200);

    // A - B
    Envelope ab_intersection;
    bool ab_intersect = a.intersection(b, ab_intersection);
    ensure(ab_intersect);
    ensure_equals(ab_intersection, Envelope(1, 1, 1, 1));

    // A - C
    Envelope ac_intersection;
    bool ac_intersect = a.intersection(c, ac_intersection);
    ensure(!ac_intersect);
    ensure(ac_intersection.isNull());

    // A - D
    Envelope ad_intersection;
    bool ad_intersect = a.intersection(c, ad_intersection);
    ensure(!ad_intersect);
    ensure(ad_intersection.isNull());

    // B - C
    Envelope bc_intersection;
    bool bc_intersect = a.intersection(c, bc_intersection);
    ensure(!bc_intersect);
    ensure(bc_intersection.isNull());

    ensure_no_fp_except();
}

// Test of centre
template<>
template<>
void object::test<16>
()
{
    // regular envelope
    Envelope e(0, 1, 2, 4);
    CoordinateXY c;

    bool success = e.centre(c);
    ensure(success);
    ensure_equals(c, CoordinateXY{0.5, 3});

    // null envelope
    Envelope empty;
    success = empty.centre(c);
    ensure(!success);
    ensure_equals(c, CoordinateXY{0.5, 3});

    ensure_no_fp_except();
}

// Test of translate
template<>
template<>
void object::test<17>
()
{
    // regular envelope
    Envelope e(0, 1, 2, 4);
    e.translate(1, 2);
    ensure_equals(e, Envelope(1, 2, 4, 6));

    // null envelope
    Envelope empty;
    empty.translate(1, 2);
    ensure(empty.isNull());

    ensure_no_fp_except();
}

// Test of hashCode
template<>
template<>
void object::test<18>
()
{
    Envelope a(0, 1, 2, 3);
    Envelope b;

    ensure(a.hashCode() != b.hashCode());

    ensure_no_fp_except();
}

// Test of expandToInclude(Coordinate)
template<>
template<>
void object::test<19>
()
{
    Envelope e;
    e.expandToInclude({6, 7});

    ensure_equals(e, Envelope(6, 6, 7, 7));

    e.expandToInclude(0, 1);
    ensure_equals(e, Envelope(0, 6, 1, 7));

    ensure_no_fp_except();
}

// Test of HashCode
template<>
template<>
void object::test<20>
()
{
    std::unordered_set<Envelope, Envelope::HashCode> set;
    set.emplace(0, 1, 0, 1);
    set.emplace(1, 0, 1, 0);
    set.emplace(2, 0, 2, 1);

    ensure_equals(set.size(), 2u);
}

} // namespace tut

