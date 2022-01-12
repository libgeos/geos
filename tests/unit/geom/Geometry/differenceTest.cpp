//
// Test Suite for geos::geom::Geometry::difference() method.

#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Point.h>
#include <geos/geom/LineString.h>
#include <geos/util.h>

namespace tut {
// dummy data, not used
struct test_geometry_difference_data {
    geos::io::WKTReader wktreader;
    geos::io::WKTWriter wktwriter;

    test_geometry_difference_data()
        :
        wktreader()
    {}
};

typedef test_group<test_geometry_difference_data> group;
typedef group::object object;

group test_geometry_difference_group("geos::geom::Geometry::difference");

//
// Test Cases


// This issue exercised a bug in MonotoneChainBuilder
// https://github.com/libgeos/geos/issues/290
template<>
template<>
void object::test<1>
()
{
    std::string wkt1("POLYGON((0 0, 0 2, 2 2, 2 0, 0 0))");
    std::unique_ptr<geos::geom::Geometry> g1(wktreader.read(wkt1));

    std::string wkt2("POLYGON((0.1 0.1, 4.0 0.1, 4.0 1.9, 0.1 1.9, 0.1 0.1))");
    std::unique_ptr<geos::geom::Geometry> g2(wktreader.read(wkt2));

    auto result = g2->difference(g1.get());
    result->normalize();

    std::string wktExpected("POLYGON ((2 0.1, 2 1.9, 4 1.9, 4 0.1, 2 0.1))");
    std::unique_ptr<geos::geom::Geometry> expected(wktreader.read(wktExpected));

    ensure_equals_geometry(expected.get(), result.get());
}


} // namespace tut
