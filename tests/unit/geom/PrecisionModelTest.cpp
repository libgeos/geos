//
// Test Suite for geos::geom::PrecisionModel class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Coordinate.h>
#include <geos/math/DD.h>
// std
#include <random>
#include <vector>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_precisionmodel_data {
    typedef geos::geom::PrecisionModel PrecisionModel;
    typedef geos::geom::Coordinate Coordinate;
    typedef geos::math::DD DD;
    test_precisionmodel_data() {}

    void
    preciseCoordinateTester(const PrecisionModel& pm,
                            double x1, double y1,
                            double x2, double y2)
    {
        Coordinate p(x1, y1);

        pm.makePrecise(p);

        Coordinate pPrecise(x2, y2);
        ensure(p.equals2D(pPrecise));
    }


};

typedef test_group<test_precisionmodel_data> group;
typedef group::object object;

group test_precisionmodel_group("geos::geom::PrecisionModel");

//
// Test Cases
//

// Test of default constructor
template<>
template<>
void object::test<1>
()
{
    PrecisionModel pm;
    ensure(pm.isFloating());
    ensure_equals(pm.getMaximumSignificantDigits(), 16);
    ensure_equals(pm.getScale(), 0);
}

// Test FLOAT_SINGLE model
template<>
template<>
void object::test<2>
()
{
    PrecisionModel pm(PrecisionModel::FLOATING_SINGLE);
    ensure(pm.isFloating());
    ensure_equals(pm.getType(), PrecisionModel::FLOATING_SINGLE);
    ensure_equals(pm.getMaximumSignificantDigits(), 6);
}

// Test default FIXED model
template<>
template<>
void object::test<3>
()
{
    PrecisionModel pm(PrecisionModel::FIXED);
    ensure(!pm.isFloating());
    ensure_equals(pm.getType(), PrecisionModel::FIXED);
    ensure_equals(pm.getMaximumSignificantDigits(), 0);
}

// Test maximum significant digits (1:0)
template<>
template<>
void object::test<4>
()
{
    PrecisionModel pm(1);
    ensure(!pm.isFloating());
    ensure_equals(pm.getType(), PrecisionModel::FIXED);
    ensure_equals(pm.getMaximumSignificantDigits(), 0);
}

// Test maximum significant digits (10:1)
template<>
template<>
void object::test<5>
()
{
    PrecisionModel pm(10);
    ensure_equals(pm.getType(), PrecisionModel::FIXED);
    ensure_equals(pm.getMaximumSignificantDigits(), 1);
}

// Test maximum significant digits (1000:3)
template<>
template<>
void object::test<6>
()
{
    PrecisionModel pm(1000);
    ensure_equals(pm.getType(), PrecisionModel::FIXED);
    ensure_equals(pm.getMaximumSignificantDigits(), 3);
}

// Test maximum significant digits (0.1:-1)
template<>
template<>
void object::test<7>
()
{
    PrecisionModel pm(0.1);
    ensure_equals(pm.getType(), PrecisionModel::FIXED);
    ensure_equals(pm.getMaximumSignificantDigits(), -1);
}

// Test maximum significant digits (0.001:-3)
template<>
template<>
void object::test<8>
()
{
    PrecisionModel pm(0.001);
    ensure_equals(pm.getType(), PrecisionModel::FIXED);
    ensure_equals(pm.getMaximumSignificantDigits(), -3);
}

// Test makePrecise
template<>
template<>
void object::test<9>
()
{
    PrecisionModel pm_10(0.1);
    preciseCoordinateTester(pm_10, 1200.4, 1240.4, 1200, 1240);
    preciseCoordinateTester(pm_10, 1209.4, 1240.4, 1210, 1240);
}

// makePrecise of a double-double value applies the rounding rule of
// makePrecise(double). Away from half-cell boundaries both return the same
// grid value, for every type of model and for scales and grid sizes.
template<>
template<>
void object::test<10>
()
{
    set_test_name("makePrecise of a double-double agrees with a double away from ties");
    std::vector<PrecisionModel> models = {
        PrecisionModel(1e12), PrecisionModel(62500000000.0), PrecisionModel(1e8),
        PrecisionModel(1000.0), PrecisionModel(3.0), PrecisionModel(1.0),
        PrecisionModel(0.3), PrecisionModel(0.1), PrecisionModel(0.001),
        PrecisionModel(PrecisionModel::FIXED), PrecisionModel(),
        PrecisionModel(PrecisionModel::FLOATING_SINGLE)
    };
    std::mt19937_64 gen(1);
    std::uniform_int_distribution<long long> cellDist(-1000000000000LL, 1000000000000LL);
    // offsets from the cell centre, in cells, at least 0.05 from a half-cell boundary
    std::uniform_real_distribution<double> offsetDist(-0.45, 0.45);
    for (const auto& pm : models) {
        double cell = pm.isFloating() ? 1.0 : pm.getGridSize();
        for (int i = 0; i < 1000; i++) {
            double val = (static_cast<double>(cellDist(gen)) + offsetDist(gen)) * cell;
            double expected = pm.makePrecise(val);
            ensure_equals("double-double of a double", pm.makePrecise(DD(val)), expected, 0.0);
            ensure_equals("double-double beyond a double", pm.makePrecise(DD(val) + DD(val * 1e-20)), expected, 0.0);
        }
    }
}

