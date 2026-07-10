#include <tut/tut.hpp>
#include "utility.h"

#include <geos/geom/CircularString.h>
#include <geos/noding/NodableArcString.h>

using geos::algorithm::Orientation;
using geos::geom::CircularArc;
using geos::geom::Ordinate;
using geos::geom::CircularString;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYZM;
using geos::noding::ArcString;
using geos::noding::NodableArcString;

namespace tut {

struct test_nodablearcstring_data {

    using XY = CoordinateXY;
    using XYZ = Coordinate;
    using XYM = geos::geom::CoordinateXYM;
    using XYZM = CoordinateXYZM;

    static void test_add_points(const CircularArc& arc, const std::vector<CoordinateXY>& coords,
                                const std::vector<std::vector<CircularArc>>& expected, bool reversed=false) {
        std::vector<CircularArc> arcs;
        arcs.push_back(arc);
        NodableArcString nas(arcs, nullptr, false, false, nullptr);

        for (const auto& coord : coords) {
            nas.addIntersection(coord, 0);
        }

        std::vector<std::unique_ptr<ArcString>> noded;
        nas.getNoded(noded);

        ensure_equals(noded.size(), expected.size());

        for (std::size_t i = 0; i < noded.size(); i++) {
            for (std::size_t j = 0; j < noded[i]->getSize(); j++) {
                ensure_arc_equals(noded[i]->getArc(j), expected[i][j], 1e-8);
            }
        }

        if (!reversed) {
            const auto revArc = arc.reverse();

            std::vector<std::vector<CircularArc>> revExpected;
            for (const auto& x : expected) {
                revExpected.push_back(x);
                std::reverse(revExpected.back().begin(), revExpected.back().end());
                for (auto& y : revExpected.back()) {
                    y = y.reverse();
                }
            }
            std::reverse(revExpected.begin(), revExpected.end());

            test_add_points(revArc, coords, revExpected, true);
        }
    }

    void checkSplit(const std::string& wkt, const CoordinateXY& splitPt, std::size_t pathIndex, const std::vector<std::string>& expected_wkt)
    {
        const auto g = reader_.read<CircularString>(wkt);
        const auto& gfact = *g->getFactory();

        NodableArcString nas(g->getArcs(), g->getSharedCoordinates(), false, false, nullptr);
        nas.addIntersection(splitPt, pathIndex);

        std::vector<std::unique_ptr<ArcString>> noded;
        nas.getNoded(noded);
        ensure_equals(noded.size(), expected_wkt.size());

        for (std::size_t i = 0; i < noded.size(); i++) {
            std::unique_ptr<Geometry> actual = gfact.createCircularString(noded[i]->getCoordinates());
            std::unique_ptr<Geometry> expected = reader_.read(expected_wkt[i]);
        }
    }

    static void ensure_arc_equals(const CircularArc& actual, const CircularArc& expected, double tol) {
        ensure(actual.toString() + " does not equal expected " + expected.toString() ,actual.equals(expected, tol));
    }

