#include <tut/tut.hpp>

#include <geos/algorithm/CircularArcIntersector.h>
#include <geos/geom/LineSegment.h>
#include <geos/constants.h>
#include <variant>

#include "geos/util.h"

using geos::algorithm::CircularArcIntersector;
using geos::algorithm::Orientation;
using geos::geom::Ordinate;
using geos::geom::Coordinate;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYM;
using geos::geom::CoordinateXYZM;
using geos::geom::CircularArc;
using geos::MATH_PI;

namespace tut {

struct test_circulararcintersector_data {

    using XY = CoordinateXY;
    using XYZ = Coordinate;
    using XYM = CoordinateXYM;
    using XYZM = CoordinateXYZM;

    static constexpr double NaN = geos::DoubleNotANumber;

    using ArcOrPoint = std::variant<XY, XYZ, XYM, XYZM, CircularArc>;

    static std::string to_string(CircularArcIntersector::intersection_type t)
    {
        switch (t) {
        case CircularArcIntersector::NO_INTERSECTION:
            return "no intersection";
        case CircularArcIntersector::ONE_POINT_INTERSECTION:
            return "one-point intersection";
        case CircularArcIntersector::TWO_POINT_INTERSECTION:
            return "two-point intersection";
        case CircularArcIntersector::COCIRCULAR_INTERSECTION:
            return "cocircular intersection";
        }

        return "";
    }

    static std::string toWKT(const CoordinateXYZM& pt)
    {
        const bool hasZ = !std::isnan(pt.z);
        const bool hasM = !std::isnan(pt.m);

        std::stringstream ss;
        ss << "POINT ";
        if (hasZ) {
            ss << "Z";
        }
        if (hasM) {
            ss << "M";
        }
        if (hasZ || hasM) {
            ss << " ";
        }
        ss << "(";
        ss << pt.x << " " << pt.y;
        if (hasZ) {
            ss << " " << pt.z;
        }
        if (hasM) {
            ss << " " << pt.m;
        }
        ss << ")";

        return ss.str();
    }

    static std::string toWKT(const CircularArc& arc)
    {
        return arc.toString();
    }

    static std::string toWKT(const CoordinateSequence & seg)
    {
        std::stringstream ss;

        ss << "LINESTRING (";
        seg.applyAt(0, [&ss](const auto& pt) {
            ss << pt << ", " << *(&pt + 1);
        });
        ss << ")";

        return ss.str();
    }

    template<typename C1, typename C2>
    static void checkIntersection(C1 p0, C1 p1, C1 p2,
                                  C2 q0, C2 q1, C2 q2,
                                  CircularArcIntersector::intersection_type result,
                                  const ArcOrPoint& i0 = CoordinateXYZM::getNull(),
                                  const ArcOrPoint& i1 = CoordinateXYZM::getNull())
    {
        CoordinateSequence cs1(3, C1::template has<Ordinate::Z>(), C1::template has<Ordinate::M>());
        cs1.setAt(p0, 0);
        cs1.setAt(p1, 1);
        cs1.setAt(p2, 2);

        CoordinateSequence cs2(3, C2::template has<Ordinate::Z>(), C2::template has<Ordinate::M>());
        cs2.setAt(q0, 0);
        cs2.setAt(q1, 1);
        cs2.setAt(q2, 2);

        const CircularArc a0(cs1, 0);
        const CircularArc a1(cs2, 0);

        checkIntersection(a0, a1, result, i0, i1);
    }

    template<typename C1, typename C2>
    static void checkIntersectionArcSeg(C1 p0, C1 p1, C1 p2,
                                        C2 q0, C2 q1,
                                        CircularArcIntersector::intersection_type result,
                                        const ArcOrPoint& i0 = CoordinateXYZM::getNull(),
                                        const ArcOrPoint& i1 = CoordinateXYZM::getNull())
    {
        CoordinateSequence cs1(3, C1::template has<Ordinate::Z>(), C1::template has<Ordinate::M>());
        cs1.setAt(p0, 0);
        cs1.setAt(p1, 1);
        cs1.setAt(p2, 2);

        const CircularArc arc(cs1, 0);

        CoordinateSequence seg(2, C2::template has<Ordinate::Z>(), C2::template has<Ordinate::M>());
        seg.setAt(q0, 0);
        seg.setAt(q1, 1);

        checkIntersection(arc, seg, result, i0, i1);
    }

    template<typename C1, typename C2>
    static void checkIntersectionSegArc(C1 p0, C1 p1,
                                        C2 q0, C2 q1, C2 q2,
                                        CircularArcIntersector::intersection_type result,
                                        const ArcOrPoint& i0 = CoordinateXYZM::getNull(),
                                        const ArcOrPoint& i1 = CoordinateXYZM::getNull())
    {
        CoordinateSequence seg(2, C1::template has<Ordinate::Z>(), C1::template has<Ordinate::M>());
        seg.setAt(p0, 0);
        seg.setAt(p1, 1);


        CoordinateSequence arcSeq(3, C2::template has<Ordinate::Z>(), C2::template has<Ordinate::M>());
        arcSeq.setAt(q0, 0);
        arcSeq.setAt(q1, 1);
        arcSeq.setAt(q2, 2);
        const CircularArc arc(arcSeq, 0);

        checkIntersection(arc, seg, result, i0, i1, true);
    }

    template<typename C1, typename C2>
    static void checkIntersectionSegSeg(C1 p0, C1 p1,
                                        C2 q0, C2 q1,
                                        CircularArcIntersector::intersection_type result,
                                        const ArcOrPoint& i0 = CoordinateXYZM::getNull(),
                                        const ArcOrPoint& i1 = CoordinateXYZM::getNull())
    {
        CoordinateSequence seg0(2, C1::template has<Ordinate::Z>(), C1::template has<Ordinate::M>());
        seg0.setAt(p0, 0);
        seg0.setAt(p1, 1);

        CoordinateSequence seg1(2, C2::template has<Ordinate::Z>(), C2::template has<Ordinate::M>());
        seg1.setAt(q0, 0);
        seg1.setAt(q1, 1);

        checkIntersection(seg0, seg1, result, i0, i1, true);
    }


    static bool pointWithinTolerance(const CoordinateXYZM& actual, const CoordinateXYZM& expected, double tol)
    {
        if (std::isnan(actual.z) != std::isnan(expected.z)) {
            return false;
        }

        if (std::isnan(actual.m) != std::isnan(expected.m)) {
            return false;
        }

        if (!std::isnan(expected.z) && std::abs(actual.z - expected.z) > tol * std::abs(expected.z)) {
            return false;
        }

        if (!std::isnan(expected.m) && std::abs(actual.m - expected.m) > tol * std::abs(expected.m)) {
            return false;
        }

        if (actual.distance(expected) < tol) {
            return true;
        }

        if (std::abs(actual.x - expected.x) > tol * std::abs(expected.x)) {
            return false;
        }

        if (std::abs(actual.y - expected.y) > tol * std::abs(expected.y)) {
            return false;
        }

        return true;
    }

