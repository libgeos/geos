#include <tut/tut.hpp>
#include "utility.h"

#include <geos/noding/NodableArcString.h>

using geos::algorithm::Orientation;
using geos::geom::CircularArc;
using geos::geom::CoordinateXY;
using geos::noding::NodableArcString;

namespace tut {

struct test_nodablearcstring_data {

    static void test_add_points(const CircularArc& arc, const std::vector<CoordinateXY>& coords,
                                const std::vector<CircularArc>& expected, bool reversed=false) {
        std::vector<CircularArc> arcs;
        arcs.push_back(arc);
        NodableArcString nas(arcs);

        for (const auto& coord : coords) {
            nas.addInt(coord, 0);
        }

        auto noded = nas.getNoded();

        ensure_equals(noded->getSize(), expected.size());

        for (std::size_t i = 0; i < expected.size(); i++) {
            ensure_arc_equals(noded->getArc(i), expected[i]);
        }

        if (!reversed) {
            auto revArc = arc;
            revArc.reverse();

            auto revExpected = expected;
            for (auto& x : revExpected) {
                x.reverse();
            }
            std::reverse(revExpected.begin(), revExpected.end());

            test_add_points(revArc, coords, revExpected, true);
        }
    }

    static void ensure_arc_equals(const CircularArc& actual, const CircularArc& expected) {
        ensure_equals_xy(actual.p0, expected.p0);
        ensure_equals_xy(actual.p2, expected.p2);
        ensure_equals_xy(actual.getCenter(), expected.getCenter());
        ensure_equals(actual.getRadius(), expected.getRadius());
        ensure_equals(actual.getOrientation(), expected.getOrientation());
    }
};

typedef test_group<test_nodablearcstring_data> group;
typedef group::object object;

group test_nodablearcstring_group("geos::noding::NodableArcString");

template<>
template<>
void object::test<1>()
{
    set_test_name("CW half-circle, upper half-plane");

    CircularArc in(CoordinateXY{-5, 0}, CoordinateXY{0, 5}, CoordinateXY{5, 0});

    std::vector<CoordinateXY> coords;
    coords.emplace_back(4, 3);
    coords.emplace_back(3, 4);
    coords.emplace_back(-3, 4);
    coords.emplace_back(-4, 3);

    std::vector<CircularArc> expected;
    expected.push_back(CircularArc({-5, 0}, {-4, 3}, {0, 0}, 5, Orientation::CLOCKWISE));
    expected.push_back(CircularArc({-4, 3}, {-3, 4}, {0, 0}, 5, Orientation::CLOCKWISE));
    expected.push_back(CircularArc({-3, 4}, {3, 4}, {0, 0}, 5, Orientation::CLOCKWISE));
    expected.push_back(CircularArc({3, 4}, {4, 3}, {0, 0}, 5, Orientation::CLOCKWISE));
    expected.push_back(CircularArc({4, 3}, {5, 0}, {0, 0}, 5, Orientation::CLOCKWISE));

    test_add_points(in, coords, expected);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("CW half-circle, right half-plane");

    CircularArc in(CoordinateXY{0, 5}, CoordinateXY{5, 0}, CoordinateXY{0, -5});

    std::vector<CoordinateXY> coords;
    coords.emplace_back(4, -3);
    coords.emplace_back(4, 3);
    coords.emplace_back(3, -4);
    coords.emplace_back(3, 4);
    coords.emplace_back(5, 0);

    std::vector<CircularArc> expected;
    expected.push_back(CircularArc({0, 5}, {3, 4}, {0, 0}, 5, Orientation::CLOCKWISE));
    expected.push_back(CircularArc({3, 4}, {4, 3}, {0, 0}, 5, Orientation::CLOCKWISE));
    expected.push_back(CircularArc({4, 3}, {5, 0}, {0, 0}, 5, Orientation::CLOCKWISE));
    expected.push_back(CircularArc({5, 0}, {4, -3}, {0, 0}, 5, Orientation::CLOCKWISE));
    expected.push_back(CircularArc({4, -3}, {3, -4}, {0, 0}, 5, Orientation::CLOCKWISE));
    expected.push_back(CircularArc({3, -4}, {0, -5}, {0, 0}, 5, Orientation::CLOCKWISE));

    test_add_points(in, coords, expected);
}

}