// Half-cell ties round towards positive infinity for both kinds of value.
template<>
template<>
void object::test<11>
()
{
    set_test_name("makePrecise of a double-double rounds ties as a double");
    PrecisionModel pmScale(1.0);
    ensure_equals("grid value", pmScale.makePrecise(2.5), 3.0, 0.0);
    ensure_equals("grid value", pmScale.makePrecise(DD(2.5)), 3.0, 0.0);
    ensure_equals("grid value", pmScale.makePrecise(-2.5), -2.0, 0.0);
    ensure_equals("grid value", pmScale.makePrecise(DD(-2.5)), -2.0, 0.0);

    PrecisionModel pmGrid(0.1);
    ensure_equals("grid value", pmGrid.makePrecise(15.0), 20.0, 0.0);
    ensure_equals("grid value", pmGrid.makePrecise(DD(15.0)), 20.0, 0.0);
    ensure_equals("grid value", pmGrid.makePrecise(-15.0), -10.0, 0.0);
    ensure_equals("grid value", pmGrid.makePrecise(DD(-15.0)), -10.0, 0.0);
    ensure_equals("grid value", pmGrid.makePrecise(-1000000000000005.0), -1000000000000000.0, 0.0);
    ensure_equals("grid value", pmGrid.makePrecise(DD(-1000000000000005.0)), -1000000000000000.0, 0.0);
}

// A double-double value is rounded into its own cell where the double
// arithmetic of makePrecise(double) cannot find it.
template<>
template<>
void object::test<12>
()
{
    set_test_name("makePrecise of a double-double finds the cell of the value");
    // just below a half-cell boundary whose double is the boundary itself
    PrecisionModel pmScale(1.0);
    ensure_equals("grid value", pmScale.makePrecise(0.5), 1.0, 0.0);
    ensure_equals("grid value", pmScale.makePrecise(DD(0.5, -1e-20)), 0.0, 0.0);
    ensure_equals("grid value", pmScale.makePrecise(-0.5), 0.0, 0.0);
    ensure_equals("grid value", pmScale.makePrecise(DD(-0.5, -1e-20)), -1.0, 0.0);

    PrecisionModel pmGrid(0.1);
    ensure_equals("grid value", pmGrid.makePrecise(1005.0), 1010.0, 0.0);
    ensure_equals("grid value", pmGrid.makePrecise(DD(1005.0, -1e-20)), 1000.0, 0.0);

    // The double 377804.96362807497 lies below the half-cell boundary
    // 377804.963628075, but its product with the scale rounds onto the
    // boundary in double arithmetic.
    PrecisionModel pm(1e8);
    ensure_equals("grid value", pm.makePrecise(377804.96362807497), 377804.96362808, 0.0);
    ensure_equals("grid value", pm.makePrecise(DD(377804.96362807497)), 377804.96362807, 0.0);
}

// On a grid finer than a double, the cell index reaches 2^53 and is not
// exactly representable as a double. A cell of 1e-12 near 5e5 and 6e6 is
// far smaller than half the spacing of doubles there, so the nearest double
// to the grid value of a double's own cell is that double.
template<>
template<>
void object::test<13>
()
{
    set_test_name("makePrecise of a double-double keeps cells beyond 2^53");
    PrecisionModel pm(1e12);
    std::mt19937_64 gen(1);
    std::uniform_real_distribution<double> x(500000.0, 505000.0);
    std::uniform_real_distribution<double> y(6000000.0, 6005000.0);
    for (int i = 0; i < 1000; i++) {
        double vx = x(gen);
        double vy = y(gen);
        ensure_equals("x", pm.makePrecise(DD(vx)), vx, 0.0);
        ensure_equals("y", pm.makePrecise(DD(vy)), vy, 0.0);
    }
}

} // namespace tut