    template<typename T1, typename T2>
    static void checkIntersection(const T1& a0,
                                  const T2& a1,
                                  CircularArcIntersector::intersection_type result,
                                  const ArcOrPoint& p0 = CoordinateXYZM::getNull(),
                                  const ArcOrPoint& p1 = CoordinateXYZM::getNull(),
                                  bool useSegEndpoints=false)
    {
        CircularArcIntersector cai;
        if constexpr (std::is_same_v<T1, CircularArc>)
            if constexpr (std::is_same_v<T2, CircularArc>) {
                geos::ignore_unused_variable_warning(useSegEndpoints); // needed for gcc 10
                cai.intersects(a0, a1);
            } else {
                cai.intersects(a0, a1, 0, 1, useSegEndpoints);
        } else {
            static_assert(std::is_same_v<T1, CoordinateSequence>);
            geos::ignore_unused_variable_warning(useSegEndpoints); // needed for gcc 10
            cai.intersects(a0, 0, 1, a1, 0, 1);
        }

        ensure_equals("incorrect intersection type between " + toWKT(a0) + " and " + toWKT(a1), to_string(cai.getResult()), to_string(result));

        std::vector<CoordinateXYZM> expectedPoints;
        std::vector<CircularArc> expectedArcs;

        for (const auto& intersection : { p0, p1 }) {
            std::visit([&expectedArcs, &expectedPoints](const auto& isect) {
                using IntersectionType = std::decay_t<decltype(isect)>;

                if constexpr (std::is_same_v<IntersectionType, CircularArc>) {
                    expectedArcs.push_back(isect);
                } else {
                    if (!isect.isNull()) {
                        expectedPoints.push_back(CoordinateXYZM(isect));
                    }
                }
            }, intersection);
        }

        std::vector<CoordinateXYZM> actualPoints;
        std::vector<CircularArc> actualArcs;

        for (std::uint8_t i = 0; i < cai.getNumPoints(); i++) {
            actualPoints.push_back(cai.getPoint(i));
        }

        for (std::uint8_t i = 0; i < cai.getNumArcs(); i++) {
            actualArcs.push_back(cai.getArc(i));
        }

        auto compareArcs = [](const CircularArc& a, const CircularArc& b) {
            int cmp = a.p0().compareTo(b.p0());
            if (cmp != 0) {
                return cmp == -1;
            }
            cmp = a.p2().compareTo(b.p2());
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
                if (!pointWithinTolerance(actualPoints[i], expectedPoints[i], eps)) {
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

                if (!pointWithinTolerance(XYZM(actualArcs[i].getCenter()), XYZM(expectedArcs[i].getCenter()), eps)) {
                    equal = false;
                }

                XYZM actual0, expected0;
                actualArcs[i].getCoordinateSequence()->getAt(actualArcs[i].getCoordinatePosition(), actual0);
                expectedArcs[i].getCoordinateSequence()->getAt(expectedArcs[i].getCoordinatePosition(), expected0);

                if (!pointWithinTolerance(actual0, expected0, eps)) {
                    equal = false;
                }

                XYZM actual2, expected2;
                actualArcs[i].getCoordinateSequence()->getAt(actualArcs[i].getCoordinatePosition(), actual2);
                expectedArcs[i].getCoordinateSequence()->getAt(expectedArcs[i].getCoordinatePosition(), expected2);

                if (!pointWithinTolerance(actual2, expected2, eps)) {
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

        ensure_equals("incorrect intersection loc between " + toWKT(a0) + " and " + toWKT(a1), actual, expected);
    }

    const CoordinateXY NW_ = { -std::sqrt(2)/2, std::sqrt(2)/2 };
    const CoordinateXY N_ =  { 0, 1};
    const CoordinateXY NE_ = {  std::sqrt(2)/2, std::sqrt(2)/2 };
    const CoordinateXY E_ =  { 1, 0};
    const CoordinateXY SE_ = {  std::sqrt(2)/2, -std::sqrt(2)/2 };
    const CoordinateXY S_ =  { 0, -1};
    const CoordinateXY SW_ = { -std::sqrt(2)/2, -std::sqrt(2)/2 };
    const CoordinateXY W_ =  { -1, 0};
};

using group = test_group<test_circulararcintersector_data>;
using object = group::object;

group test_circulararcintersector_group("geos::algorithm::CircularArcIntersector");

template<>
template<>
void object::test<1>()
{
    set_test_name("interior/interior intersection (one point)");

    checkIntersection(
        XY{0, 0}, XY{1, std::sqrt(3)}, XY{2, 2},
        XY{0, 2}, XY{1, std::sqrt(3)}, XY{2, 0},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XY{1, std::sqrt(3)}
    );
}

template<>
template<>
void object::test<2>()
{
    set_test_name("interior/interior intersection (two points)");

    // result from CGAL 5.4
    checkIntersection(
        XY{0, 0}, XY{2, 2}, XY{4, 0},
        XY{0, 1}, XY{2, -1}, XY{4, 1},
        CircularArcIntersector::TWO_POINT_INTERSECTION,
        XY{0.0635083268962914893, 0.5},
        XY{3.93649167310370851, 0.5}
    );
}

template<>
template<>
void object::test<3>()
{
    set_test_name("single endpoint-endpoint intersection");

    checkIntersection(
        XY{0, 0}, XY{1, 1}, XY{2, 0},
        XY{2, 0}, XY{3, -1}, XY{4, 0},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XY{2, 0}
    );
}

template<>
template<>
void object::test<4>()
{
    set_test_name("single interior-interior intersection at point of tangency");

    checkIntersection(
        XY{0, 0}, XY{1, 1}, XY{2, 0},
        XY{0, 2}, XY{1, 1}, XY{2, 2},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XY{1, 1}
    );
}

template<>
template<>
void object::test<5>()
{
    set_test_name("supporting circles intersect but arcs do not");

    checkIntersection(
        XY{0, 0}, XY{2, 2}, XY{4, 0},
        XY{1, 1}, XY{0, -1}, XY{-1, 1},
      CircularArcIntersector::NO_INTERSECTION
    );

}

template<>
template<>
void object::test<6>()
{
    set_test_name("one circle contained within other");

    checkIntersection(
        XY{0, 0}, XY{4, 4}, XY{8, 0},
        XY{2, 0}, XY{4, 2}, XY{6, 0},
        CircularArcIntersector::NO_INTERSECTION
    );
}

template<>
template<>
void object::test<7>()
{
    set_test_name("cocircular with double endpoint intersection");

    checkIntersection(
        XY{0, 0}, XY{1, 1}, XY{2, 0},
        XY{0, 0}, XY{1, -1}, XY{2, 0},
        CircularArcIntersector::TWO_POINT_INTERSECTION,
        XY{0, 0}, XY{2, 0}
    );
}

template<>
template<>
void object::test<8>()
{
    set_test_name("cocircular with single endpoint intersection");

    checkIntersection(
        XY{-2, 0}, XY{0, 2}, XY{2, 0},
        XY{0, -2}, XY{std::sqrt(2), -std::sqrt(2)}, XY{2, 0},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XY{2, 0}
    );
}

template<>
template<>
void object::test<9>()
{
    set_test_name("cocircular disjoint");

    checkIntersection(NW_, N_, NE_,
                      SW_, S_, SE_,
                      CircularArcIntersector::NO_INTERSECTION);
}

template<>
template<>
void object::test<10>()
{
    set_test_name("cocircular with single arc intersection (clockwise)");

    checkIntersection(
        XY{-5, 0}, XY{0, 5}, XY{5, 0}, // CW
        XY{-4, 3}, XY{0, 5}, XY{4, 3}, // CW
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XY{-4, 3}, {0, 5}, {4, 3})
    ); // CW
}

template<>
template<>
void object::test<11>()
{
    set_test_name("cocircular with single arc intersection (counter-clockwise)");

    checkIntersection(
        XY{5, 0}, XY{0, 5}, XY{-5, 0}, // CCW
        XY{-4, 3}, XY{0, 5}, XY{4, 3}, // CW
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XY{4, 3}, {0, 5}, {-4, 3})
    ); // CCW
}

template<>
template<>
void object::test<12>()
{
    set_test_name("cocircular with arc and point intersections");

    checkIntersection(
        XY{-5, 0}, XY{0, 5}, XY{5, 0},
        XY{5, 0}, XY{0, -5}, XY{0, 5},
      CircularArcIntersector::COCIRCULAR_INTERSECTION,
     CircularArc::create(XY{-5, 0}, {-5 * std::sqrt(2) / 2, 5 * std::sqrt(2) / 2}, {0, 5}),
     XY{5, 0}
    );
}

template<>
template<>
void object::test<13>()
{
    set_test_name("cocircular with two arc intersections");

    checkIntersection(
        XY{-5, 0}, XY{0, 5}, XY{5, 0},
        XY{3, 4}, XY{0, -5}, XY{-3, 4},
      CircularArcIntersector::COCIRCULAR_INTERSECTION,
     CircularArc::create(XY{3, 4}, {4.4721359549995796, 2.2360679774997898}, {5, 0}),
     CircularArc::create(XY{-5, 0}, {-4.4721359549995796, 2.2360679774997907}, {-3, 4})
    );
}

template<>
template<>
void object::test<20>()
{
    set_test_name("arc - degenerate arc with single interior intersection");

    checkIntersection(XY{0, 0}, XY{2, 2}, XY{4, 0}, // CW arc
                      XY{-1, -4}, XY{1, 0}, XY{3, 4}, // degenerate arc
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{2, 2});

    checkIntersection(XY{-1, -4}, XY{1, 0}, XY{3, 4}, // degenerate arc
                      XY{0, 0}, XY{2, 2}, XY{4, 0}, // CW arc
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{2, 2});
}

template<>
template<>
void object::test<21>()
{
    set_test_name("two degenerate arcs with single interior intersection");

    checkIntersection(XY{0, 0}, XY{4, 4}, XY{10, 10},
                      XY{10, 0}, XY{1, 9}, XY{0, 10},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{5, 5});
}

template<>
template<>
void object::test<30>()
{
    set_test_name("arc-segment with single interior intersection");

    checkIntersectionArcSeg(
        XY{0, 0}, XY{2, 2}, XY{4, 0},
        XY{1, 0}, XY{3, 4},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XY{2, 2}
    );
}

template<>
template<>
void object::test<31>()
{
    set_test_name("arc-vertical segment with single interior intersection");

    checkIntersectionArcSeg(
        XY{-2, 0}, XY{0, 2}, XY{2, 0},
        XY{0, 0}, XY{0, 4},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XY{0, 2}
    );
}

template<>
template<>
void object::test<32>()
{
    set_test_name("arc-segment with two interior intersections");

    checkIntersectionArcSeg(
        W_, E_, SW_,
        XY{-10, 10}, XY{10, -10},
        CircularArcIntersector::TWO_POINT_INTERSECTION,
        NW_, SE_
    );
}

template<>
template<>
void object::test<33>()
{
    set_test_name("arc-vertical segment with two interior intersections");

    checkIntersectionArcSeg(
        W_, E_, SW_,
        XY{0, -2}, XY{0, 2},
        CircularArcIntersector::TWO_POINT_INTERSECTION,
        S_, N_
    );
}

template<>
template<>
void object::test<34>()
{
    set_test_name("arc-segment disjoint with bbox containment");

    checkIntersectionArcSeg(
        W_, N_, E_,
        XY{0, 0}, XY{0.2, 0.2},
        CircularArcIntersector::NO_INTERSECTION
    );
}

template<>
template<>
void object::test<35>()
{
    set_test_name("degenerate arc-segment with interior intersection");

    checkIntersectionArcSeg(
        XY{-5, -5}, XY{0, 0}, XY{5, 5},
        XY{-5, 5},  XY{5, -5},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XY{0, 0}
    );
}

template<>
template<>
void object::test<36>()
{
    set_test_name("intersection between a segment and a degenerate arc (radius = Infinity)");

    checkIntersectionArcSeg(
        XY{-5, -5}, XY{0, 0}, XY{5, 5 + 1e-14},
        XY{-5, 5}, XY{5, -5},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XY{0, 0}
    );
}

template<>
template<>
void object::test<37>()
{
    set_test_name("intersection between a segment and a nearly-degenerate arc (radius ~= 1e5)");

    checkIntersectionArcSeg(
        XY{-5, -5}, XY{0, 0}, XY{5, 5 + 1e-4},
        XY{-5, 5}, XY{5, -5},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XY{0, 0}
    );
}

template<>
template<>
void object::test<38>()
{
    set_test_name("arc-segment tests from ILI validator");
    // https://github.com/claeis/iox-ili/blob/master/jtsext/src/test/java/ch/interlis/iom_j/itf/impl/hrg/ISCILRTest.java

    // test_1a
    checkIntersectionArcSeg(XY{0, 5},  XY{5, 0}, XY{0, -5},
                          XY{20, 5}, XY{20, -5},
                          CircularArcIntersector::NO_INTERSECTION),

    // test_2a
    checkIntersectionArcSeg(XY{0, 5}, XY{5, 0}, XY{0, -5},
                      XY{5, 5}, XY{5, 0},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{5, 0});

    // test_2b
    checkIntersectionArcSeg(XY{0, 5}, XY{5, 0}, XY{0, -5},
                      XY{5, 5}, XY{5, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{5, 0});

    // test_2c
    checkIntersectionArcSeg(XY{0, 5}, XY{4, 3}, XY{0, -5},
                      XY{5, 5}, XY{5, 0},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{5, 0});

    // test_2d
    checkIntersectionArcSeg(XY{0, 5}, XY{4, 3}, XY{0, -5},
                      XY{5, 5}, XY{5, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{5, 0});

    // test_3a
    checkIntersectionArcSeg(XY{0, 5}, XY{5, 0}, XY{0, -5},
                      XY{4, 5}, XY{4, -5},
                      CircularArcIntersector::TWO_POINT_INTERSECTION,
                      XY{4, 3}, XY{4, -3});

    // test_3b
    checkIntersectionArcSeg(XY{0, 5},  XY{5, 0}, XY{0, -5},
                      XY{-4, 5}, XY{-4, -5},
                      CircularArcIntersector::NO_INTERSECTION);

    // test_3c
    checkIntersectionArcSeg(XY{0, 5},  XY{5, 0}, XY{0, -5},
                      XY{4, 10}, XY{4, 5},
                      CircularArcIntersector::NO_INTERSECTION);


    // test_3d
    checkIntersectionArcSeg(XY{0, 5}, XY{3, 4}, XY{5, 0},
                      XY{4, 5}, XY{4, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{4, 3});

    // test_3e
    checkIntersectionArcSeg(XY{0, 5}, XY{5, 0}, XY{0, -5},
                      XY{4, 5}, XY{4, 0},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{4, 3});
}

template<>
template<>
void object::test<39>()
{
    set_test_name("arc-arc tests from ILI validator");
    // https://github.com/claeis/iox-ili/blob/master/jtsext/src/test/java/ch/interlis/iom_j/itf/impl/hrg/ISCICRTest.java

    // test_1: circles do not overlap
    checkIntersection(XY{0, 5},  XY{5, 0},  XY{0, -5},
                      XY{20, 5}, XY{15, 0}, XY{20, -5},
                      CircularArcIntersector::NO_INTERSECTION);

    // test_2a: arcs overlap at a point
    checkIntersection(XY{0, 5},  XY{5, 0}, XY{0, -5},
                      XY{10, 5}, XY{5, 0}, XY{10, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{5, 0});
    // test_2b: arcs overlap at a point that is not a definition point of either arc
    checkIntersection(XY{0, 5},  XY{4, 3}, XY{0, -5},
                      XY{10, 5}, XY{6, 3}, XY{10, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{5, 0});

    // test_3a: circles overlap at two points that are within both arcs
    checkIntersection(XY{0, 5}, XY{5, 0}, XY{0, -5},
                      XY{8, 5}, XY{3, 0}, XY{8, -5},
                      CircularArcIntersector::TWO_POINT_INTERSECTION,
                      XY{4, 3}, XY{4, -3});

    // test_3b: circles overlap at two points but neither is on the first arc
    checkIntersection(XY{0, 5}, XY{-5, 0}, XY{0, -5},
                      XY{8, 5}, XY{3, 0},  XY{8, -5},
                      CircularArcIntersector::NO_INTERSECTION);

    // test_3c: circles overlap at two points but neither is on the first or second arc
    checkIntersection(XY{0, 5}, XY{-5, 0}, XY{0, -5},
                      XY{8, 5}, XY{13, 0}, XY{8, -5},
                      CircularArcIntersector::NO_INTERSECTION);

    // test_3d: circles overlap at two points but one is not on the first arc
    checkIntersection(XY{5, 0}, XY{3, -4}, XY{0, -5},
                      XY{8, 5}, XY{3, 0},  XY{8, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{4, -3});

    // test_3e: circles overlap at two points but one is not on the second arc
    checkIntersection(XY{0, 5}, XY{5, 0},  XY{0, -5},
                      XY{3, 0}, XY{5, -4}, XY{8, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{4, -3});

    // test_4a: cocircular
    checkIntersection(XY{0, 5}, XY{5, 0}, XY{0, -5},
                      XY{4, 3}, XY{5, 0}, XY{4, -3},
                      CircularArcIntersector::COCIRCULAR_INTERSECTION,
                      CircularArc::create(XY{4, 3}, {5, 0}, {4, -3}));
}

#if 0
// failed assertion: Values are not equal: expected `POINT (0 0)` actual `POINT (-5.4568517953157425e-06 5.4568517953157425e-06)`
template<>
template<>
void object::test<40>()
{
    set_test_name("intersection between a segment and a nearly-degenerate arc (radius ~= 2e6)");

    checkIntersection(XY{-5, -5}, XY{0, 0}, XY{5, 5 + 1e-9},
                      XY{-5, 5},  XY{5, -5},
                      CircularArcIntersector::ONE_POINT_INTERSECTION,
                      XY{0, 0});
}
#endif

template<>
template<>
void object::test<41>()
{
    set_test_name("IOX-ILI: testFastGerade");

    checkIntersectionArcSeg(
        XY{611770.424, 234251.322}, XY{611770.171, 234250.059}, XY{611769.918, 234248.796},
        XY{611613.84, 233467.819},  XY{611610.392, 233468.995},
        CircularArcIntersector::NO_INTERSECTION
    );
}

template<>
template<>
void object::test<42>()
{
    set_test_name("IOX-ILI: testCircleCircleEndptTolerance");
    // two nearly-linear arcs touching at a single endpoint
    // Potential fix is to use tolerance for checking if computed points are within arc.

    checkIntersection(
        XY{645175.553, 248745.374}, XY{ 645092.332, 248711.677}, XY{ 645009.11, 248677.98},
        XY{645009.11, 248677.98}, XY{644926.69, 248644.616}, XY{ 644844.269, 248611.253},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XY{645009.110, 248677.980}
    );
}

template<>
template<>
void object::test<43>()
{
    set_test_name("IOX-ILI: overlayTwoARCS_SameEndPointsS_ameDirection");
    // two arcs with same arcPoint and radius.
    // startPoints and endPoints are same. lines are in same direction

    checkIntersection(
        XY{100.0, 100.0}, XY{120,150.0}, XY{100.0,200.0},
        XY{100.0, 100.0}, XY{120,150.0}, XY{100.0,200.0},
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XY{100.0, 100.0}, {120, 150}, {100, 200})
    );
}

template<>
template<>
void object::test<44>()
{
    set_test_name("IOX-ILI: overlayTwoARCS_DifferentArcPointOnSameArcLineS_ameDirection");
    // two arcs with different arcPoint (on same arcLine) and same radius length.
    // startPoints and endPoints are same. lines are in same direction.

    checkIntersection(
        XY{0.0, 10.0}, XY{4.0,8.0}, XY{0.0,0.0},
        XY{0.0, 10.0}, XY{4.0,2.0}, XY{0.0,0.0},
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XY{0, 10}, {5, 5}, {0, 0})
    );
}

template<>
template<>
void object::test<45>()
{
    set_test_name("IOX-ILI: overlayTwoARCSS_ameArcPointOnSameArcLine_OneArcLineIsLonger");
    // two arcs with same arcPoint (on same arcLine) and same radius length.
    // one arc line is longer than the other arc line.
    // startPoints is same, endPoints are different. lines are in same direction.

    checkIntersection(
        XY{0.0, 10.0}, XY{4.0,8.0}, XY{0.0,0.0},
        XY{0.0, 10.0}, XY{4.0,8.0}, XY{4.0,2.0},
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XY{0, 10}, XY{4, 2}, XY{0, 5}, 5, Orientation::CLOCKWISE)
    );
}

template<>
template<>
void object::test<46>() 
{
    set_test_name("IOX-ILI: overlayTwoARCSS_ameEndPoints_OtherDirection");
    // two arcs with same arcPoint and radius
    // startPoint1 is equal to endPoint2, startPoint2 is equal to endPoint1.

    checkIntersection(
        XY{100.0, 100.0}, XY{80.0, 150.0}, XY{100.0, 200.0},
        XY{100.0, 200.0}, XY{80.0, 150.0}, XY{100.0, 100.0},
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XY{100.0, 100.0}, {80.0, 150.0}, {100.0, 200.0}));
}

template<>
template<>
void object::test<47>() 
{
    set_test_name("IOX-ILI: overlayTwoARCS_DifferentStartPointsS_ameDirection_DifferentLength");
    // two arcs. ArcPoint is equal. different angle.
    // startPoints are different. endPoints are same.
    CircularArc a = CircularArc::create(XY{70.0, 60.0}, {50.0, 100.0}, {60.0, 130.0});
    CircularArc b = CircularArc::create(XY{60.0, 70.0}, {50.0, 100.0}, {60.0, 130.0});

    checkIntersection(a, b,
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XY{60, 70}, {60, 130}, a.getCenter(), a.getRadius(), a.getOrientation()));
}

template<>
template<>
void object::test<48>()
{
    set_test_name("IOX-ILI: overlayTwoARCS_DifferentStartEndPoints_OtherDirection_DifferentLength");
    // Two cocircular arcs with opposite orientation.
    // ArcPoint is equal.
    // startPoints are different. endPoints are different.

    CircularArc a = CircularArc::create(XY{70.0,  60.0}, {50.0, 100.0}, {70.0, 140.0});
    CircularArc b = CircularArc::create(XY{60.0, 130.0}, {50.0, 100.0}, {60.0,  70.0});

    checkIntersection(a, b, 
            CircularArcIntersector::COCIRCULAR_INTERSECTION,
            CircularArc::create(XY{60, 70}, XY{60, 130}, a.getCenter(), a.getRadius(), a.getOrientation()));

}

template<>
template<>
void object::test<49>()
{
    set_test_name("IOX-ILI: overlayTwoARCS_DifferentEndPointsS_ameDirection_DifferentLength");
    // Two arcs with same orientation. 
    // ArcPoint is equal.
    // startPoints are same, endpoints are different

    CircularArc a = CircularArc::create(XY{70.0, 60.0}, {50.0, 100.0}, {70.0, 140.0});
    CircularArc b = CircularArc::create(XY{70.0, 60.0}, {50.0, 100.0}, {60.0, 130.0});

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


    CircularArc a = CircularArc::create(XY{70.0,  60.0}, {50.0, 100.0}, {70.0, 140.0});
    CircularArc b = CircularArc::create(XY{60.0, 130.0}, {50.0, 100.0}, {70.0,  60.0});

    checkIntersection(a, b,
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XY{70, 60}, XY{60, 130}, a.getCenter(), a.getRadius(), a.getOrientation()));
}

template<>
template<>
void object::test<51>()
{
    set_test_name("IOX-ILI: twoARCSS_ameRadiusAndCenter_DontOverlay");
    // two arcs with same center and radius that don't touch each other.

    CircularArc a = CircularArc::create(XY{70.0,  60.0}, {50.0,  100.0}, {70.0,  140.0});
    CircularArc b = CircularArc::create(XY{140.0, 70.0}, {150.0, 100.0}, {140.0, 130.0});

    checkIntersection(a, b, CircularArcIntersector::NO_INTERSECTION);
}

template<>
template<>
void object::test<52>()
{
    set_test_name("IOX-ILI: twoARCSS_ameRadiusAndCenter_Touch_DontOverlay");
    // Two arcs with same radius and center that touch at the endpoints

    CircularArc a = CircularArc::create(XY{50.0,  100.0}, {100.0, 150.0}, {150.0, 100.0});
    CircularArc b = CircularArc::create(XY{150.0, 100.0}, {100.0, 50.0},  {50.0,  100.0});

    checkIntersection(a, b, CircularArcIntersector::TWO_POINT_INTERSECTION, a.p0(), a.p2());
}

template<>
template<>
void object::test<53>()
{
    set_test_name("IOX-ILI: twoARCSS_ameRadiusAndCenter_Touch_DontOverlay_real");
    // arcs touch at endpoints
    // Potential fix is to use tolerance for checking if computed points are within arc.

    CircularArc a = CircularArc::create(XY{2654828.912, 1223354.671}, {2654829.982, 1223353.601}, {2654831.052, 1223354.671});
    CircularArc b = CircularArc::create(XY{2654831.052, 1223354.671}, {2654829.982, 1223355.741}, {2654828.912, 1223354.671});

    checkIntersection(a, b, CircularArcIntersector::TWO_POINT_INTERSECTION, a.p0(), a.p2());
}

template<>
template<>
void object::test<54>()
{
    set_test_name("IOX-ILI: twoARCS_intersect0");
    // https://github.com/claeis/ilivalidator/issues/186

    CircularArc a = CircularArc::create(XY{2658317.225, 1250832.586}, {2658262.543, 1250774.465}, {2658210.528, 1250713.944});
    CircularArc b = CircularArc::create(XY{2658211.456, 1250715.072}, {2658161.386, 1250651.279}, {2658114.283, 1250585.266});

    // An intersection is visually apparent in QGIS, but CGAL 5.6 reports no intersections...
    checkIntersection(a, b, CircularArcIntersector::NO_INTERSECTION);
}

#if 0
// Failing because two intersection points are detected.
// One is and endpoint (exact), the other is an approximation of the same endpoint.
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
        XY{2653134.35399999982, 1227788.18800000008});
}
#endif

/// Z/M interpolation

template<>
template<>
void object::test<56>()
{
    set_test_name("arc XYZ / arc XYZ interior intersection");
    // Z value at the intersection point is the average of the interpolated values from each arc
    // In both cases the value is interpolated from points 2 3 of the arc

    checkIntersection(
        XYZ{-5, 0, 1}, XYZ{-4, 3, 41}, XYZ{4, 3, 53},
        XYZ{-5, 10, 0}, XYZ{-2, 9, 7}, XYZ{-2, 1, 13},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{0, 5, 0.25*(41 + 53 + 7 + 13)}
    );
}

template<>
template<>
void object::test<57>()
{
    set_test_name("arc XYZ / segment XYZ interior intersection");
    // Z value at the intersection point is the average of the interpolated values from the arc and segment

    checkIntersectionArcSeg(
        XYZ{-5, 0, 1}, XYZ{-4, 3, 41}, XYZ{4, 3, 53},
        XYZ{0, 0, 7}, XYZ{0, 10, 13},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{0, 5, 0.25*(41 + 53 + 7 + 13)}
    );
}

template<>
template<>
void object::test<58>()
{
    set_test_name("arc XYZ / arc XYM interior intersection");
    // Z value at the intersection point is interpolated from the arc with Z values
    // M value at the intersection point is interpolated from the arc with M values

    checkIntersection(
        XYZ{-5, 0, 1}, XYZ{-4, 3, 41}, XYZ{4, 3, 53},
        XYM{-5, 10, 0}, XYM{-2, 9, 7}, XYM{-2, 1, 13},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZM{0, 5, 0.5*(41 + 53), 0.5*(7 + 13)}
    );
}

template<>
template<>
void object::test<59>()
{
    set_test_name("arc XYZ / arc XYZ interior intersection with control point Z = NaN");
    // Z value at the intersection point is the average of the interpolated values from each arc
    // Because the control point Z is NaN, interpolation is done from the arc endpoints

    checkIntersection(
        XYZ{-5, 0, 41}, XYZ{-4, 3, NaN}, XYZ{5, 0, 53},
        XYZ{-2, 9, 7}, XYZ{-1, 8, NaN}, XYZ{-2, 1, 13},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{0, 5, 0.25*(41 + 53 + 7 + 13)}
    );
}

template<>
template<>
void object::test<60>()
{
    set_test_name("arc XYM / arc XYZ interior intersection with control point Z/M = NaN");
    // Z value at the intersection point is interpolated from the arc with Z values
    // Because the control point Z is NaN, interpolation is done from the arc endpoints
    // M value at the intersection point is interpolated from the arc with M values
    // Because the control point M is NaN, interpolation is done from the arc endpoints

    checkIntersection(
        XYM{-5, 0, 41}, XYM{-4, 3, NaN}, XYM{5, 0, 53},
        XYZ{-2, 9, 7}, XYZ{-1, 8, NaN}, XYZ{-2, 1, 13},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZM{0, 5, 0.5*(7 + 13), 0.5*(41 + 53)}
    );
}

template<>
template<>
void object::test<61>()
{
    set_test_name("arc XYM / arc XYZ interior intersection with endpoint Z/M = NaN");
    // Z value at the intersection point is interpolated from the arc with Z values
    // Because the endpoint Z is NaN, the value of the control point is taken
    // M value at the intersection point is interpolated from the arc with M values
    // Because the endpoint M is NaN, the value of the control point is taken

    checkIntersection(
        XYM{-5, 0, 41}, XYM{-4, 3, 53}, XYM{5, 0, NaN},
        XYZ{-2, 9, 7}, XYZ{-1, 8, 13}, XYZ{-2, 1, NaN},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZM{0, 5, 13, 53}
    );
}

template<>
template<>
void object::test<62>()
{
    set_test_name("arc XYM / arc XYZ interior intersection with control point and endpoint Z/M = NaN");
    // Z value at the intersection point is interpolated from the arc with Z values
    // Because the endpoint and control point Z is NaN, the value of the other endpoint is taken
    // M value at the intersection point is interpolated from the arc with M values
    // Because the endpoint and control point M is NaN, the value of the other endpoint is taken

    checkIntersection(
        XYM{-5, 0, 41}, XYM{-4, 3, NaN}, XYM{5, 0, NaN},
        XYZ{-2, 9, 7}, XYZ{-1, 8, NaN}, XYZ{-2, 1, NaN},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZM{0, 5, 7, 41}
    );
}

template<>
template<>
void object::test<63>()
{
    set_test_name("arc XYZ / segment XYZ interior intersection with control point Z = NaN");
    // Z value at the intersection point is the average of the interpolated values from the arc and the segment
    // Because the control point Z is NaN, interpolation is done from the arc endpoints

    checkIntersectionArcSeg(
        XYZ{-5, 0, 41}, XYZ{-4, 3, NaN}, XYZ{5, 0, 53},
        XYZ{0, 0, 7}, XYZ{0, 10, 13},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{0, 5, 0.25*(41 + 53 + 7 + 13)}
    );
}

template<>
template<>
void object::test<64>()
{
    set_test_name("arc XYZ / segment XYZ interior intersection, segment endpoint Z = NaN");
    // Z value at the intersection point is the average of the interpolated values from the arc and the other segment endpoint

    checkIntersectionArcSeg(
        XYZ{-5, 0, 7}, XYZ{-3, 4, 41}, XYZ{3, 4, 53},
        XYZ{0, 0, NaN}, XYZ{0, 10, 13},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{0, 5, 0.25*(41 + 53 + 13 + 13)}
    );
}

template<>
template<>
void object::test<65>()
{
    set_test_name("arc XYZ / arc XYZ endpoint intersection");
    // Result Z value at intersection point is taken from the first input

    checkIntersection(
        XYZ{0, 0, 0}, XYZ{1, 1, 1}, XYZ{2, 0, 2},
        XYZ{2, 0, 500}, XYZ{3, -1, 501}, XYZ{4, 0, 502},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{2, 0, 2}
    );
}

template<>
template<>
void object::test<66>()
{
    set_test_name("arc XYZ / segment XYZ endpoint intersection");
    // Result Z value at intersection point is taken from the first input
    // Related: RobustLineIntersectorZTest::testEndpoint

    checkIntersectionArcSeg(
        XYZ{0, 0, 0},   XYZ{1, 1, 1}, XYZ{2, 0, 2},
        XYZ{2, 0, 500}, XYZ{4, 0, 502},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{2, 0, 2}
    );

    // Same inputs as above but with order reversed
    checkIntersectionSegArc(
        XYZ{2, 0, 500}, XYZ{4, 0, 502},
        XYZ{0, 0, 0},   XYZ{1, 1, 1}, XYZ{2, 0, 2},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{2, 0, 500}
    );
}

#if 0
// Fails because computed intersection point is not exactly equal to endpoint, so special handling doesn't apply.
template<>
template<>
void object::test<67>() {
    set_test_name("arc XYZ / arc XYZ interior / endpoint intersection");
    // Result Z is taken from the endpoint
    // Related: RobustLineIntersectorZTest::testInteriorEndpoint

    checkIntersection(
        XYZ{-1, 0, 1}, XYZ{0, 1, 2}, XYZ{1, 0, 3},
        XYZ{-2, 1, 7}, XYZ{-1, 2, 8}, XYZ{0, 1, 9},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{0, 1, 9}
    );
}
#endif

template<>
template<>
void object::test<68>() {
    set_test_name("arc XYZ / seg XYZ interior / endpoint intersection");
    // Result Z is taken from the endpoint
    // Related: RobustLineIntersectorZTest::testInteriorEndpoint

    checkIntersectionArcSeg(
        XYZ{-1, 0, 1}, XYZ{0, 1, 2}, XYZ{1, 0, 3},
        XYZ{0, 0, 5}, {0, 1, 9},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{0, 1, 9}
    );

    // Same inputs as above but with order reversed
    checkIntersectionSegArc(
        XYZ{0, 0, 5}, {0, 1, 9},
        XYZ{-1, 0, 1}, XYZ{0, 1, 2}, XYZ{1, 0, 3},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{0, 1, 9}
    );

}

template<>
template<>
void object::test<69>() {
    set_test_name("arc XYZ / arc XY interior / endpoint intersection");
    // Intersection is at interior of XYZ arc, endpoint of XY arc
    // Result Z is interpolated
    // Related: RobustLineIntersectorZTest::testInteriorEndpoint3D2D

    checkIntersection(
        XYZ{-5, 0, 1}, XYZ{-4, 3, 2}, XYZ{4, 3, 3},
        XY{-10, 5}, XY{-5, 10}, XY{0, 5},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{0, 5, 2.5}
    );
}

template<>
template<>
void object::test<70>() {
    set_test_name("arc XYZ / segment XY interior / endpoint intersection");
    // Intersection is at interior of XYZ arc, endpoint of XY segment
    // Result Z is interpolated
    // Related: RobustLineIntersectorZTest::testInteriorEndpoint3D2D

    checkIntersectionArcSeg(
        XYZ{-5, 0, 1}, XYZ{-4, 3, 2}, XYZ{4, 3, 3},
        XY{0, 0}, XY{0, 5},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{0, 5, 2.5}
    );
}

#if 0
// Fails because computed intersection point is not exactly equal to endpoint, so special handling doesn't apply.
template<>
template<>
void object::test<71>() {
    set_test_name("arc XY / arc XYZ interior / endpoint intersection");
    // Intersection is at interior of XY arc, endpoint of XYZ arc
    // Result Z is from 3D endpoint
    // Related: RobustLineIntersectorZTest::testInteriorEndpoint2D3D

    checkIntersection(
        XY{-5, 0}, XY{-4, 3}, XY{4, 3},
        XYZ{-10, 5, 8}, XYZ{-5, 10, 11}, XYZ{0, 5, 17},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{0, 5, 17}
    );
}
#endif

template<>
template<>
void object::test<72>() {
    set_test_name("arc XY / segment XYZ interior / endpoint intersection");
    // Intersection is at interior of XY arc, endpoint of XYZ segment
    // Result Z is from 3D endpoint
    // Related: RobustLineIntersectorZTest::testInteriorEndpoint2D3D

    checkIntersectionArcSeg(
        XY{-5, 0}, XY{-4, 3}, XY{4, 3},
        XYZ{0, 0, 3}, XYZ{0, 5, 17},
        CircularArcIntersector::ONE_POINT_INTERSECTION,
        XYZ{0, 5, 17}
    );
}

template<>
template<>
void object::test<73>() {
    set_test_name("XYZ arc intersected with itself");
    // Related:: RobustLineIntersectorTest::testCollinearEqual

    // clockwise inputs
    checkIntersection(
        XYZ{-5, 0, 0}, XYZ{0, 5, 0}, XYZ{5, 0, 15},
        XYZ{-5, 0, 0}, XYZ{0, 5, 0}, XYZ{5, 0, 15},
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XYZ{-5, 0, 0}, XYZ{0, 5, 0}, XYZ{5, 0, 15})
    );

