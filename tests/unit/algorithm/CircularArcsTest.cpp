#include <tut/tut.hpp>

#include <geos/geom/Coordinate.h>
#include <geos/algorithm/CircularArcs.h>
#include <geos/geom/CircularArc.h>
#include <geos/geom/Quadrant.h>
#include <geos/algorithm/Orientation.h>

#include "utility.h"

using geos::geom::CircularArc;
using geos::geom::CoordinateXY;
using geos::algorithm::CircularArcs;
using geos::geom::Envelope;
using geos::geom::Quadrant;
using geos::MATH_PI;

namespace tut {

struct test_circulararcs_data {
    const double eps = 1e-8;

    using XY = CoordinateXY;

    void checkEnvelope(const CoordinateXY& p0, const CoordinateXY& p1, const CoordinateXY& p2,
                       double xmin, double ymin, double xmax, double ymax) const
    {
        {
            Envelope e;
            CircularArcs::expandEnvelope(e, p0, p1, p2);

            ensure_equals("p0-p1-p2 xmin", e.getMinX(), xmin, eps);
            ensure_equals("p0-p1-p2 xmax", e.getMaxX(), xmax, eps);
            ensure_equals("p0-p1-p2 ymin", e.getMinY(), ymin, eps);
            ensure_equals("p0-p1-p2 ymax", e.getMaxY(), ymax, eps);
        }

        {
            Envelope e;
            CircularArcs::expandEnvelope(e, p2, p1, p0);

            ensure_equals("p2-p1-p0 xmin", e.getMinX(), xmin, eps);
            ensure_equals("p2-p1-p0 xmax", e.getMaxX(), xmax, eps);
            ensure_equals("p2-p1-p0 ymin", e.getMinY(), ymin, eps);
            ensure_equals("p2-p1-p0 ymax", e.getMaxY(), ymax, eps);
        }
    }

    void checkArc(const std::string& message,
                  const CoordinateXY& center, double radius, int orientation, double from, double to,
                  const CoordinateXY& p0, const CoordinateXY& p1, const CoordinateXY& p2) const
    {
        CoordinateXY fromPt = CircularArcs::createPoint(center, radius, from);
        CoordinateXY toPt = CircularArcs::createPoint(center, radius, to);

        auto arc = CircularArc::create(fromPt, toPt, center, radius, orientation);

        if (arc.p0().distance(p0) > eps || arc.p1().distance(p1) > eps || arc.p2().distance(p2) > eps) {
            ensure_equals(message, arc.toString(), CircularArc::create(p0, p1, p2).toString());
        }
    }

    static void
    checkArcIntersectionPoint(const CircularArc& a, const CircularArc& b, const CoordinateXY& expected, double tol = 0)
    {
        auto actual =  CircularArcs::arcIntersectionPoint(a.getCenter(), a.getRadius(), a.p0(), a.p2(), a.isCCW(),
                                                   b.getCenter(), b.getRadius(), b.p0(), b.p2(), b.isCCW());

        ensure(actual.has_value());

        ensure_equals_xy(actual.value(), expected, tol);
    }

    static void
    checkArcDisjoint(const CircularArc& a, const CircularArc& b)
    {
        auto actual =  CircularArcs::arcIntersectionPoint(a.getCenter(), a.getRadius(), a.p0(), a.p2(), a.isCCW(),
                                                   b.getCenter(), b.getRadius(), b.p0(), b.p2(), b.isCCW());

        ensure(!actual.has_value());
    }

