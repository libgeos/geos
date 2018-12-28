//
// Test Suite for geos::geom::GeometryCollection class.

#include <tut/tut.hpp>
#include <utility.h>


namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_geometry_collection_data {
    typedef geos::geom::GeometryFactory GeometryFactory;

    geos::geom::PrecisionModel pm_;
    GeometryFactory::Ptr factory_;

    test_geometry_collection_data()
        : pm_(1000), factory_(GeometryFactory::create(&pm_, 0))
    {
    }
};

typedef test_group<test_geometry_collection_data> group;
typedef group::object object;

group test_geometry_collection_group("geos::geom::GeometryCollection");

//
// Test Cases
//

// Test of user's constructor to build empty Point
template<>
template<>
void object::test<1>()
{
    GeometryPtr empty_point = factory_->createPoint();
    ensure( empty_point != nullptr );

    geos::geom::Coordinate coord(1, 2);
    GeometryPtr point = factory_->createPoint(coord);
    ensure( point != nullptr );

    std::vector<GeometryPtr> geoms{empty_point, point};
    GeometryColPtr col = factory_->createGeometryCollection(geoms);
    ensure( col != nullptr );

    ensure( col->getCoordinate() != nullptr );
    ensure_equals( col->getCoordinate()->x, 1 );
    ensure_equals( col->getCoordinate()->y, 2 );
}

} // namespace tut