    // counter-clockwise inputs
    checkIntersection(
        XYZ{5, 0, 15}, XYZ{0, 5, 0}, XYZ{-5, 0, 0},
        XYZ{5, 0, 15}, XYZ{0, 5, 0}, XYZ{-5, 0, 0},
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XYZ{5, 0, 15}, XYZ{0, 5, 0}, XYZ{-5, 0, 0})
    );

    // mixed-orientation inputs
    checkIntersection(
        XYZ{-5, 0, 0}, XYZ{0, 5, 0}, XYZ{5, 0, 15},
        XYZ{5, 0, 15}, XYZ{0, 5, 0}, XYZ{-5, 0, 0},
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XYZ{-5, 0, 0}, XYZ{0, 5, 0}, XYZ{5, 0, 15})
    );
}

template<>
template<>
void object::test<74>() {
    set_test_name("XYZ arc intersected with 2D version of same arc");
    // Related:: RobustLineIntersectorTest::testCollinearEqual3D2D

    // clockwise inputs
    checkIntersection(
        XYZ{-5, 0, 0}, XYZ{0, 5, 0}, XYZ{5, 0, 15},
        XY{-5, 0}, XY{0, 5}, XY{5, 0},
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XYZ{-5, 0, 0}, XYZ{0, 5, 0}, XYZ{5, 0, 15})
    );

    // counter-clockwise input
    checkIntersection(
        XYZ{5, 0, 15}, XYZ{0, 5, 0}, XYZ{-5, 0, 0},
        XY{5, 0}, XY{0, 5}, XY{-5, 0},
        CircularArcIntersector::COCIRCULAR_INTERSECTION,
        CircularArc::create(XYZ{5, 0, 15}, XYZ{0, 5, 0}, XYZ{-5, 0, 0})
    );
}