    geos::io::WKTReader reader_;
};

typedef test_group<test_nodablearcstring_data> group;
typedef group::object object;

group test_nodablearcstring_group("geos::noding::NodableArcString");

template<>
template<>
void object::test<1>()
{
    set_test_name("CW half-circle, upper half-plane");

    CircularArc in = CircularArc::create(CoordinateXY{-5, 0}, CoordinateXY{0, 5}, CoordinateXY{5, 0});

    std::vector<CoordinateXY> coords;
    coords.emplace_back(4, 3);
    coords.emplace_back(3, 4);
    coords.emplace_back(-3, 4);
    coords.emplace_back(-4, 3);

    std::vector<std::vector<CircularArc>> expected;
    expected.push_back({CircularArc::create(XY{-5, 0}, {-4, 3}, {0, 0}, 5, Orientation::CLOCKWISE)});
    expected.push_back({CircularArc::create(XY{-4, 3}, {-3, 4}, {0, 0}, 5, Orientation::CLOCKWISE)});
    expected.push_back({CircularArc::create(XY{-3, 4}, {3, 4}, {0, 0}, 5, Orientation::CLOCKWISE)});
    expected.push_back({CircularArc::create(XY{3, 4}, {4, 3}, {0, 0}, 5, Orientation::CLOCKWISE)});
    expected.push_back({CircularArc::create(XY{4, 3}, {5, 0}, {0, 0}, 5, Orientation::CLOCKWISE)});

    test_add_points(in, coords, expected);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("CW half-circle, right half-plane");

    CircularArc in = CircularArc::create(CoordinateXY{0, 5}, CoordinateXY{5, 0}, CoordinateXY{0, -5});

    std::vector<CoordinateXY> coords;
    coords.emplace_back(4, -3);
    coords.emplace_back(4, 3);
    coords.emplace_back(3, -4);
    coords.emplace_back(3, 4);
    coords.emplace_back(5, 0);

    std::vector<std::vector<CircularArc>> expected;
    expected.push_back({CircularArc::create(XY{0, 5}, {3, 4}, {0, 0}, 5, Orientation::CLOCKWISE)});
    expected.push_back({CircularArc::create(XY{3, 4}, {4, 3}, {0, 0}, 5, Orientation::CLOCKWISE)});
    expected.push_back({CircularArc::create(XY{4, 3}, {5, 0}, {0, 0}, 5, Orientation::CLOCKWISE)});
    expected.push_back({CircularArc::create(XY{5, 0}, {4, -3}, {0, 0}, 5, Orientation::CLOCKWISE)});
    expected.push_back({CircularArc::create(XY{4, -3}, {3, -4}, {0, 0}, 5, Orientation::CLOCKWISE)});
    expected.push_back({CircularArc::create(XY{3, -4}, {0, -5}, {0, 0}, 5, Orientation::CLOCKWISE)});

    test_add_points(in, coords, expected);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("no points added");
    CircularArc in = CircularArc::create(CoordinateXY{-1, 0}, CoordinateXY{0, 1}, CoordinateXY{1, 0});

    std::vector<CoordinateXY> coords;
    std::vector<std::vector<CircularArc>> expected;
    expected.push_back({in});
    test_add_points(in, coords, expected);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("Center point Z/M in constructed arcs interpolated from endpoints");

    CoordinateSequence seq = CoordinateSequence::XYZM(3);
    CoordinateXYZM p0{0, 5, 6, 2};
    CoordinateXYZM p1{5, 0, 7, 3};
    CoordinateXYZM p2{4, -3, 9, 1};

    seq.setAt(p0, 0);
    seq.setAt(p1, 1);
    seq.setAt(p2, 2);

    CircularArc arc (seq, 0);

    CoordinateXYZM intPt{4, 3, 13, 5};

    std::vector<CircularArc> in { arc };
    NodableArcString nas(std::move(in), nullptr, true, true, nullptr);

    nas.addIntersection( intPt, 0);

    std::vector<std::unique_ptr<ArcString>> noded;
    nas.getNoded(noded);

    ensure_equals(noded.size(), 2u);
    const CircularArc& arc0 = noded[0]->getArc(0);
    ensure_arc_equals(arc0, CircularArc::create(p0, intPt, arc.getCenter(), arc.getRadius(), arc.getOrientation()), 1e-8);
    ensure_equals(arc0.p1<CoordinateXYZM>().z, (p0.z + intPt.z) / 2);
    ensure_equals(arc0.p1<CoordinateXYZM>().m, (p0.m + intPt.m) / 2);

    const CircularArc& arc1 = noded[1]->getArc(0);
    ensure_arc_equals(arc1, CircularArc::create(intPt, p2, arc.getCenter(), arc.getRadius(), arc.getOrientation()), 1e-8);
    ensure_equals(arc1.p1<CoordinateXYZM>().z, (intPt.z + p2.z) / 2);
    ensure_equals(arc1.p1<CoordinateXYZM>().m, (intPt.m + p2.m) / 2);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("splits at arc endpoints");

    checkSplit("CIRCULARSTRING (100 0, 50 50, 0 0, -50 50, 0 100, 50 80, 100 100, 150 50, 100 0)",
                {0, 0}, 0, {
                 "CIRCULARSTRING (100 0, 50 50, 0 0)",
                 "CIRCULARSTRING (0 0, -50 50, 0 100, 50 80, 100 100, 150 50, 100 0)"
                });

    checkSplit("CIRCULARSTRING (100 0, 50 50, 0 0, -50 50, 0 100, 50 80, 100 100, 150 50, 100 0)",
                {0, 0}, 1, {
                 "CIRCULARSTRING (100 0, 50 50, 0 0)",
                 "CIRCULARSTRING (0 0, -50 50, 0 100, 50 80, 100 100, 150 50, 100 0)"
                });

    checkSplit("CIRCULARSTRING (100 0, 50 50, 0 0, -50 50, 0 100, 50 80, 100 100, 150 50, 100 0)",
                {100, 100}, 2, {
                 "CIRCULARSTRING (100 0, 50 50, 0 0)",
                 "CIRCULARSTRING (0 0, -50 50, 0 100, 50 80, 100 100, 150 50, 100 0)"
                });

    checkSplit("CIRCULARSTRING (100 0, 50 50, 0 0, -50 50, 0 100, 50 80, 100 100, 150 50, 100 0)",
                {100, 100}, 3, {
                 "CIRCULARSTRING (100 0, 50 50, 0 0)",
                 "CIRCULARSTRING (0 0, -50 50, 0 100, 50 80, 100 100, 150 50, 100 0)"
                });
}

}