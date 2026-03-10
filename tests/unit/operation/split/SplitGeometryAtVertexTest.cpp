#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>
#include <utility.h>

#include <geos/geom/CircularString.h>
#include <geos/geom/LineString.h>
#include <geos/operation/split/SplitGeometryAtVertex.h>
#include <geos/io/WKTReader.h>

using geos::geom::CircularString;
using geos::geom::LineString;
using geos::operation::split::SplitGeometryAtVertex;

namespace tut {

struct test_splitgeometryatvertex_data {
    const geos::io::WKTReader reader_;
};

typedef test_group<test_splitgeometryatvertex_data, 255> group;
typedef group::object object;

group test_splitgeometryatvertextest_group("geos::operation::split::SplitGeometryAtVertex");

template<>
template<>
void object::test<1>()
{
    set_test_name("LineString");

    auto input = reader_.read<LineString>("LINESTRING ZM (0 3 2 3, 5 8 3 4, 2 2 4 5, 6 1 5 6)");

    {
        auto splitAtStart = SplitGeometryAtVertex::splitSimpleCurveAtVertex(*input, 0);

        ensure(splitAtStart.first->isEmpty());
        ensure(splitAtStart.first->hasZ());
        ensure(splitAtStart.first->hasM());
        ensure(splitAtStart.second->equalsIdentical(input.get()));
    }

    {
        auto splitAtEnd = SplitGeometryAtVertex::splitSimpleCurveAtVertex(*input, input->getNumPoints() - 1);

        ensure(splitAtEnd.first->equalsIdentical(input.get()));
        ensure(splitAtEnd.second->isEmpty());
        ensure(splitAtEnd.second->hasZ());
        ensure(splitAtEnd.second->hasM());
    }

    {
        auto splitInMiddle = SplitGeometryAtVertex::splitSimpleCurveAtVertex(*input, 2);

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
    set_test_name("CircularString");

    auto input = reader_.read<CircularString>("CIRCULARSTRING ZM (-5 0 1 2, 0 5 2 3, 5 0 3 4, 10 -5 4 5, 15 0 5 6)");

    {
        auto splitAtStart = SplitGeometryAtVertex::splitSimpleCurveAtVertex(*input, 0);

        ensure(splitAtStart.first->isEmpty());
        ensure(splitAtStart.first->hasZ());
        ensure(splitAtStart.first->hasM());
        ensure(splitAtStart.second->equalsIdentical(input.get()));
    }

    {
        auto splitAtEnd = SplitGeometryAtVertex::splitSimpleCurveAtVertex(*input, input->getNumPoints() - 1);

        ensure(splitAtEnd.first->equalsIdentical(input.get()));
        ensure(splitAtEnd.second->isEmpty());
        ensure(splitAtEnd.second->hasZ());
        ensure(splitAtEnd.second->hasM());
    }

    {
        auto splitInMiddle = SplitGeometryAtVertex::splitSimpleCurveAtVertex(*input, 2);

        auto expectedFirst = reader_.read("CIRCULARSTRING ZM (-5 0 1 2, 0 5 2 3, 5 0 3 4)");
        auto expectedSecond = reader_.read("CIRCULARSTRING ZM (5 0 3 4, 10 -5 4 5, 15 0 5 6)");

        ensure(splitInMiddle.first->equalsIdentical(expectedFirst.get()));
        ensure(splitInMiddle.second->equalsIdentical(expectedSecond.get()));
    }

    ensure_THROW(SplitGeometryAtVertex::splitSimpleCurveAtVertex(*input, 1), geos::util::IllegalArgumentException);
}

}