template<>
template<>
void object::test<75>() {
    set_test_name("Two cocircular XYZ arcs with endpoint intersections");
    // Z values of inputs are the same and are copied to output
    // Related:: RobustLineIntersectorTest::testCollinearEndpoint

    checkIntersection(
        XYZ{-5, 0, 3}, XYZ{0, 5, 11}, XYZ{5, 0, 15},
        XYZ{-5, 0, 3}, XYZ{0, -5, 11}, XYZ{5, 0, 15},
        CircularArcIntersector::TWO_POINT_INTERSECTION,
        XYZ{-5, 0, 3},
        XYZ{5, 0, 15}
    );

    // Same arguments as above, order reversed
    checkIntersection(
        XYZ{-5, 0, 3}, XYZ{0, -5, 11}, XYZ{5, 0, 15},
        XYZ{-5, 0, 3}, XYZ{0, 5, 11}, XYZ{5, 0, 15},
        CircularArcIntersector::TWO_POINT_INTERSECTION,
        XYZ{-5, 0, 3},
        XYZ{5, 0, 15}
    );
}

template<>
template<>
void object::test<76>() {
    set_test_name("Cocircular XYZ and XY arcs with endpoint intersections");
    // Z values of result is taken from the XYZ input
    // Related:: RobustLineIntersectorTest::testCollinearEndpoint3D2D

    // clockwise inputs
    checkIntersection(
        XYZ{-5, 0, 3}, XYZ{0, 5, 11}, XYZ{5, 0, 15},
        XY{-5, 0}, XY{0, -5}, XY{5, 0},
        CircularArcIntersector::TWO_POINT_INTERSECTION,
        XYZ{-5, 0, 3},
        XYZ{5, 0, 15}
    );

    // Same arguments as above, order reversed
    checkIntersection(
        XY{-5, 0}, XY{0, -5}, XY{5, 0},
        XYZ{-5, 0, 3}, XYZ{0, 5, 11}, XYZ{5, 0, 15},
        CircularArcIntersector::TWO_POINT_INTERSECTION,
        XYZ{-5, 0, 3},
        XYZ{5, 0, 15}
    );

    // counter-clockwise inputs
    checkIntersection(
        XYZ{5, 0, 15}, XYZ{0, 5, 11}, XYZ{-5, 0, 3},
        XY{5, 0}, XY{0, -5}, XY{-5, 0},
        CircularArcIntersector::TWO_POINT_INTERSECTION,
        XYZ{-5, 0, 3},
        XYZ{5, 0, 15}
    );

    // Same arguments as above, order reversed
    checkIntersection(
        XY{5, 0}, XY{0, -5}, XY{-5, 0},
        XYZ{5, 0, 15}, XYZ{0, 5, 11}, XYZ{-5, 0, 3},
        CircularArcIntersector::TWO_POINT_INTERSECTION,
        XYZ{-5, 0, 3},
        XYZ{5, 0, 15}
    );
}

