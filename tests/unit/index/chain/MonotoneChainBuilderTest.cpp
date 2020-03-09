//
// Test Suite for geos::index::chain::MonotoneChainBuilder class.

#include <tut/tut.hpp>
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
struct test_monotonechainbuilder_data {
    geos::io::WKTReader wktreader;
    geos::io::WKTWriter wktwriter;

    test_monotonechainbuilder_data()
        :
        wktreader(),
        wktwriter()
    {}
};

typedef test_group<test_monotonechainbuilder_data> group;
typedef group::object object;

group test_monotonechainbuilder_group("geos::index::chain::MonotoneChainBuilder");

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

    // POLYGON ((2.0 1.9, 4.0 1.9, 4.0 0.1, 2.0 0.1, 2.0 1.9))
    double x[5] = {2.0, 4.0, 4.0, 2.0, 2.0};
    double y[5] = {1.9, 1.9, 0.1, 0.1, 1.9};

    auto g3 = g2->difference(g1.get());
    auto cs = g3->getCoordinates();

    // std::string wkt3 = wktwriter.write(g3.get());
    // std::cout << wkt3 << std::endl;

    for (int i = 0; i < 5; ++i)
    {
        // std::cout << csx << ", " << csy << std::endl;
        // std::cout << x[i] << ", " << y[i] << std::endl;
        ensure_equals("x", cs->getOrdinate(i, 0), x[i], 0.0001);
        ensure_equals("y", cs->getOrdinate(i, 1), y[i], 0.0001);
    }

}


} // namespace tut

