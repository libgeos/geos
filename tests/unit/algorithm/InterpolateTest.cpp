#include <tut/tut.hpp>
// geos
#include <geos/geom/Coordinate.h>
#include <geos/algorithm/Interpolate.h>

namespace tut {

// dummy data, not used
struct test_interpolate_data {};

typedef test_group<test_interpolate_data> group;
typedef group::object object;

group test_interpolate_data("geos::algorithm::Interpolate");

using geos::algorithm::Interpolate;
using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYZM;
using geos::geom::CoordinateXYM;

// Interpolate Z between XYZ coordinates
template<>
template<>
void object::test<1>
()
{
    Coordinate p0(0, 0);
    Coordinate p1(1, 1);

    CoordinateXY q(0.8, 0.8);

    // Interpolate between NaN and NaN
    double z = Interpolate::zInterpolate(q, p0, p1);
    ensure("NaN-NaN", std::isnan(z));

    // Interpolate between NaN and a value
    p1.z = 5;
    z = Interpolate::zInterpolate(q, p0, p1);
    ensure_equals("NaN-value", z, 5);

    p0.z = 0;
    z = Interpolate::zInterpolate(q, p0, p1);
    ensure_equals("value-value", z, 4);
}

// Interpolate M between XYM coordinates
template<>
template<>
void object::test<2>
()
{
    CoordinateXYM p0({0, 0});
    CoordinateXYM p1({1, 1});

    CoordinateXY q(0.8, 0.8);

    // Interpolate between NaN and NaN
    double m = Interpolate::mInterpolate(q, p0, p1);
    ensure("NaN-NaN", std::isnan(m));

    // Interpolate between a value and NaN
    p0.m = 5;
    m = Interpolate::mInterpolate(q, p0, p1);
    ensure_equals("NaN-value", m, 5);

    p1.m = 0;
    m = Interpolate::mInterpolate(q, p0, p1);
    ensure_equals("value-value", m, 1);
}

// zGet, mGet (Coordinate, Coordinate)
template<>
template<>
void object::test<3>
()
{
    Coordinate p0(0, 0);
    Coordinate p1(0, 0, 7);

    ensure_equals(Interpolate::zGet(p0, p1), 7);
    ensure(std::isnan(Interpolate::mGet(p0, p1)));

    p0.z = 6;
    ensure_equals(Interpolate::zGet(p0, p1), 6);
    ensure(std::isnan(Interpolate::mGet(p0, p1)));
}

// zGet, mGet (CoordinateXY, CoordinateXY)
template<>
template<>
void object::test<4>
()
{
    CoordinateXY p0(0, 0);
    CoordinateXY p1(0, 0);

    ensure(std::isnan(Interpolate::mGet(p0, p1)));
    ensure(std::isnan(Interpolate::mGet(p0, p1)));
}

// Interpolate Z, M between two segment
template<>
template<>
void object::test<5>
()
{
    CoordinateXYM p0(0,   0, 0);
    CoordinateXYM p1(10, 10, 10);

    CoordinateXYZM q0(3, 0,  10, 100);
    CoordinateXYZM q1(3, 10, 20, 200);

    CoordinateXY p(3, 3);

    // both segments have m values, so we take the average
    ensure_equals(Interpolate::mInterpolate(p, p0, p1, q0, q1),
                  0.5*(Interpolate::mInterpolate(p, p0, p1) + Interpolate::mInterpolate(p, q0, q1)));

    // only segment q0-q1 has z values, so we use those
    ensure_equals(Interpolate::zInterpolate(p, p0, p1, q0, q1),
                  Interpolate::zInterpolate(p, q0, q1));
}

} // namespace tut