template<>
template<>
void object::test<77>() {
    set_test_name("XYZ arc intersected with a subset of itself");
    // Related:: RobustLineIntersectorTest::testCollinearContained

    const double theta = std::atan(3.0 / 4.0);
    const double frac = theta / MATH_PI;

    // Clockwise inputs
    checkIntersection(
        XYZ{-5, 0, 0}, XYZ{0, 5, 7.5}, XYZ{5, 0, 15},
        XYZ{-4, 3, frac*15}, XYZ{0, 5, 7.5}, XYZ{4, 3, (1-frac)*15},
           CircularArcIntersector::COCIRCULAR_INTERSECTION,
                CircularArc::create(XYZ{-4, 3, frac*15}, XYZ{0, 5, 0}, XYZ{4, 3, (1-frac)*15})
    );

    // Counter-clockwise inputs
    checkIntersection(
        XYZ{5, 0, 15}, XYZ{0, 5, 7.5}, XYZ{-5, 0, 0},
        XYZ{4, 3, (1-frac)*15}, XYZ{0, 5, 7.5}, XYZ{-4, 3, frac*15},
           CircularArcIntersector::COCIRCULAR_INTERSECTION,
                CircularArc::create(XYZ{4, 3, (1-frac)*15}, XYZ{0, 5, 0}, XYZ{-4, 3, frac*15})
    );
}

