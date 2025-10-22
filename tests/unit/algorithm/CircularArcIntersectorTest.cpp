#include <tut/tut.hpp>

#include <geos/algorithm/CircularArcIntersector.h>
#include <geos/geom/LineSegment.h>
#include <geos/constants.h>
#include <variant>

using geos::algorithm::CircularArcIntersector;
using geos::algorithm::Orientation;
using geos::geom::CoordinateXY;
using geos::geom::CircularArc;
using geos::MATH_PI;

namespace tut {

struct test_circulararcintersector_data {

    using ArcOrPoint = std::variant<CoordinateXY, CircularArc>;

    static std::string to_string(CircularArcIntersector::intersection_type t)
    {
        switch (t) {
        case geos::algorithm::CircularArcIntersector::NO_INTERSECTION:
            return "no intersection";
        case geos::algorithm::CircularArcIntersector::ONE_POINT_INTERSECTION:
            return "one-point intersection";
        case geos::algorithm::CircularArcIntersector::TWO_POINT_INTERSECTION:
            return "two-point intersection";
        case geos::algorithm::CircularArcIntersector::COCIRCULAR_INTERSECTION:
            return "cocircular intersection";
            break;
        }

        return "";
    }

    static std::string toWKT(const CoordinateXY& pt)
    {
        return "POINT (" + pt.toString() + ")";
    }

    static std::string toWKT(const CircularArc& arc)
    {
        return "CIRCULARSTRING (" + arc.p0.toString() + ", " + arc.p1.toString() + ", " + arc.p2.toString() + ")";
    }

    static std::string toWKT(const geos::geom::LineSegment& seg)
    {
        return "LINESTRING (" + seg.p0.toString() + ", " + seg.p1.toString() + ")";
    }

    static void checkIntersection(CoordinateXY p0, CoordinateXY p1, CoordinateXY p2,
                                  CoordinateXY q0, CoordinateXY q1, CoordinateXY q2,
                                  CircularArcIntersector::intersection_type result,
                                  ArcOrPoint i0 = CoordinateXY::getNull(),
                                  ArcOrPoint i1 = CoordinateXY::getNull())
    {
        CircularArc a0(p0, p1, p2);
        CircularArc a1(q0, q1, q2);

        checkIntersection(a0, a1, result, i0, i1);
    }

    static void checkIntersection(CoordinateXY p0, CoordinateXY p1, CoordinateXY p2,
                                  CoordinateXY q0, CoordinateXY q1,
                                  CircularArcIntersector::intersection_type result,
                                  CoordinateXY i0 = CoordinateXY::getNull(),
                                  CoordinateXY i1 = CoordinateXY::getNull())
    {
        CircularArc a(p0, p1, p2);
        geos::geom::LineSegment s(geos::geom::Coordinate{q0}, geos::geom::Coordinate{q1});

        checkIntersection(a, s, result, i0, i1);
    }

