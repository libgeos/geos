//
// Test Suite for geos::algorithm::Rectangle

#include <tut/tut.hpp>
// geos
#include <geos/algorithm/Rectangle.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
#include <geos/constants.h>
#include <utility.h>
// std
#include <string>
#include <memory>

using geos::algorithm::Rectangle;
using geos::geom::Geometry;
using geos::io::WKTReader;


namespace tut {
//
// Test Group
//

// dummy data, not used
struct test_rectangle_data {

    const double TOL = 1e-10;

    WKTReader reader_;

    test_rectangle_data() {};

    void
    checkRectangle(const std::string& wkt, const std::string& wktExpected)
    {

        std::unique_ptr<Geometry> geom = reader_.read(wkt);
        const LineString *line = static_cast<LineString*>(geom.get());

        const Coordinate& baseRightPt = line->getCoordinateN(0);
        const Coordinate& baseLeftPt = line->getCoordinateN(1);
        const Coordinate& leftSidePt = line->getCoordinateN(2);
        const Coordinate& oppositePt = line->getCoordinateN(3);
        const Coordinate& rightSidePt = line->getCoordinateN(4);
        std::unique_ptr<Polygon> actual = Rectangle::createFromSidePts(
            baseRightPt, baseLeftPt,
            oppositePt, leftSidePt,
            rightSidePt, line->getFactory());
        std::unique_ptr<Geometry> expected = reader_.read(wktExpected);
        ensure_equals_geometry(
            expected.get(),
            static_cast<Geometry*>(actual.get()),
            TOL);
    }

};

typedef test_group<test_rectangle_data> group;
typedef group::object object;

group test_rectangle_group("geos::algorithm::Rectangle");

//
// Test Cases
//

// testOrthogonal()
template<>
template<>
void object::test<1>()
{
    checkRectangle(
        "LINESTRING (9 1, 1 1, 0 5, 7 10, 10 6)",
        "POLYGON ((0 1, 0 10, 10 10, 10 1, 0 1))"
        );
}

// test45()
template<>
template<>
void object::test<2>()
{
    checkRectangle(
        "LINESTRING (10 5, 5 0, 2 1, 2 7, 9 9)",
        "POLYGON ((-1 4, 6.5 11.5, 11.5 6.5, 4 -1, -1 4))"
        );
}

// testCoincidentBaseSides()
template<>
template<>
void object::test<3>()
{
    checkRectangle(
        "LINESTRING (10 5, 7 0, 7 0, 2 7, 10 5)",
        "POLYGON ((0.2352941176470591 4.0588235294117645, 3.2352941176470598 9.058823529411764, 10 5, 7 0, 0.2352941176470591 4.0588235294117645))"
        );
}


} // namespace tut

