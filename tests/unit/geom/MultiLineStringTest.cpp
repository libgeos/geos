//
// Test Suite for geos::geom::MultiLineString class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/MultiLineString.h>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_multilinestring_data {
    test_multilinestring_data() {}
};

typedef test_group<test_multilinestring_data> group;
typedef group::object object;

group test_multilinestring_group("geos::geom::MultiLineString");

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
    std::unique_ptr<geos::geom::Geometry> g(gf->createMultiLineString());

    ensure(g->getCoordinate() == nullptr);
}

} // namespace tut