    template<typename CircularArcOrLineSegment>
    static void checkIntersection(const CircularArc& a0,
                                  const CircularArcOrLineSegment& a1,
                                  CircularArcIntersector::intersection_type result,
                                  ArcOrPoint p0 = CoordinateXY::getNull(),
                                  ArcOrPoint p1 = CoordinateXY::getNull())
    {
        CircularArcIntersector cai;
        cai.intersects(a0, a1);

        ensure_equals("incorrect intersection type between " + toWKT(a0) + " and " + toWKT(a1), to_string(cai.getResult()), to_string(result));

        std::vector<CoordinateXY> expectedPoints;
        std::vector<CircularArc> expectedArcs;

        for (const auto& intersection : { p0, p1 }) {
            if (std::holds_alternative<CoordinateXY>(intersection)) {
                const CoordinateXY& pt = std::get<CoordinateXY>(intersection);
                if (!pt.isNull()) {
                    expectedPoints.push_back(pt);
                }
            }
            else {
                expectedArcs.push_back(std::get<CircularArc>(intersection));
            }
        }

        std::vector<CoordinateXY> actualPoints;
        std::vector<CircularArc> actualArcs;

        for (std::uint8_t i = 0; i < cai.getNumPoints(); i++) {
            actualPoints.push_back(cai.getPoint(i));
        }

        for (std::uint8_t i = 0; i < cai.getNumArcs(); i++) {
            actualArcs.push_back(cai.getArc(i));
        }

        auto compareArcs = [](const CircularArc& a, const CircularArc& b) {
            int cmp;
            cmp = a.p0.compareTo(b.p0);
            if (cmp != 0) {
                return cmp == -1;
            }
            cmp = a.p2.compareTo(b.p2);
            if (cmp != 0) {
                return cmp == -1;
            }
            cmp = a.getCenter().compareTo(b.getCenter());
            if (cmp != 0) {
                return cmp == -1;
            }
            return a.getOrientation() < b.getOrientation();
        };

        std::sort(actualPoints.begin(), actualPoints.end());
        std::sort(actualArcs.begin(), actualArcs.end(), compareArcs);
        std::sort(expectedPoints.begin(), expectedPoints.end());
        std::sort(expectedArcs.begin(), expectedArcs.end(), compareArcs);

        bool equal = true;
        if (actualPoints.size() != expectedPoints.size()) {
            equal = false;
        }
        if (actualArcs.size() != expectedArcs.size()) {
            equal = false;
        }

        constexpr double eps = 1e-8;

        if (equal) {
            for (std::size_t i = 0; i < actualPoints.size(); i++) {
                if (actualPoints[i].distance(expectedPoints[i]) > eps) {
                    equal = false;
                }
            }
            for (std::size_t i = 0; i < actualArcs.size(); i++) {
                if (actualArcs[i].getOrientation() != expectedArcs[i].getOrientation()) {
                    equal = false;
                }

                if (std::abs(actualArcs[i].getRadius() - expectedArcs[i].getRadius()) > eps) {
                    equal = false;
                }

                if (actualArcs[i].getCenter().distance(expectedArcs[i].getCenter()) > eps) {
                    equal = false;
                }

                if (actualArcs[i].p0.distance(expectedArcs[i].p0) > eps) {
                    equal = false;
                }

                if (actualArcs[i].p2.distance(expectedArcs[i].p2) > eps) {
                    equal = false;
                }
            }
        }

        if (equal) {
            return;
        }

        std::string actual;
        for (const auto& pt : actualPoints) {
            if (!actual.empty()) {
                actual += ", ";
            }
            actual += toWKT(pt);
        }
        for (const auto& arc : actualArcs) {
            if (!actual.empty()) {
                actual += ", ";
            }
            actual += toWKT(arc);
        }

        std::string expected;
        for (const auto& pt : expectedPoints) {
            if (!expected.empty()) {
                expected += ", ";
            }
            expected += toWKT(pt);
        }
        for (const auto& arc : expectedArcs) {
            if (!expected.empty()) {
                expected += ", ";
            }
            expected += toWKT(arc);
        }

        ensure_equals(actual, expected);
    }

