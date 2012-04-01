// $Id$
// 
// Test Suite for geos::geom::PrecisionModel class.

// tut
#include <tut.hpp>
// geos
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/Coordinate.h>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_precisionmodel_data
    {
        typedef geos::geom::PrecisionModel PrecisionModel;
        typedef geos::geom::Coordinate Coordinate;
        test_precisionmodel_data() {}

        void preciseCoordinateTester(const PrecisionModel& pm,
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
    void object::test<1>()
    {
        PrecisionModel pm;
        ensure(pm.isFloating());
        ensure_equals(pm.getMaximumSignificantDigits(), 16);
        ensure_equals(pm.getScale(), 0);
    }

    // Test FLOAT_SINGLE model
    template<>
    template<>
    void object::test<2>()
    {
        PrecisionModel pm(PrecisionModel::FLOATING_SINGLE);
        ensure(pm.isFloating());
        ensure_equals(pm.getType(), PrecisionModel::FLOATING_SINGLE);
        ensure_equals(pm.getMaximumSignificantDigits(), 6);
    }

    // Test default FIXED model
    template<>
    template<>
    void object::test<3>()
    {
        PrecisionModel pm(PrecisionModel::FIXED);
        ensure(!pm.isFloating());
        ensure_equals(pm.getType(), PrecisionModel::FIXED);
        ensure_equals(pm.getMaximumSignificantDigits(), 0);
    }

    // Test maximum significant digits (1:0)
    template<>
    template<>
    void object::test<4>()
    {
        PrecisionModel pm(1);
        ensure(!pm.isFloating());
        ensure_equals(pm.getType(), PrecisionModel::FIXED);
        ensure_equals(pm.getMaximumSignificantDigits(), 0);
    }

    // Test maximum significant digits (10:1)
    template<>
    template<>
    void object::test<5>()
    {
        PrecisionModel pm(10);
        ensure_equals(pm.getType(), PrecisionModel::FIXED);
        ensure_equals(pm.getMaximumSignificantDigits(), 1);
    }

    // Test maximum significant digits (1000:3)
    template<>
    template<>
    void object::test<6>()
    {
        PrecisionModel pm(1000);
        ensure_equals(pm.getType(), PrecisionModel::FIXED);
        ensure_equals(pm.getMaximumSignificantDigits(), 3);
    }

    // Test maximum significant digits (0.1:-1)
    template<>
    template<>
    void object::test<7>()
    {
        PrecisionModel pm(0.1);
        ensure_equals(pm.getType(), PrecisionModel::FIXED);
        ensure_equals(pm.getMaximumSignificantDigits(), -1);
    }

    // Test maximum significant digits (0.001:-3)
    template<>
    template<>
    void object::test<8>()
    {
        PrecisionModel pm(0.001);
        ensure_equals(pm.getType(), PrecisionModel::FIXED);
        ensure_equals(pm.getMaximumSignificantDigits(), -3);
    }

    // Test makePrecise
    template<>
    template<>
    void object::test<9>()
    {
        PrecisionModel pm_10(0.1);
        preciseCoordinateTester(pm_10, 1200.4, 1240.4, 1200, 1240);
        preciseCoordinateTester(pm_10, 1209.4, 1240.4, 1210, 1240);
    }

} // namespace tut

