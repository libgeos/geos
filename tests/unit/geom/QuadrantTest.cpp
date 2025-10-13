#include <tut/tut.hpp>

#include <geos/geom/Quadrant.h>

using geos::geom::Quadrant;

namespace tut {

struct test_quadrant_data {

    static std::string quadrantName(int quadrant) {
        switch (quadrant) {
            case Quadrant::NE: return "NE";
                case Quadrant::NW: return "NW";
                case Quadrant::SE: return "SE";
                case Quadrant::SW: return "SW";
        }
        return "unknown";
    }

    static void checkQuadrant(double dx, double dy, int expected) {
        int quadrant = Quadrant::quadrant({0, 0}, {dx, dy});
        ensure_equals(quadrant, expected);
    }

    static void checkPseudoAngleMatchesQuadrant(double dx, double dy) {
        int quadrant = Quadrant::quadrant({0, 0}, {dx, dy});
        double pa = Quadrant::pseudoAngle({0, 0}, {dx, dy});

        if (quadrant != static_cast<int>(pa)) {
            std::stringstream ss;
            ss << "relative coordinate (" << dx << ", " << dy << ") expected to be in quadrant " << quadrantName(quadrant) << " but pseudoAngle " << pa << " corresponds to quadrant " << quadrantName(static_cast<int>(pa));
            fail(ss.str());
        }

        ensure_equals(quadrant, static_cast<int>(pa));
    }
};

typedef test_group<test_quadrant_data> group;
typedef group::object object;

group test_quadrant_group("geos::geom::Quadrant");

template<>
template<>
void object::test<1>()
{
    checkQuadrant(1, 1, Quadrant::NE);
    checkQuadrant(1, -1, Quadrant::SE);
    checkQuadrant(-1, -1, Quadrant::SW);
    checkQuadrant(-1, 1, Quadrant::NW);
}

template<>
template<>
void object::test<2>()
{
    static constexpr double eps = 1e-8;

    // center of each quadrant
    checkPseudoAngleMatchesQuadrant(1, 1);
    checkPseudoAngleMatchesQuadrant(1, -1);
    checkPseudoAngleMatchesQuadrant(-1, -1);
    checkPseudoAngleMatchesQuadrant(-1, 1);

    // near axes
    checkPseudoAngleMatchesQuadrant(1, eps); // +X
    checkPseudoAngleMatchesQuadrant(1, -eps); // +X
    checkPseudoAngleMatchesQuadrant(eps, 1); // +Y
    checkPseudoAngleMatchesQuadrant(-eps, 1); // +Y
    checkPseudoAngleMatchesQuadrant(-1, eps); // +X
    checkPseudoAngleMatchesQuadrant(-1, -eps); // +X
    checkPseudoAngleMatchesQuadrant(eps, -1); // -Y
    checkPseudoAngleMatchesQuadrant(-eps, -1); // -Y

    // axes
    checkPseudoAngleMatchesQuadrant(1, 0); // +X
    //checkPseudoAngleMatchesQuadrant(0, 1); // +Y
    //checkPseudoAngleMatchesQuadrant(-1, 0); // +X
    //checkPseudoAngleMatchesQuadrant(0, -1); // -Y
}
}