    const CoordinateXY _NW = { -std::sqrt(2)/2, std::sqrt(2)/2 };
    const CoordinateXY _N =  { 0, 1};
    const CoordinateXY _NE = {  std::sqrt(2)/2, std::sqrt(2)/2 };
    const CoordinateXY _E =  { 1, 0};
    const CoordinateXY _SE = {  std::sqrt(2)/2, -std::sqrt(2)/2 };
    const CoordinateXY _S =  { 0, -1};
    const CoordinateXY _SW = { -std::sqrt(2)/2, -std::sqrt(2)/2 };
    const CoordinateXY _W =  { -1, 0};
};

using group = test_group<test_circulararcintersector_data>;
using object = group::object;

group test_circulararcintersector_group("geos::algorithm::CircularArcIntersector");

template<>
template<>
void object::test<1>()
{
    set_test_name("interior/interior intersection (one point)");

    checkIntersection({0, 0}, {1, std::sqrt(3)}, {2, 2},
    {0, 2}, {1, std::sqrt(3)}, {2, 0},
    CircularArcIntersector::ONE_POINT_INTERSECTION,
    CoordinateXY{1, std::sqrt(3)});
}

template<>
template<>
void object::test<2>()
{
    set_test_name("interior/interior intersection (two points)");

    // result from CGAL 5.4
    checkIntersection({0, 0}, {2, 2}, {4, 0},
    {0, 1}, {2, -1}, {4, 1},
    CircularArcIntersector::TWO_POINT_INTERSECTION,
    CoordinateXY{0.0635083268962914893, 0.5},
    CoordinateXY{3.93649167310370851, 0.5});
}

template<>
template<>
void object::test<3>()
{
    set_test_name("single endpoint-endpoint intersection");

    checkIntersection({0, 0}, {1, 1}, {2, 0},
    {2, 0}, {3, -1}, {4, 0},
    CircularArcIntersector::ONE_POINT_INTERSECTION,
    CoordinateXY{2, 0});
}

template<>
template<>
void object::test<4>()
{
    set_test_name("single interior-interior intersection at point of tangency");

    checkIntersection({0, 0}, {1, 1}, {2, 0},
    {0, 2}, {1, 1}, {2, 2},
    CircularArcIntersector::ONE_POINT_INTERSECTION,
    CoordinateXY{1, 1});
}

template<>
template<>
void object::test<5>()
{
    set_test_name("supporting circles intersect but arcs do not");

    checkIntersection({0, 0}, {2, 2}, {4, 0},
    {1, 1}, {0, -1}, {-1, 1},
    CircularArcIntersector::NO_INTERSECTION);

}

template<>
template<>
void object::test<6>()
{
    set_test_name("one circle contained within other");

    checkIntersection({0, 0}, {4, 4}, {8, 0},
    {2, 0}, {4, 2}, {6, 0},
    CircularArcIntersector::NO_INTERSECTION);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("cocircular with double endpoint intersection");

    checkIntersection({0, 0}, {1, 1}, {2, 0},
    {0, 0}, {1, -1}, {2, 0},
    CircularArcIntersector::TWO_POINT_INTERSECTION,
    CoordinateXY{0, 0}, CoordinateXY{2, 0});
}

template<>
template<>
void object::test<8>()
{
    set_test_name("cocircular with single endpoint intersection");

    checkIntersection({-2, 0}, {0, 2}, {2, 0},
    {0, -2}, {std::sqrt(2), -std::sqrt(2)}, {2, 0},
    CircularArcIntersector::ONE_POINT_INTERSECTION,
    CoordinateXY{2, 0});
}

template<>
template<>
void object::test<9>()
{
    set_test_name("cocircular disjoint");

    checkIntersection(_NW, _N, _NE,
                      _SW, _S, _SE,
                      CircularArcIntersector::NO_INTERSECTION);
}

template<>
template<>
void object::test<10>()
{
    set_test_name("cocircular with single arc intersection (clockwise)");

    checkIntersection({-5, 0}, {0, 5}, {5, 0}, // CW
    {-4, 3}, {0, 5}, {4, 3}, // CW
    CircularArcIntersector::COCIRCULAR_INTERSECTION,
    CircularArc{{-4, 3}, {0, 5}, {4, 3}}); // CW
}

template<>
template<>
void object::test<11>()
{
    set_test_name("cocircular with single arc intersection (counter-clockwise)");

    checkIntersection({5, 0},  {0, 5}, {-5, 0}, // CCW
    {-4, 3}, {0, 5}, {4, 3},  // CW
    CircularArcIntersector::COCIRCULAR_INTERSECTION,
    CircularArc{{4, 3}, {0, 5}, {-4, 3}}); // CCW
}

template<>
template<>
void object::test<12>()
{
    set_test_name("cocircular with arc and point intersections");

    checkIntersection({-5, 0}, {0, 5}, {5, 0},
    {5, 0}, {0, -5}, {0, 5},
    CircularArcIntersector::COCIRCULAR_INTERSECTION,
    CircularArc{{-5, 0}, {-5*std::sqrt(2)/2, 5*std::sqrt(2)/2}, {0, 5}},
    CoordinateXY{5, 0});
}

template<>
template<>
void object::test<13>()
{
    set_test_name("cocircular with two arc intersections");

    checkIntersection({-5, 0}, {0, 5}, {5, 0},
    {3, 4}, {0, -5}, {-3, 4},
    CircularArcIntersector::COCIRCULAR_INTERSECTION,
    CircularArc{{3, 4}, {4.4721359549995796, 2.2360679774997898}, {5, 0}},
    CircularArc{{-5, 0}, {-4.4721359549995796, 2.2360679774997907}, {-3, 4}});
}

template<>
template<>
void object::test<20>()
{
    set_test_name("arc - degenerate arc with single interior intersection");

    checkIntersection({0, 0}, {2, 2}, {4, 0}, // CW arc
                      {-1, -4}, {1, 0}, {3, 4}, // degenerate arc
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      CoordinateXY{2, 2});

    checkIntersection({-1, -4}, {1, 0}, {3, 4}, // degenerate arc
                      {0, 0}, {2, 2}, {4, 0}, // CW arc
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      CoordinateXY{2, 2});
}

template<>
template<>
void object::test<21>()
{
    set_test_name("two degenerate arcs with single interior intersection");

    checkIntersection({0, 0}, {4, 4}, {10, 10},
                      {10, 0}, {1, 9}, {0, 10},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      CoordinateXY{5, 5});
}

template<>
template<>
void object::test<30>()
{
    set_test_name("arc-segment with single interior intersection");

    checkIntersection({0, 0}, {2, 2}, {4, 0},
    {1, 0}, {3, 4},
    CircularArcIntersector::ONE_POINT_INTERSECTION,
    {2, 2});
}

template<>
template<>
void object::test<31>()
{
    set_test_name("arc-vertical segment with single interior intersection");

    checkIntersection({-2, 0}, {0, 2}, {2, 0},
    {0, 0}, {0, 4},
    CircularArcIntersector::ONE_POINT_INTERSECTION,
    {0, 2});
}

template<>
template<>
void object::test<32>()
{
    set_test_name("arc-segment with two interior intersections");

    checkIntersection(_W, _E, _SW,
    {-10, 10}, {10, -10},
    CircularArcIntersector::TWO_POINT_INTERSECTION,
    _NW, _SE);
}

template<>
template<>
void object::test<33>()
{
    set_test_name("arc-vertical segment with two interior intersections");

    checkIntersection(_W, _E, _SW,
    {0, -2}, {0, 2},
    CircularArcIntersector::TWO_POINT_INTERSECTION,
    _S, _N);
}

template<>
template<>
void object::test<34>()
{
    set_test_name("arc-segment disjoint with bbox containment");

    checkIntersection(_W, _N, _E,
    {0, 0}, {0.2, 0.2},
    CircularArcIntersector::NO_INTERSECTION);
}

template<>
template<>
void object::test<35>()
{
    set_test_name("degenerate arc-segment with interior intersection");

    checkIntersection({-5, -5}, {0, 0}, {5, 5},
                      {-5, 5},  {5, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      {0, 0});
}

template<>
template<>
void object::test<36>()
{
    set_test_name("intersection between a segment and a degenerate arc (radius = Infinity)");

    checkIntersection({-5, -5}, {0, 0}, {5, 5 + 1e-14},
                      {-5, 5}, {5, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      CoordinateXY{0, 0});
}

template<>
template<>
void object::test<37>()
{
    set_test_name("intersection between a segment and a nearly-degenerate arc (radius ~= 1e5)");

    checkIntersection({-5, -5}, {0, 0}, {5, 5 + 1e-4},
                      {-5, 5}, {5, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      CoordinateXY{0, 0});
}

template<>
template<>
void object::test<38>()
{
    set_test_name("arc-segment tests from ILI validator");
    // https://github.com/claeis/iox-ili/blob/master/jtsext/src/test/java/ch/interlis/iom_j/itf/impl/hrg/ISCILRTest.java

    // test_1a
    checkIntersection({0, 5}, {5, 0}, {0, -5},
                      {20, 5}, {20, -5},
                      CircularArcIntersector::NO_INTERSECTION),

    // test_2a
    checkIntersection({0, 5}, {5, 0}, {0, -5},
                      {5, 5}, {5, 0},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      {5, 0});

    // test_2b
    checkIntersection({0, 5}, {5, 0}, {0, -5},
                      {5, 5}, {5, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      {5, 0});

    // test_2c
    checkIntersection({0, 5}, {4, 3}, {0, -5},
                      {5, 5}, {5, 0},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      {5, 0});

    // test_2d
    checkIntersection({0, 5}, {4, 3}, {0, -5},
                      {5, 5}, {5, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      {5, 0});

    // test_3a
    checkIntersection({0, 5}, {5, 0}, {0, -5},
                      {4, 5}, {4, -5},
                      CircularArcIntersector::TWO_POINT_INTERSECTION,
                      {4, 3}, {4, -3});

    // test_3b
    checkIntersection({0, 5}, {5, 0}, {0, -5},
                      {-4, 5}, {-4, -5},
                      CircularArcIntersector::NO_INTERSECTION);

    // test_3c
    checkIntersection({0, 5}, {5, 0}, {0, -5},
                      {4, 10}, {4, 5},
                      CircularArcIntersector::NO_INTERSECTION);


    // test_3d
    checkIntersection({0, 5}, {3, 4}, {5, 0},
                      {4, 5}, {4, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      {4, 3});

    // test_3e
    checkIntersection({0, 5}, {5, 0}, {0, -5},
                      {4, 5}, {4, 0},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      {4, 3});
}

template<>
template<>
void object::test<39>()
{
    set_test_name("arc-arc tests from ILI validator");
    // https://github.com/claeis/iox-ili/blob/master/jtsext/src/test/java/ch/interlis/iom_j/itf/impl/hrg/ISCICRTest.java

    // test_1: circles do not overlap
    checkIntersection({0, 5}, {5, 0}, {0, -5},
                      {20, 5}, {15, 0}, {20, -5},
                      CircularArcIntersector::NO_INTERSECTION);

    // test_2a: arcs overlap at a point
    checkIntersection({0, 5}, {5, 0}, {0, -5},
                      {10, 5}, {5, 0}, {10, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      CoordinateXY{5, 0});
    // test_2b: arcs overlap at a point that is not a definition point of either arc
    checkIntersection({0, 5}, {4, 3}, {0, -5},
                      {10, 5}, {6, 3}, {10, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      CoordinateXY{5, 0});

    // test_3a: circles overlap at two points that are within both arcs
    checkIntersection({0, 5}, {5, 0}, {0, -5},
                      {8, 5}, {3, 0}, {8, -5},
                      CircularArcIntersector::TWO_POINT_INTERSECTION,
                      CoordinateXY{4, 3}, CoordinateXY{4, -3});

    // test_3b: circles overlap at two points but neither is on the first arc
    checkIntersection({0, 5}, {-5, 0}, {0, -5},
                      {8, 5}, {3, 0}, {8, -5},
                      CircularArcIntersector::NO_INTERSECTION);

    // test_3c: circles overlap at two points but neither is on the first or second arc
    checkIntersection({0, 5}, {-5, 0}, {0, -5},
                      {8, 5}, {13, 0}, {8, -5},
                      CircularArcIntersector::NO_INTERSECTION);

    // test_3d: circles overlap at two points but one is not on the first arc
    checkIntersection({5, 0}, {3, -4}, {0, -5},
                      {8, 5}, {3, 0}, {8, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      CoordinateXY{4, -3});

    // test_3e: circles overlap at two points but one is not on the second arc
    checkIntersection({0, 5}, {5, 0}, {0, -5},
                      {3, 0}, {5, -4}, {8, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      CoordinateXY{4, -3});

    // test_4a: cocircular
    checkIntersection({0, 5}, {5, 0}, {0, -5},
                      {4, 3}, {5, 0}, {4, -3},
                      CircularArcIntersector::COCIRCULAR_INTERSECTION,
                      CircularArc{{4, 3}, {5, 0}, {4, -3}});
}

// failed assertion: Values are not equal: expected `POINT (0 0)` actual `POINT (-5.4568517953157425e-06 5.4568517953157425e-06)`
template<>
template<>
void object::test<40>()
{
    set_test_name("intersection between a segment and a nearly-degenerate arc (radius ~= 2e6)");

    checkIntersection({-5, -5}, {0, 0}, {5, 5 + 1e-9},
                      {-5, 5}, {5, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      CoordinateXY{0, 0});
}

template<>
template<>
void object::test<41>()
{
    set_test_name("IOX-ILI: testFastGerade");

    checkIntersection({611770.424, 234251.322}, {611770.171, 234250.059}, {611769.918, 234248.796},
    {611613.84, 233467.819},
    {611610.392, 233468.995},
    CircularArcIntersector::NO_INTERSECTION);
}

template<>
template<>
void object::test<42>()
{
    set_test_name("IOX-ILI: testCircleCircleEndptTolerance");
    // two nearly-linear arcs touching at a single endpoint

    checkIntersection({645175.553, 248745.374}, { 645092.332, 248711.677}, { 645009.11, 248677.98},
                      {645009.11, 248677.98}, {644926.69, 248644.616}, { 644844.269, 248611.253},
                       CircularArcIntersector::ONE_POINT_INTERSECTION,
                      CoordinateXY{645009.110, 248677.980});
}

template<>
template<>
void object::test<43>()
{
    set_test_name("IOX-ILI: overlayTwoARCS_SameEndPoints_SameDirection");
    // two arcs with same arcPoint and radius.
    // startPoints and endPoints are same. lines are in same direction

    checkIntersection(
    {100.0, 100.0},{120,150.0},{100.0,200.0},
    {100.0, 100.0},{120,150.0},{100.0,200.0},
    CircularArcIntersector::COCIRCULAR_INTERSECTION,
    CircularArc{{100.0, 100.0}, {120, 150}, {100, 200}});
}

template<>
template<>
void object::test<44>()
{
    set_test_name("IOX-ILI: overlayTwoARCS_DifferentArcPointOnSameArcLine_SameDirection");
	// two arcs with different arcPoint (on same arcLine) and same radius length.
	// startPoints and endPoints are same. lines are in same direction.

    checkIntersection(
    {0.0, 10.0},{4.0,8.0},{0.0,0.0},
    {0.0, 10.0},{4.0,2.0},{0.0,0.0},
    CircularArcIntersector::COCIRCULAR_INTERSECTION,
    CircularArc{{0, 10}, {5, 5}, {0, 0}});
}

template<>
template<>
void object::test<45>()
{
    set_test_name("IOX-ILI: overlayTwoARCS_SameArcPointOnSameArcLine_OneArcLineIsLonger");
    // two arcs with same arcPoint (on same arcLine) and same radius length.
    // one arc line is longer than the other arc line.
    // startPoints is same, endPoints are different. lines are in same direction.

    checkIntersection(
    {0.0, 10.0},{4.0,8.0},{0.0,0.0},
    {0.0, 10.0},{4.0,8.0},{4.0,2.0},
    CircularArcIntersector::COCIRCULAR_INTERSECTION,
    CircularArc{{0, 10}, {4, 2}, {0, 5}, 5, Orientation::CLOCKWISE});
}

template<>
template<>
void object::test<46>() 
{
    set_test_name("IOX-ILI: overlayTwoARCS_SameEndPoints_OtherDirection");
    // two arcs with same arcPoint and radius
    // startPoint1 is equal to endPoint2, startPoint2 is equal to endPoint1.

    checkIntersection(
        CircularArc({100.0, 100.0}, {80.0, 150.0}, {100.0, 200.0}),
        CircularArc({100.0, 200.0}, {80.0, 150.0}, {100.0, 100.0}),
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc({100.0, 100.0}, {80.0, 150.0}, {100.0, 200.0}));
}

template<>
template<>
void object::test<47>() 
{
    set_test_name("IOX-ILI: overlayTwoARCS_DifferentStartPoints_SameDirection_DifferentLength");
    // two arcs. ArcPoint is equal. different angle.
    // startPoints are different. endPoints are same.
    CircularArc a({70.0, 60.0}, {50.0, 100.0}, {60.0, 130.0});
    CircularArc b({60.0, 70.0}, {50.0, 100.0}, {60.0, 130.0});

    checkIntersection(a, b,
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc({60, 70}, {60, 130}, a.getCenter(), a.getRadius(), a.getOrientation()));
}

template<>
template<>
void object::test<48>()
{
    set_test_name("IOX-ILI: overlayTwoARCS_DifferentStartEndPoints_OtherDirection_DifferentLength");
    // Two cocircular arcs with opposite orientation.
    // ArcPoint is equal.
    // startPoints are different. endPoints are different.

    CircularArc a({70.0,  60.0}, {50.0, 100.0}, {70.0, 140.0});
    CircularArc b({60.0, 130.0}, {50.0, 100.0}, {60.0,  70.0});

    checkIntersection(a, b, 
            CircularArcIntersector::COCIRCULAR_INTERSECTION,
            CircularArc({60, 70}, {60, 130}, a.getCenter(), a.getRadius(), a.getOrientation()));

}

template<>
template<>
void object::test<49>()
{
    set_test_name("IOX-ILI: overlayTwoARCS_DifferentEndPoints_SameDirection_DifferentLength");
    // Two arcs with same orientation. 
    // ArcPoint is equal.
    // startPoints are same, endpoints are different

    CircularArc a({70.0, 60.0}, {50.0, 100.0}, {70.0, 140.0});
    CircularArc b({70.0, 60.0}, {50.0, 100.0}, {60.0, 130.0});

    checkIntersection(a, b,
        CircularArcIntersector::COCIRCULAR_INTERSECTION, b);
}

template<>
template<>
void object::test<50>()
{
    set_test_name("IOX-ILI: overlayTwoARCS_DifferentEndPoints_OtherDirection_DifferentLength");
    // Two arcs with opposite orientation.
    // ArcPoint is equal.
    // One endpoint is the same, one is different.


    CircularArc a({70.0,  60.0}, {50.0, 100.0}, {70.0, 140.0});
    CircularArc b({60.0, 130.0}, {50.0, 100.0}, {70.0,  60.0});

    checkIntersection(a, b,
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc({70, 60}, {60, 130}, a.getCenter(), a.getRadius(), a.getOrientation()));
}

template<>
template<>
void object::test<51>()
{
    set_test_name("IOX-ILI: twoARCS_SameRadiusAndCenter_DontOverlay");
    // two arcs with same center and radius that don't touch each other.

    CircularArc a({70.0,  60.0}, {50.0,  100.0}, {70.0,  140.0});
    CircularArc b({140.0, 70.0}, {150.0, 100.0}, {140.0, 130.0});

    checkIntersection(a, b, CircularArcIntersector::NO_INTERSECTION);
}

template<>
template<>
void object::test<52>()
{
    set_test_name("IOX-ILI: twoARCS_SameRadiusAndCenter_Touch_DontOverlay");
    // Two arcs with same radius and center that touch at the endpoints

    CircularArc a({50.0,  100.0}, {100.0, 150.0}, {150.0, 100.0});
    CircularArc b({150.0, 100.0}, {100.0, 50.0},  {50.0,  100.0});

    checkIntersection(a, b, CircularArcIntersector::TWO_POINT_INTERSECTION, a.p0, a.p2);
}

template<>
template<>
void object::test<53>()
{
    set_test_name("IOX-ILI: twoARCS_SameRadiusAndCenter_Touch_DontOverlay_real");

    CircularArc a({2654828.912, 1223354.671}, {2654829.982, 1223353.601}, {2654831.052, 1223354.671});
    CircularArc b({2654831.052, 1223354.671}, {2654829.982, 1223355.741}, {2654828.912, 1223354.671});

    checkIntersection(a, b, CircularArcIntersector::TWO_POINT_INTERSECTION, a.p0, a.p2);
}

template<>
template<>
void object::test<54>()
{
    set_test_name("IOX-ILI: twoARCS_intersect0");
    // https://github.com/claeis/ilivalidator/issues/186

    CircularArc a({2658317.225, 1250832.586}, {2658262.543, 1250774.465}, {2658210.528, 1250713.944});
    CircularArc b({2658211.456, 1250715.072}, {2658161.386, 1250651.279}, {2658114.283, 1250585.266});

    // An intersection is visually apparent in QGIS, but CGAL 5.6 reports no intersections...
    checkIntersection(a, b, CircularArcIntersector::NO_INTERSECTION);
}

template<>
template<>
void object::test<55>()
{
    set_test_name("IOX-ILI: twoARCS_issue308");
    // https://github.com/claeis/ili2db/issues/308
    
    CircularArc a({2653134.354, 1227788.188}, {2653137.455, 1227797.289}, {2653140.555, 1227806.391});
    CircularArc b({2653135.557, 1227789.0},   {2653134.819, 1227788.796}, {2653134.354, 1227788.188});

    // expected result calculated with CGAL 5.6
    checkIntersection(a, b,
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        CoordinateXY{2653134.35399999982, 1227788.18800000008});
}

}