    const CircularArc unit_semi = CircularArc::create(XY{-1, 0},
        XY{0, 1},
        XY{1, 0});
};

using group = test_group<test_circulararcs_data>;
using object = group::object;

group test_circulararcs_group("geos::algorithm::CircularArcs");

template<>
template<>
void object::test<1>()
{
    CoordinateXY p0{0, 10};
    CoordinateXY p1{100, 110};
    CoordinateXY p2{200, 10};

    auto center = CircularArcs::getCenter(p0, p1, p2);

    ensure_equals(center, CoordinateXY{100, 10});
}

template<>
template<>
void object::test<2>()
{
    CoordinateXY p0{0, 0};
    CoordinateXY p1{1, 1};
    CoordinateXY p2{0, 2};

    auto center = CircularArcs::getCenter(p0, p1, p2);

    ensure_equals(center, CoordinateXY{0, 1});
}

template<>
template<>
void object::test<3>()
{
    CoordinateXY p0{54.22, 31.8};
    CoordinateXY p1{16.07, 11.9};
    CoordinateXY p2{12.22, 3.99};

    auto center = CircularArcs::getCenter(p0, p1, p2);

    ensure(center.distance(CoordinateXY{52.0123, -10.486}) < 1e-4);
}

// complete circle
template<>
template<>
void object::test<4>()
{
    CoordinateXY p0{3, 4};
    CoordinateXY p1{7, 8};
    CoordinateXY p2{3, 4};

    auto center = CircularArcs::getCenter(p0, p1, p2);

    ensure_equals(center, CoordinateXY{5, 6});
}

// collinear
template<>
template<>
void object::test<5>()
{
    CoordinateXY p0{1, 2};
    CoordinateXY p1{2, 3};
    CoordinateXY p2{3, 4};

    auto center = CircularArcs::getCenter(p0, p1, p2);

    ensure(std::isnan(center.x));
    ensure(std::isnan(center.y));
}

// CCW quadrant 2 to quadrant 1
template<>
template<>
void object::test<6>()
{
    CoordinateXY p0{-std::sqrt(2), -std::sqrt(2)};
    CoordinateXY p1{2, 0};
    CoordinateXY p2{-std::sqrt(2), std::sqrt(2)};

    checkEnvelope(p0, p1, p2,
                  -std::sqrt(2), -2, 2, 2);
}

// quadrant 0 to quadrant 0, crossing all axes
template<>
template<>
void object::test<7>()
{
    CoordinateXY p0{std::sqrt(2), std::sqrt(2)};
    CoordinateXY p1{2, 0};
    CoordinateXY p2{std::sqrt(3), 1};

    checkEnvelope(p0, p1, p2, -2, -2, 2, 2);
}


// quadrant 0 to quadrant 0, crossing no axes
template<>
template<>
void object::test<8>()
{
    CoordinateXY p0{1, std::sqrt(3)};
    CoordinateXY p1{std::sqrt(2), std::sqrt(2)};
    CoordinateXY p2{std::sqrt(3), 1};

    checkEnvelope(p0, p1, p2,
                  1, 1, std::sqrt(3), std::sqrt(3));
}

// half circle with start points on -/+ x axis
template<>
template<>
void object::test<9>()
{
    CoordinateXY p0{-1, 0};
    CoordinateXY p1{0, 1};
    CoordinateXY p2{1, 0};

    checkEnvelope(p0, p1, p2,
                  -1, 0, 1, 1);
}

// CCW quadrant 0 to quadrant 3
template<>
template<>
void object::test<10>()
{
    CoordinateXY p0{std::sqrt(2), std::sqrt(2)};
    CoordinateXY p1{-2, 0};
    CoordinateXY p2{std::sqrt(2), -std::sqrt(2)};

    checkEnvelope(p0, p1, p2,
                  -2, -2, std::sqrt(2), 2);
}

// collinear
template<>
template<>
void object::test<11>()
{
    CoordinateXY p0{-1, -1};
    CoordinateXY p1{1, 1};
    CoordinateXY p2{2, 2};

    checkEnvelope(p0, p1, p2,
                  -1, -1, 2, 2);
}

template<>
template<>
void object::test<12>()
{
    set_test_name("envelope: arc defined by three collinear points");

    CoordinateXY p0{1, 2};
    CoordinateXY p1{2, 3};
    CoordinateXY p2{3, 4};

    checkEnvelope(p0, p1, p2,
                  1, 2, 3, 4);
}

template<>
template<>
void object::test<13>()
{
    set_test_name("envelope: arc defined by three repeated points");

    CoordinateXY p0{3, 4};
    CoordinateXY p1{3, 4};
    CoordinateXY p2{3, 4};

    checkEnvelope(p0, p1, p2,
                  3, 4, 3, 4);
}

template<>
template<>
void object::test<14>() {

    CoordinateXY p0{2, 0};
    CoordinateXY p1{4, 2};
    CoordinateXY p2{2, 1};

    checkEnvelope(p0, p1, p2,
    2, -1.0811388300841898, 5.08113883008419,2.08113883008419);
}

template<>
template<>
void object::test<15>()
{
    set_test_name("createArc");

    auto CCW = geos::algorithm::Orientation::COUNTERCLOCKWISE;
    auto CW = geos::algorithm::Orientation::CLOCKWISE;

    checkArc("CCW: upper half-circle", {0, 0}, 1, CCW, 0, MATH_PI, {1, 0}, {0, 1}, {-1, 0});
    checkArc("CCW: lower half-circle", {0, 0}, 1, CCW, MATH_PI, 0, {-1, 0}, {0, -1}, {1, 0});
    checkArc("CCW: left half-circle", {0, 0}, 1, CCW, MATH_PI/2, -MATH_PI/2, {0, 1}, {-1, 0}, {0, -1});
    checkArc("CCW: right half-circle", {0, 0}, 1, CCW, -MATH_PI/2, MATH_PI/2, {0, -1}, {1, 0}, {0, 1});

    checkArc("CW: upper half-circle", {0, 0}, 1, CW, MATH_PI, 0, {-1, 0}, {0, 1}, {1, 0});
    checkArc("CW: lower half-circle", {0, 0}, 1, CW, 0, MATH_PI, {1, 0}, {0, -1}, {-1, 0});
    checkArc("CW: left half-circle", {0, 0}, 1, CW, -MATH_PI/2, MATH_PI/2, {0, -1}, {-1, 0}, {0, 1});
    checkArc("CW: right half-circle", {0, 0}, 1, CW, MATH_PI/2, -MATH_PI/2, {0, 1}, {1, 0}, {0, -1});
}

#if 0
template<>
template<>
void object::test<16>()
{
    set_test_name("splitAtPoint");

    CircularArc cwArc(XY{-1, 0}, XY{0, 1}, XY{1, 0});
    auto [arc1, arc2] = cwArc.splitAtPoint({std::sqrt(2)/2, std::sqrt(2)/2});

    ensure_equals(arc1.p0(), CoordinateXY{-1, 0});
    ensure_equals(arc1.p2(), CoordinateXY{std::sqrt(2)/2, std::sqrt(2)/2});
    ensure_equals(arc1.getCenter(), cwArc.getCenter());
    ensure_equals(arc1.getRadius(), cwArc.getRadius());

    ensure_equals(arc2.p0(), CoordinateXY{std::sqrt(2)/2, std::sqrt(2)/2});
    ensure_equals(arc2.p2(), CoordinateXY{1, 0});
    ensure_equals(arc2.getCenter(), cwArc.getCenter());
    ensure_equals(arc2.getRadius(), cwArc.getRadius());

    ensure_equals(cwArc.getLength(), arc1.getLength() + arc2.getLength());
}
#endif

template<>
template<>
void object::test<17>() {
    set_test_name("getSagitta");

    CircularArc halfCircle = CircularArc::create(XY{-1, 0}, XY{0, 1}, XY{1, 0});
    ensure_equals(halfCircle.getSagitta(), 1);

    CircularArc quarterCircle = CircularArc::create(XY{0, 1}, XY{std::sqrt(2)/2, std::sqrt(2)/2}, {1, 0});
    ensure_equals(quarterCircle.getSagitta(),
        CoordinateXY{std::sqrt(2)/2, std::sqrt(2)/2}.distance(CoordinateXY{0.5, 0.5}));
}

template<>
template<>
void object::test<18>() {
    set_test_name("getDirectionPoint");

    const CircularArc arc1 = CircularArc::create(XY{-4, 3}, XY{0, -5}, XY{-4, -3});
    const auto dp1 = arc1.getDirectionPoint();
    ensure_equals(Quadrant::quadrant(arc1.p0(), dp1), 0); // NE
    ensure_equals("dp1.x", dp1.x, -1, 1e-5);
    ensure_equals("dp1.y", dp1.y, 7, 1e-5);
    ensure_equals("point is not expected distance from p0", arc1.p0().distance(dp1), arc1.getRadius(), 1e-5);


    const auto arc2 = arc1.reverse();
    const auto dp2 = arc2.getDirectionPoint();
    ensure_equals(Quadrant::quadrant(arc2.p0(), dp2), 3); // SE
    ensure_equals("dp2.x", dp2.x, -1, 1e-5);
    ensure_equals("dp2.y", dp2.y, -7, 1e-5);
    ensure_equals("point is not expected distance from p0", arc2.p0().distance(dp2), arc2.getRadius(), 1e-5);
}

template<>
template<>
void object::test<20>()
{
    set_test_name("pt-arc distance, point within unit semicircle");

    ensure_equals(unit_semi.distance(XY{0, 0.5}), 0.5);
}

template<>
template<>
void object::test<21>()
{
    set_test_name("pt-arc distance, point outside unit semicircle");

    ensure_equals(unit_semi.distance(XY{0, 1.5}), 0.5);
}

template<>
template<>
void object::test<22>()
{
    set_test_name("pt-arc distance, point outside unit semicircle, sqrt(2) units from endpoint");

    ensure_equals(unit_semi.distance(XY{0, -1}), std::sqrt(2));
}

template<>
template<>
void object::test<23>()
{
    set_test_name("pt-arc distance, point outside unit semicircle, sqrt(2)-1 units from endpoint");

    ensure_equals(unit_semi.distance(XY{1, 1}), std::sqrt(2)-1);
}

template<>
template<>
void object::test<24>()
{
    set_test_name("pt-arc distance, point on unit semicircle midpoint");

    ensure_equals(unit_semi.distance(XY{0, 1}), 0);
}

template<>
template<>
void object::test<25>()
{
    set_test_name("pt-arc distance, point on unit semicircle endpoint");

    ensure_equals(unit_semi.distance(XY{1, 0}), 0);
}

template<>
template<>
void object::test<26>()
{
    set_test_name("pt-arc distance, point on unit semicircle center");

    ensure_equals(unit_semi.distance(XY{0, 0}), 1);
}

template<>
template<>
void object::test<27>()
{
    set_test_name("pt-arc distance, point inside closed circle");

    ensure_equals(unit_semi.distance(XY{0, 0.5}), 0.5);
}

template<>
template<>
void object::test<28>()
{
    set_test_name("arc-seg distance, horizontal edge above unit semicircle");

    ensure_equals(unit_semi.distance(XY{-2, 2}, XY{2, 2}), 1.0);
}

template<>
template<>
void object::test<29>()
{
    set_test_name("arc-seg distance, vertical edge to the right of unit semicircle");

    ensure_equals(unit_semi.distance(XY{2, -2}, XY{2, 2}), 1.0);
}

template<>
template<>
void object::test<30>()
{
    set_test_name("arc-seg distance, vertical edge to the left of unit semicircle");

    ensure_equals(unit_semi.distance(XY{-2, -2}, XY{-2, 2}), 1.0);
}

template<>
template<>
void object::test<31>()
{
    set_test_name("arc-seg distance, vertical edge within unit semicircle");

    ensure_equals(unit_semi.distance(XY{0, 0}, XY{0, 0.5}), 0.5);
}

template<>
template<>
void object::test<32>()
{
    set_test_name("arc-seg distance, horizontal edge grazing unit semicircle");

    ensure_equals(unit_semi.distance(XY{-2, 1}, XY{2, 1}), 0);
}

template<>
template<>
void object::test<33>()
{
    set_test_name("arc-seg distance, horizontal edge upper-right of unit semicircle");

    ensure_equals(unit_semi.distance(XY{1, 1}, XY{2, 1}), std::sqrt(2) - 1);
}

template<>
template<>
void object::test<34>()
{
    set_test_name("arc-seg distance, edge intersecting unit semicircle");

    ensure_equals(unit_semi.distance(XY{0, 0}, XY{2, 2}), 0);
}

template<>
template<>
void object::test<35>()
{
    set_test_name("arc-seg distance, edge upper-left of unit semicircle");

    ensure_equals("distance is incorrect", unit_semi.distance(XY{-1, 1}, XY{-2, 2}), std::sqrt(2) - 1, 1e-6);
}

template<>
template<>
void object::test<36>()
{
    set_test_name("arc-arc distance, PostGIS ticket #4326");

    CircularArc a = CircularArc::create(
        XY{-1, 4},
        XY{0, 5},
        XY{1, 4});

    CircularArc b = CircularArc::create(
        XY{1, 6},
        XY{6, 1},
        XY{9, 7});

    ensure_equals("distance is incorrect", a.distance(b), 0.0475666, 1e-6);
}

template<>
template<>
void object::test<37>()
{
    set_test_name("arc-arc distance, arc inside unit semicircle");

    CircularArc a = CircularArc::create(
        XY{0, 0.5},
        XY{-0.3, 0.5},
        XY{0, 0.6});

    ensure_equals("distance is incorrect", unit_semi.distance(a), 0.271798, 1e-6);
}

template<>
template<>
void object::test<38>()
{
    set_test_name("arc-arc distance, arc inside unit semicircle (2)");

    CircularArc a = CircularArc::create(
        XY{-0.5, 0.5},
        XY{-0.4, 0.2},
        XY{0, 0});

    ensure_equals("distance is incorrect", unit_semi.distance(a), 0.292893, 1e-6);
}

template<>
template<>
void object::test<39>()
{
    set_test_name("arc-arc distance, arc above unit semicircle");

    CircularArc a = CircularArc::create(
        XY{-1, 3},
        XY{0, 2},
        XY{1, 3});

    ensure_equals("distance is incorrect", unit_semi.distance(a), 1, 1e-6);
}

template<>
template<>
void object::test<40>()
{
    set_test_name("arc-arc distance, arc grazes unit semicircle");

    CircularArc a = CircularArc::create(
        XY{-1, 2},
        XY{0, 1},
        XY{1, 2});

    ensure_equals("distance is incorrect", unit_semi.distance(a), 0, 1e-6);
}

template<>
template<>
void object::test<41>()
{
    set_test_name("arc-arc distance, circles intersect but arcs do not");

    CircularArc a = CircularArc::create(
        XY{-1, 1},
        XY{0, 2},
        XY{1, 1});

    ensure_equals("distance is incorrect", unit_semi.distance(a), std::sqrt(2) - 1, 1e-6);
}

template<>
template<>
void object::test<42>()
{
    set_test_name("arc-arc distance, circles and arcs intersect");

    CircularArc a = CircularArc::create(
        XY{-1, 1},
        XY{0, 0},
        XY{1, 1});

    ensure_equals(unit_semi.distance(a), 0);
}

template<>
template<>
void object::test<43>()
{
    set_test_name("arc-arc distance, closed circle");

    CircularArc a = CircularArc::create(
        XY{-2, -0.1},
        XY{1.5, -0.1},
        XY{-2, -0.1});

    ensure_equals("distance is incorrect", unit_semi.distance(a), 0.480742, 1e-6);
}

template<>
template<>
void object::test<44>()
{
    set_test_name("arc-arc distance, concentric and fully parallel");

    CircularArc a = CircularArc::create(
        XY{-2, 0},
        XY{0, 2},
        XY{2, 0});

    ensure_equals("distance is incorrect", unit_semi.distance(a), 1.0, 1e-6);
}

template<>
template<>
void object::test<45>()
{
    set_test_name("arc-arc distance, concentric with A fully included in unit semicircle's range");

    CircularArc a = CircularArc::create(
        XY{-0.5 / std::sqrt(2), 0.5 / std::sqrt(2)},
        XY{0, 0.5},
        XY{0.5 / std::sqrt(2), 0.5 / std::sqrt(2)});

    ensure_equals("distance is incorrect", unit_semi.distance(a), 0.5, 1e-6);
}

template<>
template<>
void object::test<46>()
{
    set_test_name("arc-arc distance, concentric with A partially included in unit semicircle's range");

    CircularArc a = CircularArc::create(
        XY{-0.5 / std::sqrt(2), -0.5 / std::sqrt(2)},
        XY{-0.5, 0},
        XY{-0.5 / std::sqrt(2), 0.5 / std::sqrt(2)});

    ensure_equals("distance is incorrect", unit_semi.distance(a), 0.5, 1e-6);
}

template<>
template<>
void object::test<47>()
{
    set_test_name("arc-arc distance, concentric without parallel segments");

    CircularArc a = CircularArc::create(
        XY{-0.5 / std::sqrt(2), -0.5 / std::sqrt(2)},
        XY{0, -0.5},
        XY{0.5 / std::sqrt(2), -0.5 / std::sqrt(2)});

    ensure_equals("distance is incorrect", unit_semi.distance(a), 0.736813, 1e-6);
}

template<>
template<>
void object::test<48>()
{
    set_test_name("arc-arc distance, arcs are the same");

    ensure_equals(unit_semi.distance(unit_semi), 0);
}

template<>
template<>
void object::test<49>()
{
    set_test_name("arc-arc distance, different orientations (1)");

    CircularArc a = CircularArc::create(XY{-22, 0}, XY{-17, -5}, XY{-12, 0});
    CircularArc b = CircularArc::create(XY{-10, 0}, XY{0, 10}, XY{10, 0});

    ensure_equals("distance is incorrect", a.distance(b), 2.0, 1e-6);
}

template<>
template<>
void object::test<50>()
{
    set_test_name("arc-arc distance, different orientations (2)");

    CircularArc a = CircularArc::create(XY{-19, 0}, XY{-14, -5}, XY{-9, 0});
    CircularArc b = CircularArc::create(XY{-10, 0}, XY{0, 10}, XY{10, 0});

    ensure_equals("distance is incorrect", a.distance(b), 1.0, 1e-6);
}

template<>
template<>
void object::test<51>()
{
    set_test_name("arc-arc distance, different orientations (3)");

    CircularArc a = CircularArc::create(XY{-9, 0}, XY{-4, -5}, XY{1, 0});
    CircularArc b = CircularArc::create(XY{-10, 0}, XY{0, 10}, XY{10, 0});

    ensure_equals("distance is incorrect", a.distance(b), 1.0, 1e-6);
}

template<>
template<>
void object::test<52>()
{
    set_test_name("arc-arc distance, different orientations (4)");

    CircularArc a = CircularArc::create(XY{-1, 0}, XY{4, -5}, XY{9, 0});
    CircularArc b = CircularArc::create(XY{-10, 0}, XY{0, 10}, XY{10, 0});

    ensure_equals("distance is incorrect", a.distance(b), 1.0, 1e-6);
}

template<>
template<>
void object::test<53>()
{
    set_test_name("arc-arc distance, different orientations (5)");

    CircularArc a = CircularArc::create(XY{1, 0}, XY{6, -5}, XY{11, 0});
    CircularArc b = CircularArc::create(XY{-10, 0}, XY{0, 10}, XY{10, 0});

    ensure_equals("distance is incorrect", a.distance(b), 1.0, 1e-6);
}

template<>
template<>
void object::test<54>()
{
    set_test_name("arc-arc distance, different orientations (6)");

    CircularArc a = CircularArc::create(XY{11, 0}, XY{16, -5}, XY{21, 0});
    CircularArc b = CircularArc::create(XY{-10, 0}, XY{0, 10}, XY{10, 0});

    ensure_equals("distance is incorrect", a.distance(b), 1.0, 1e-6);
}

template<>
template<>
void object::test<55>()
{
    set_test_name("arc-arc distance, different orientations (7)");

    CircularArc a = CircularArc::create(XY{-15, -6}, XY{-10, -1}, XY{-5, -6});
    CircularArc b = CircularArc::create(XY{-10, 0}, XY{0, 10}, XY{10, 0});

    ensure_equals("distance is incorrect", a.distance(b), 1.0, 1e-6);
}

template<>
template<>
void object::test<56>()
{
    set_test_name("arc-arc distance, different orientations (8)");

    CircularArc a = CircularArc::create(XY{-5, 0}, XY{0, 5}, XY{5, 0});
    CircularArc b = CircularArc::create(XY{-10, 0}, XY{0, 10}, XY{10, 0});

    ensure_equals("distance is incorrect", a.distance(b), 5.0, 1e-6);
}

template<>
template<>
void object::test<57>()
{
    set_test_name("arc-arc distance, different orientations (9)");

    CircularArc a = CircularArc::create(XY{-5, 0}, XY{0, -5}, XY{5, 0});
    CircularArc b = CircularArc::create(XY{-10, 0}, XY{0, 10}, XY{10, 0});

    ensure_equals("distance is incorrect", a.distance(b), 5.0, 1e-6);
}

template<>
template<>
void object::test<58>()
{
    set_test_name("arc-seg distance, closest point is segment interior");

    XY p0{1, -1};
    XY p1{2, 0};

    ensure_equals("distance is incorrect", unit_semi.distance(p0, p1), std::sqrt(2)/2, 1e-6);
}

template<>
template<>
void object::test<59>()
{
    set_test_name("arc-seg distance, closest distance is between segment interior and arc interior");

    CircularArc a = CircularArc::create(XY{-5, 0}, XY{0, 5}, XY{4, -3});

    XY p0{4, -5};
    XY p1{7, 0};

    ensure_equals("distance is incorrect", a.distance(p0, p1), 1.0024502, 1e-6);
}

template<>
template<>
void object::test<60>()
{
    set_test_name("arc-seg distance (vertical segment tangent to arc)");

    CircularArc a = CircularArc::create(XY{5, 1}, XY{5, 3}, XY{5, 2}, 1, geos::algorithm::Orientation::CLOCKWISE);

    XY q0{4, 4};
    XY q1{4, 0};

    ensure_equals("distance is incorrect", a.distance(q0, q1), 0);
}

template<>
template<>
void object::test<61>()
{
    set_test_name("intersection point: arcs from disjoint circles");

    CircularArc a = CircularArc::create(XY{-1, 0}, XY{0, 1}, XY{1, 0});
    CircularArc b = CircularArc::create(XY{5, 0}, XY{6, 1}, XY{7, 0});

    checkArcDisjoint(a, b);
}

template<>
template<>
void object::test<62>()
{
    set_test_name("intersection point: arcs from contained circles");

    CircularArc a = CircularArc::create(XY{-1, 0}, XY{0, 1}, XY{1, 0});
    CircularArc b = CircularArc::create(XY{-5, 0}, XY{0, 5}, XY{5, 0});

    checkArcDisjoint(a, b);
}

template<>
template<>
void object::test<63>()
{
    set_test_name("intersection point: arcs with endpoint intersections");

    CircularArc a = CircularArc::create(XY{-1, 0}, XY{0, 1}, XY{1, 0});
    CircularArc b = CircularArc::create(XY{1, 0}, XY{2, -1}, XY{3, 0});
    CircularArc c = CircularArc::create(XY{3, 0}, XY{4, 1}, XY{5, 0});

    checkArcIntersectionPoint(a, b, XY{1, 0});
    checkArcIntersectionPoint(b, a, XY{1, 0});

    checkArcIntersectionPoint(b, c, XY{3, 0});
    checkArcIntersectionPoint(c, b, XY{3, 0});

    checkArcDisjoint(a, c);
}

template<>
template<>
void object::test<64>()
{
    set_test_name("intersection point: cocircular arcs with endpoint intersection");

    CircularArc a = CircularArc::create(XY{-5, 0}, XY{0, 5}, XY{5, 0});
    CircularArc b = CircularArc::create(XY{-5, 0}, XY{0, -5}, XY{ 4, -3});

    checkArcIntersectionPoint(a, b, XY{-5, 0}, 1e-8);
    checkArcIntersectionPoint(b, a, XY{-5, 0}, 1e-8);
}

template<>
template<>
void object::test<65>()
{
    set_test_name("intersection point: cocircular arcs with overlap");

    CircularArc a = CircularArc::create(XY{-5, 0}, XY{0, 5}, XY{5, 0});
    CircularArc b = CircularArc::create({0, -5}, XY{ 4, -3}, XY{4, 3});

    checkArcIntersectionPoint(a, b, XY{4, 3}, 1e-8);
    checkArcIntersectionPoint(b, a, XY{5, 0}, 1e-8);
}

template<>
template<>
void object::test<66>()
{
    set_test_name("intersection point: disjoint cocircular arcs");

    CircularArc a = CircularArc::create(XY{-4, 3}, XY{0, 5}, XY{4, 3});
    CircularArc b = CircularArc::create({-5, 0}, XY{ 0, -5}, XY{5, 0});

    checkArcDisjoint(a, b);
    checkArcDisjoint(b, a);
}

template<>
template<>
void object::test<67>()
{
    set_test_name("closest points: two collapsed arcs");

    CircularArc a = CircularArc::create(XY{-10, 0}, XY{-9, 0}, XY{-8, 0}); // horizontal line
    CircularArc b = CircularArc::create({0, -5}, XY{ 0, 0}, XY{0, 5}); // vertical line

    auto pts = a.closestPoints(b);
    ensure_equals_xy(pts[0], XY{-8, 0});
    ensure_equals_xy(pts[1], XY{0, 0});
}

template<>
template<>
void object::test<68>()
{
    set_test_name("closest points: arc and collapsed arc");

    CircularArc a = CircularArc::create(XY{-10, -10}, XY{-9, -10}, XY{10, -10}); // horizontal line
    CircularArc b = CircularArc::create({-5, 5}, XY{ -4, 2}, XY{5, 5});

    auto pts = a.closestPoints(b);
    ensure_equals_xy(pts[0], XY{0, -10});
    ensure_equals_xy(pts[1], XY{0, 0});

    pts = b.closestPoints(a);
    ensure_equals_xy(pts[0], XY{0, 0});
    ensure_equals_xy(pts[1], XY{0, -10});
}

template<>
template<>
void object::test<69>()
{
    set_test_name("distance: overlapping cocircular arcs");

    CircularArc a = CircularArc::create(XY{-5, 0}, XY{0, 5}, XY{5, 0});
    CircularArc b = CircularArc::create({-4, 3}, XY{0, 5}, XY{ 4, 3});

    ensure_equals(a.distance(b), 0);
    ensure_equals(b.distance(a), 0);
}

template<>
template<>
void object::test<70>()
{
set_test_name("getMidpointAngle");

    // half-circle
    ensure_equals(CircularArcs::getMidpointAngle(0, MATH_PI, true), MATH_PI/2);
    ensure_equals(CircularArcs::getMidpointAngle(0, MATH_PI, false), 3*MATH_PI/2);

    // full circle
    ensure_equals(CircularArcs::getMidpointAngle(0, 0, true), MATH_PI);
    ensure_equals(CircularArcs::getMidpointAngle(0, 0, false), MATH_PI);
}

}
