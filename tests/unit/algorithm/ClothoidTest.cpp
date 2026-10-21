#include <tut/tut.hpp>

#include <geos/algorithm/Clothoid.h>
#include <geos/constants.h>

#include <cmath>
#include <vector>

using geos::algorithm::clothoid::LengthSolve;
using geos::algorithm::clothoid::Moments;
using geos::algorithm::clothoid::Parameters;
using geos::algorithm::clothoid::fresnelC2;
using geos::algorithm::clothoid::fresnelS2;
using geos::algorithm::clothoid::heading;
using geos::algorithm::clothoid::linearizeSegmentCount;
using geos::algorithm::clothoid::moments;
using geos::algorithm::clothoid::placeOnChord;
using geos::algorithm::clothoid::position;
using geos::algorithm::clothoid::solveLengthHalley;

namespace tut {

struct test_clothoid_algorithm_data {
    static Parameters make(double x0, double y0, double theta0, double k0, double k1, double length)
    {
        Parameters curve;
        curve.x0 = x0;
        curve.y0 = y0;
        curve.theta0 = theta0;
        curve.k0 = k0;
        curve.k1 = k1;
        curve.length = length;
        return curve;
    }
};

typedef test_group<test_clothoid_algorithm_data> group;
typedef group::object object;

group test_clothoid_algorithm_group("geos::algorithm::Clothoid");

template<>
template<>
void object::test<1>()
{
    set_test_name("Fresnel C2 and S2");

    ensure_equals("value", fresnelC2(0.0), 0.0, 0.0);
    ensure_equals("value", fresnelS2(0.0), 0.0, 0.0);
    ensure_equals("value", fresnelC2(1.0), 0.97528768820034462, 1e-12);
    ensure_equals("value", fresnelS2(1.0), 0.16371404737570058, 1e-12);
    ensure_equals("value", fresnelC2(2.5), 0.94406391475511975, 1e-12);
    ensure_equals("value", fresnelS2(2.5), 1.265427786845702, 1e-12);
    ensure_equals("value", fresnelC2(-1.0), -fresnelC2(1.0), 1e-15);
    ensure_equals("value", fresnelS2(-2.5), -fresnelS2(2.5), 1e-12);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("Fresnel derivative past the series window");

    const double x = 8.0;
    const double h = 1e-4;
    const double slope = (fresnelC2(x + h) - fresnelC2(x - h)) / (2.0 * h);
    ensure_equals("value", slope, std::cos(0.5 * x * x), 1e-6);
    ensure(std::abs(fresnelC2(x)) < 2.0);
    ensure(std::abs(fresnelS2(x)) < 2.0);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("Halley on a straight chord");

    const LengthSolve solved = solveLengthHalley(0.0, 0.0, 3.0, 4.0, 0.0, 0.0);
    ensure(solved.converged);
    ensure_equals("value", solved.length, 5.0, 1e-12);
    ensure(solved.iterations >= 1);
    ensure(solved.iterations <= 4);

    const Moments m = moments(5.0, 0.0, 0.0);
    ensure_equals("value", m.P, 1.0, 1e-12);
    ensure_equals("value", m.Q, 0.0, 1e-12);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("Halley length on sample chords");

    struct Row {
        double x0, y0, x1, y1, k0, k1, length;
        int iterations;
    };
    const Row rows[] = {
        {166473.5960999988, 577235.5036999993, 166558.2337500006, 577204.9115500003,
            -0.0006369426751592356, 0.0, 90.0000162557141, 2},
        {168628.435899999, 576434.191300001, 168732.08165000007, 576397.3768999986,
            0.0, 0.0008333333333333334, 110.00000351764831, 3},
        {169248.6097499989, 576352.0681000017, 169359.7508499995, 576372.4052999988,
            0.0009174311926605505, 0.0, 112.99997847975716, 3},
        {81743.29825000092, 432271.9312000014, 81742.06329999864, 432281.16149999946,
            -0.0006864599999984842, -0.01, 9.313557096931358, 3},
        {108521.3913500011, 402895.7977499999, 108736.81419999897, 402360.2608500011,
            0.0, -0.00016656950112434413, 577.2999677954119, 3},
        {83095.08963999897, 432335.26976000145, 83125.17505500093, 432346.17217500135,
            0.0, -0.00046189376443418013, 31.99999718713569, 2},
        {187980.50636800006, 334494.60252099857, 187980.5104949996, 334494.6085780002,
            0.0007709304353511217, 0.0007710100231303007, 0.0073293515463363, 1},
        {182304.4404870011, 487151.59341400117, 182304.5242430009, 487151.6269589998,
            0.0, -1.1188492063491937e-06, 0.09022380193295569, 1},
        {117433.61793500185, 491277.6194499992, 117443.55124500021, 491276.4784500003,
            0.011111111111111112, 0.0, 9.999998005795014, 3},
        {148021.3672059998, 407680.2004280016, 147957.31304699928, 407657.4183260016,
            -0.002072538860103627, 0.0, 68.0000000192446, 3},
    };

    for (const Row& row : rows) {
        const LengthSolve solved = solveLengthHalley(row.x0, row.y0, row.x1, row.y1, row.k0, row.k1);
        ensure(solved.converged);
        ensure_equals("value", solved.length, row.length, 1e-6);
        ensure(solved.iterations <= 4);
        ensure_equals(solved.iterations, row.iterations);

        const auto placed = placeOnChord(row.x0, row.y0, row.x1, row.y1, row.k0, row.k1);
        ensure(placed.solve.converged);
        double x = 0.0;
        double y = 0.0;
        position(placed.parameters, placed.parameters.length, x, y);
        ensure_equals("value", x, row.x1, 1e-4);
        ensure_equals("value", y, row.y1, 1e-4);
    }
}

template<>
template<>
void object::test<5>()
{
    set_test_name("Fresnel end point and chord deviation");

    const Parameters curve = make(0.0, 0.0, 0.0, 0.0, 0.01, 25.0);
    ensure_equals("value", heading(curve, 25.0), 0.5 * 0.01 * 25.0, 1e-15);

    double x = 0.0;
    double y = 0.0;
    position(curve, 25.0, x, y);
    ensure_equals("value", x, 24.960965746830126, 1e-9);
    ensure_equals("value", y, 1.0405046700886817, 1e-9);

    const double kappaMax = 0.01;
    const double deviation = 0.01;
    const std::size_t n = linearizeSegmentCount(curve, geos::DoubleInfinity, deviation);
    const double tol = kappaMax * (curve.length / static_cast<double>(n))
        * (curve.length / static_cast<double>(n)) / 8.0;
    ensure(n >= 2);
    ensure(tol <= deviation + 1e-12);

    for (std::size_t i = 0; i < n; ++i) {
        const double s0 = curve.length * static_cast<double>(i) / static_cast<double>(n);
        const double s1 = curve.length * static_cast<double>(i + 1) / static_cast<double>(n);
        const double sm = 0.5 * (s0 + s1);
        double x0, y0, x1, y1, xm, ym;
        position(curve, s0, x0, y0);
        position(curve, s1, x1, y1);
        position(curve, sm, xm, ym);
        const double dx = x1 - x0;
        const double dy = y1 - y0;
        const double len2 = dx * dx + dy * dy;
        double distance = 0.0;
        if (len2 == 0.0) {
            distance = std::hypot(xm - x0, ym - y0);
        } else {
            const double t = std::max(0.0, std::min(1.0, ((xm - x0) * dx + (ym - y0) * dy) / len2));
            distance = std::hypot(xm - (x0 + t * dx), ym - (y0 + t * dy));
        }
        ensure(distance <= tol + 1e-9);
    }
}

template<>
template<>
void object::test<6>()
{
    set_test_name("constant curvature is the circular antiderivative");

    const double length = geos::MATH_PI / 2.0;
    const Parameters curve = make(0.0, 0.0, 0.0, 1.0, 1.0, length);
    double x = 0.0;
    double y = 0.0;
    position(curve, length, x, y);
    ensure_equals("value", x, 1.0, 1e-12);
    ensure_equals("value", y, 1.0, 1e-12);
    ensure_equals(linearizeSegmentCount(make(0.0, 0.0, 0.0, 0.0, 0.0, 10.0), 1.0, 1.0), 1u);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("zero chord is the degenerate Halley root");

    const LengthSolve solved = solveLengthHalley(1.0, 2.0, 1.0, 2.0, 0.01, -0.02);
    ensure(solved.converged);
    ensure_equals("value", solved.length, 0.0, 0.0);
    ensure_equals(solved.iterations, 0);
}

} // namespace tut
