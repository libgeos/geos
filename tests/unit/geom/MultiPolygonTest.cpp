//
// Test Suite for geos::geom::MultiPolygon class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiPolygon.h>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_multipolygon_data {
    test_multipolygon_data() {}
};

typedef test_group<test_multipolygon_data> group;
typedef group::object object;

group test_multipolygon_group("geos::geom::MultiPolygon");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    // getCoordinate() returns nullptr for empty geometry
    auto gf = geos::geom::GeometryFactory::create();
    std::unique_ptr<geos::geom::Geometry> g(gf->createMultiPolygon());

    ensure(g->getCoordinate() == nullptr);
}

} // namespace tut