template<>
template<>
void object::test<78>() {
    set_test_name("XYZ arc intersected with a 2D subset of itself");
    // Related:: RobustLineIntersectorTest::testCollinearContained3D2D

    const double theta = std::atan(3.0 / 4.0);
    const double frac = theta / MATH_PI;

    // clockwise inputs
    checkIntersection(
        XYZ{-5, 0, 0}, XYZ{0, 5, 7.5}, XYZ{5, 0, 15},
        XY{-4, 3}, XY{0, 5}, XY{4, 3},
           CircularArcIntersector::COCIRCULAR_INTERSECTION,
                CircularArc::create(XYZ{-4, 3, frac*15}, XYZ{0, 5, 7.5}, XYZ{4, 3, (1-frac)*15})
    );

    // counter-clockwise inputs
    checkIntersection(
        XYZ{5, 0, 15}, XYZ{0, 5, 7.5}, XYZ{-5, 0, 0},
        XY{4, 3}, XY{0, 5}, XY{-4, 3},
           CircularArcIntersector::COCIRCULAR_INTERSECTION,
                CircularArc::create(XYZ{4, 3, (1-frac)*15}, XYZ{0, 5, 7.5}, XYZ{-4, 3, frac*15})
    );
}

template<>
template<>
void object::test<79>() {
    set_test_name("XYZ arc intersected with a subset of itself that has different Z values");
    // Related:: RobustLineIntersectorTest::testCollinearContainedDifferentZ

    // clockwise inputs
    checkIntersection(
        XYZ{-5, 0, 0}, XYZ{0, 5, 7.5}, XYZ{5, 0, 15},
        XYZ{-4, 3, 100}, XYZ{0, 5, 150}, XYZ{4, 3, 200},
           CircularArcIntersector::COCIRCULAR_INTERSECTION,
                CircularArc::create(XYZ{-4, 3, 100}, XYZ{0, 5, 150}, XYZ{4, 3, 200})
    );

    // counter-clockwise inputs
    checkIntersection(
        XYZ{5, 0, 15}, XYZ{0, 5, 7.5}, XYZ{-5, 0, 0},
        XYZ{4, 3, 200}, XYZ{0, 5, 150}, XYZ{-4, 3, 100},
           CircularArcIntersector::COCIRCULAR_INTERSECTION,
                CircularArc::create(XYZ{4, 3, 200}, XYZ{0, 5, 150}, XYZ{-4, 3, 100})
    );
}

