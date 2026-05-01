#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>
#include <utility.h>

#include <geos/geom/CircularString.h>
#include <geos/geom/LineString.h>
#include <geos/operation/split/SplitLinealAtPoint.h>
#include <geos/io/WKTReader.h>

using geos::geom::CoordinateXY;
using geos::geom::CircularString;
using geos::geom::LineString;
using geos::operation::split::SplitLinealAtPoint;

namespace tut {

struct test_splitlinealatpoint_data {
    const geos::io::WKTReader reader_;
};

typedef test_group<test_splitlinealatpoint_data, 255> group;
typedef group::object object;

group test_splitlinealatpointtest_group("geos::operation::split::SplitLinealAtPoint");

template<>
template<>
void object::test<1>()
{
    set_test_name("Split LineString ZM at vertex");

    auto input = reader_.read<LineString>("LINESTRING ZM (0 3 2 3, 5 8 3 4, 2 2 4 5, 6 1 5 6)");

    {
        auto splitAtStart = SplitLinealAtPoint::splitSimpleCurveAtVertex(*input, 0);

        ensure(splitAtStart.first->isEmpty());
        ensure(splitAtStart.first->hasZ());
        ensure(splitAtStart.first->hasM());
        ensure(splitAtStart.second->equalsIdentical(input.get()));
    }

    {
        auto splitAtEnd = SplitLinealAtPoint::splitSimpleCurveAtVertex(*input, input->getNumPoints() - 1);

        ensure(splitAtEnd.first->equalsIdentical(input.get()));
        ensure(splitAtEnd.second->isEmpty());
        ensure(splitAtEnd.second->hasZ());
        ensure(splitAtEnd.second->hasM());
    }

    {
        auto splitInMiddle = SplitLinealAtPoint::splitSimpleCurveAtVertex(*input, 2);

        auto expectedFirst = reader_.read("LINESTRING ZM (0 3 2 3, 5 8 3 4, 2 2 4 5)");
        auto expectedSecond = reader_.read("LINESTRING ZM (2 2 4 5, 6 1 5 6)");

        ensure(splitInMiddle.first->equalsIdentical(expectedFirst.get()));
        ensure(splitInMiddle.second->equalsIdentical(expectedSecond.get()));
    }
}

template<>
template<>
void object::test<2>()
{
    set_test_name("Split CircularString ZM at vertex");

    auto input = reader_.read<CircularString>("CIRCULARSTRING ZM (-5 0 1 2, 0 5 2 3, 5 0 3 4, 10 -5 4 5, 15 0 5 6)");

    {
        auto splitAtStart = SplitLinealAtPoint::splitSimpleCurveAtVertex(*input, 0);

        ensure(splitAtStart.first->isEmpty());
        ensure(splitAtStart.first->hasZ());
        ensure(splitAtStart.first->hasM());
        ensure(splitAtStart.second->equalsIdentical(input.get()));
    }

    {
        auto splitAtEnd = SplitLinealAtPoint::splitSimpleCurveAtVertex(*input, input->getNumPoints() - 1);

        ensure(splitAtEnd.first->equalsIdentical(input.get()));
        ensure(splitAtEnd.second->isEmpty());
        ensure(splitAtEnd.second->hasZ());
        ensure(splitAtEnd.second->hasM());
    }

    {
        auto splitInMiddle = SplitLinealAtPoint::splitSimpleCurveAtVertex(*input, 2);

        auto expectedFirst = reader_.read("CIRCULARSTRING ZM (-5 0 1 2, 0 5 2 3, 5 0 3 4)");
        auto expectedSecond = reader_.read("CIRCULARSTRING ZM (5 0 3 4, 10 -5 4 5, 15 0 5 6)");

        ensure(splitInMiddle.first->equalsIdentical(expectedFirst.get()));
        ensure(splitInMiddle.second->equalsIdentical(expectedSecond.get()));
    }

    ensure_THROW(SplitLinealAtPoint::splitSimpleCurveAtVertex(*input, 1), geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("Split LineString ZM at new point");

    auto input = reader_.read<LineString>("LINESTRING ZM (0 3 2 3, 5 8 3 4, 2 2 4 5, 6 1 5 6)");

    CoordinateXY pt{2, 3};

    ensure_THROW(SplitLinealAtPoint::splitLineStringAtPoint(*input, 3, pt), geos::util::IllegalArgumentException);

    // Split first segment
    {
        auto [first, second] = SplitLinealAtPoint::splitLineStringAtPoint(*input, 0, pt);

        auto expectedFirst = reader_.read("LINESTRING ZM (0 3 2 3, 2 3 2.282842712474619 3.282842712474619)");
        auto expectedSecond = reader_.read("LINESTRING ZM (2 3 2.282842712474619 3.282842712474619, 5 8 3 4, 2 2 4 5, 6 1 5 6)");

        ensure_equals_exact_geometry_xyzm(first.get(), expectedFirst.get(), 0.0);
        ensure_equals_exact_geometry_xyzm(second.get(), expectedSecond.get(), 0.0);
    }

    // Split second segment
    {
        auto [first, second] = SplitLinealAtPoint::splitLineStringAtPoint(*input, 1, pt);

        auto expectedFirst = reader_.read("LINESTRING ZM (0 3 2 3, 5 8 3 4, 2 3 3.8692269873603533 4.869226987360353)");
        auto expectedSecond = reader_.read("LINESTRING ZM (2 3 3.8692269873603533 4.869226987360353, 2 2 4 5, 6 1 5 6)");

        ensure_equals_exact_geometry_xyzm(first.get(), expectedFirst.get(), 0.0);
        ensure_equals_exact_geometry_xyzm(second.get(), expectedSecond.get(), 0.0);
    }

}

template<>
template<>
void object::test<4>()
{
    set_test_name("Split CompoundCurve ZM at existing vertices");

    auto input = reader_.read<geos::geom::CompoundCurve>("COMPOUNDCURVE ZM(CIRCULARSTRING ZM(2 8 1 2, 4 7 3 4, 1 9 5 6, 3 15 7 8, 8 16 9 10), (8 16 9 10, 8 10 11 12, 6 14 13 14, 4 12 15 16), CIRCULARSTRING (4 12 15 16, 4 10 17 18, 2 8 19 20))");

    // Split at first point
    {
        auto [first, second] = SplitLinealAtPoint::splitCompoundCurveAtPoint(*input, 0, 0, CoordinateXY{2, 8});

        ensure(first->isEmpty());
        ensure(second->equalsIdentical(input.get()));
    }

    // Split at intermediate point of first curve
    {
        auto [first, second] = SplitLinealAtPoint::splitCompoundCurveAtPoint(*input, 0, 2, CoordinateXY{1, 9});

        auto expectedFirst = reader_.read("CIRCULARSTRING ZM (2 8 1 2, 4 7 3 4, 1 9 5 6)");
        ensure_equals_exact_geometry_xyzm(first.get(), expectedFirst.get(), 0.0);

        auto expectedSecond = reader_.read("COMPOUNDCURVE ZM (CIRCULARSTRING ZM(1 9 5 6, 3 15 7 8, 8 16 9 10), (8 16 9 10, 8 10 11 12, 6 14 13 14, 4 12 15 16), CIRCULARSTRING (4 12 15 16, 4 10 17 18, 2 8 19 20))");
        ensure_equals_exact_geometry_xyzm(second.get(), expectedSecond.get(), 0.0);
    }

    // Split at last point of first curve
    {
        auto [first, second] = SplitLinealAtPoint::splitCompoundCurveAtPoint(*input, 0, 2, CoordinateXY{8, 16});

        ensure(first->equalsIdentical(input->getCurveN(0)));

        auto expectedSecond = reader_.read("COMPOUNDCURVE ZM ((8 16 9 10, 8 10 11 12, 6 14 13 14, 4 12 15 16), CIRCULARSTRING (4 12 15 16, 4 10 17 18, 2 8 19 20))");
        ensure_equals_exact_geometry_xyzm(second.get(), expectedSecond.get(), 0.0);
    }

    // Split at first point of second curve
    {
        auto [first, second] = SplitLinealAtPoint::splitCompoundCurveAtPoint(*input, 1, 0, CoordinateXY{8, 16});

        ensure(first->equalsIdentical(input->getCurveN(0)));

        auto expectedSecond = reader_.read("COMPOUNDCURVE ZM ((8 16 9 10, 8 10 11 12, 6 14 13 14, 4 12 15 16), CIRCULARSTRING (4 12 15 16, 4 10 17 18, 2 8 19 20))");
        ensure_equals_exact_geometry_xyzm(second.get(), expectedSecond.get(), 0.0);
    }

    // Split at intermate point of second curve
    {
        auto [first, second] = SplitLinealAtPoint::splitCompoundCurveAtPoint(*input, 1, 1, CoordinateXY{6, 14});

        auto expectedFirst = reader_.read("COMPOUNDCURVE ZM (CIRCULARSTRING ZM(2 8 1 2, 4 7 3 4, 1 9 5 6, 3 15 7 8, 8 16 9 10), (8 16 9 10, 8 10 11 12, 6 14 13 14))");
        ensure_equals_exact_geometry_xyzm(first.get(), expectedFirst.get(), 0.0);

        auto expectedSecond = reader_.read("COMPOUNDCURVE ((6 14 13 14, 4 12 15 16), CIRCULARSTRING (4 12 15 16, 4 10 17 18, 2 8 19 20))");
        ensure_equals_exact_geometry_xyzm(second.get(), expectedSecond.get(), 0.0);
    }

    // Split at last point of second curve
    {
        auto [first, second] = SplitLinealAtPoint::splitCompoundCurveAtPoint(*input, 1, 3, CoordinateXY{4, 12});

        auto expectedFirst = reader_.read("COMPOUNDCURVE (CIRCULARSTRING(2 8 1 2, 4 7 3 4, 1 9 5 6, 3 15 7 8, 8 16 9 10), (8 16 9 10, 8 10 11 12, 6 14 13 14, 4 12 15 16))");
        ensure_equals_exact_geometry_xyzm(first.get(), expectedFirst.get(), 0.0);

        ensure(second->equalsIdentical(input->getCurveN(2)));
    }

    // Split at first point of third curve
    {
        auto [first, second] = SplitLinealAtPoint::splitCompoundCurveAtPoint(*input, 2, 0, CoordinateXY{4, 12});

        auto expectedFirst = reader_.read("COMPOUNDCURVE (CIRCULARSTRING(2 8 1 2, 4 7 3 4, 1 9 5 6, 3 15 7 8, 8 16 9 10), (8 16 9 10, 8 10 11 12, 6 14 13 14, 4 12 15 16))");
        ensure_equals_exact_geometry_xyzm(first.get(), expectedFirst.get(), 0.0);

        ensure(second->equalsIdentical(input->getCurveN(2)));
    }

    // Split at last point of third curve
    {
        auto [first, second] = SplitLinealAtPoint::splitCompoundCurveAtPoint(*input, 2, 2, CoordinateXY{2, 8});

        ensure(first->equalsIdentical(input.get()));
        ensure(second->isEmpty());
    }

}

template<>
template<>
void object::test<5>()
{
    set_test_name("Split CompoundCurve ZM at new point");

    auto input = reader_.read<geos::geom::CompoundCurve>("COMPOUNDCURVE ZM(CIRCULARSTRING ZM(2 8 1 2, 4 7 3 4, 1 9 5 6, 3 15 7 8, 8 16 9 10), (8 16 9 10, 8 10 11 12, 6 14 13 14, 4 12 15 16), CIRCULARSTRING (4 12 15 16, 4 10 17 18, 2 8 19 20))");

    auto [first, second] = SplitLinealAtPoint::splitCompoundCurveAtPoint(*input, 0, 2, CoordinateXY{5, 16});

    auto expectedFirst = reader_.read("CIRCULARSTRING ZM (2 8 1 2, 4 7 3 4, 1 9 5 6, 1.5502525316941682 13.328427124746192 6.33570524800221 7.33570524800221, 5 16 7.851489605544531 8.851489605544531)");
    ensure_equals_exact_geometry_xyzm(first.get(), expectedFirst.get(), 0.0);

    auto expectedSecond = reader_.read("COMPOUNDCURVE ZM (CIRCULARSTRING ZM (5 16 7.851489605544531 8.851489605544531, 6.5 16.20087712549569 8.425744802772266 9.425744802772266, 8 16 9 10), (8 16 9 10, 8 10 11 12, 6 14 13 14, 4 12 15 16), CIRCULARSTRING ZM (4 12 15 16, 4 10 17 18, 2 8 19 20)) ");
    ensure_equals_exact_geometry_xyzm(second.get(), expectedSecond.get(), 0.0);
}

}
