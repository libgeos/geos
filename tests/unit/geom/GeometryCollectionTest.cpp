//
// Test Suite for geos::geom::GeometryCollection class.

#include <tut/tut.hpp>
#include <utility.h>

#include <memory>


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
void object::test<1>
()
{
    using geos::geom::Geometry;

    std::unique_ptr<Geometry> empty_point(factory_->createPoint());
    ensure(empty_point != nullptr);

    geos::geom::Coordinate coord(1, 2);
    std::unique_ptr<Geometry> point(factory_->createPoint(coord));
    ensure(point != nullptr);

    std::vector<Geometry*> geoms{empty_point.get(), point.get()};
    std::unique_ptr<Geometry> col(factory_->createGeometryCollection(geoms));
    ensure(col != nullptr);

    ensure(col->getCoordinate() != nullptr);
    ensure_equals(col->getCoordinate()->x, 1);
    ensure_equals(col->getCoordinate()->y, 2);
}

// Test of default constructor
template<>
template<>
void object::test<2>
()
{
    using geos::geom::Geometry;

    geos::geom::PrecisionModel pm;
    auto gf = GeometryFactory::create(&pm, 1);
    std::unique_ptr<Geometry> g(gf->createEmptyGeometry());

    g->setSRID(0);
    std::vector<Geometry*> v = {g.get()};
    std::unique_ptr<Geometry> geom_col(gf->createGeometryCollection(v));
    ensure_equals(geom_col->getGeometryN(0)->getSRID(), 1);

    geom_col->setSRID(2);
    ensure_equals(geom_col->getGeometryN(0)->getSRID(), 2);

    std::unique_ptr<Geometry> clone(geom_col->clone());
    ensure_equals(clone->getGeometryN(0)->getSRID(), 2);
}
} // namespace tut