template<>
template<>
void object::test<80>() {
    set_test_name("XYZ arc intersected with a superset of itself that has different Z values");

    // clockwise inputs
    checkIntersection(
        XYZ{-4, 3, 100}, XYZ{0, 5, 150}, XYZ{4, 3, 200},
        XYZ{-5, 0, 0}, XYZ{0, 5, 7.5}, XYZ{5, 0, 15},
           CircularArcIntersector::COCIRCULAR_INTERSECTION,
                CircularArc::create(XYZ{-4, 3, 100}, XYZ{0, 5, 150}, XYZ{4, 3, 200})
    );

    // counter-clockwise inputs
    checkIntersection(
        XYZ{4, 3, 200}, XYZ{0, 5, 150}, XYZ{-4, 3, 100}, 
        XYZ{5, 0, 15},  XYZ{0, 5, 7.5}, XYZ{-5, 0, 0},
           CircularArcIntersector::COCIRCULAR_INTERSECTION,
                CircularArc::create(XYZ{4, 3, 200}, XYZ{0, 5, 150}, XYZ{-4, 3, 100})
    );

}

template<>
template<>
void object::test<81>
()
{
    // Interior intersection of two XYZM lines.
    // Result Z and M are the average of the interpolated coordinate values.
    set_test_name("XYZM segment and XYZM segment with interior intersection");
    // Related:: RobustLineIntersectorTest::testInteriorXYZM-XYZM

    checkIntersectionSegSeg(
            XYZM{1, 1, 1, -1}, XYZM{3, 3, 3, -3},
            XYZM{1, 3, 10, -10}, XYZM{3, 1, 30, -30},
            CircularArcIntersector::ONE_POINT_INTERSECTION,
            XYZM{2, 2, 11, -11}
    );
}

// TODO: check Z values of arc result centerpoints
// TODO: add tests for seg/seg

}
