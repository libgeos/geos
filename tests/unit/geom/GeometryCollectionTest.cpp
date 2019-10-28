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
    std::unique_ptr<geos::geom::Geometry> empty_gc_;

    std::unique_ptr<geos::geom::Geometry> readWKT(std::string wkt) {
        geos::io::WKTReader reader;
        return std::unique_ptr<geos::geom::Geometry>(reader.read(wkt));
    }

    test_geometry_collection_data()
        : pm_(1000), factory_(GeometryFactory::create(&pm_, 0)), empty_gc_(factory_->createGeometryCollection())
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

    std::vector<const Geometry*> geoms{empty_point.get(), point.get()};
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
    std::vector<const Geometry*> v = {g.get()};
    std::unique_ptr<Geometry> geom_col(gf->createGeometryCollection(v));
    ensure_equals(geom_col->getGeometryN(0)->getSRID(), 1);

    geom_col->setSRID(2);
    ensure_equals(geom_col->getGeometryN(0)->getSRID(), 2);

    std::unique_ptr<Geometry> clone(geom_col->clone());
    ensure_equals(clone->getGeometryN(0)->getSRID(), 2);
}

// test getCoordinate() returns nullptr for empty geometry
template<>
template<>
void object::test<3>
()
{
    ensure(empty_gc_->getCoordinate() == nullptr);
}

// test isDimensionStrict for empty GeometryCollection
template<>
template<>
void object::test<4>
()
{
    // Empty GeometryCollection passes isDimensionStrict for any input
    ensure(empty_gc_->isDimensionStrict(geos::geom::Dimension::P));
    ensure(empty_gc_->isDimensionStrict(geos::geom::Dimension::L));
    ensure(empty_gc_->isDimensionStrict(geos::geom::Dimension::A));
}

// test isDimensionStrict for homogeneous GeometryCollections
template<>
template<>
void object::test<5>
()
{
    auto point = readWKT("GEOMETRYCOLLECTION(POINT (1 1), POINT(2 2))");
    auto line = readWKT("GEOMETRYCOLLECTION(LINESTRING (1 1, 2 2), LINESTRING (3 8, 3 9))");
    auto poly = readWKT("GEOMETRYCOLLECTION(POLYGON ((1 1, 2 1, 2 2, 1 1)))");

    ensure(point->isDimensionStrict(geos::geom::Dimension::P));
    ensure(line->isDimensionStrict(geos::geom::Dimension::L));
    ensure(poly->isDimensionStrict(geos::geom::Dimension::A));

    ensure(!point->isDimensionStrict(geos::geom::Dimension::L));
    ensure(!line->isDimensionStrict(geos::geom::Dimension::A));
    ensure(!poly->isDimensionStrict(geos::geom::Dimension::P));
}

// test isDimensionStrict for heterogeneous GeometryCollections
template<>
template<>
void object::test<6>
()
{
    // Heterogenous GeometryCollection does not pass isDimensionString for any input
    auto gc = readWKT("GEOMETRYCOLLECTION(POINT (1 1), LINESTRING (1 1, 2 2))");

    ensure(!gc->isDimensionStrict(geos::geom::Dimension::P));
    ensure(!gc->isDimensionStrict(geos::geom::Dimension::L));
    ensure(!gc->isDimensionStrict(geos::geom::Dimension::A));
}

// test envelopeIntersects
template<>
template<>
void object::test<7>
()
{
    //polygon-polygon
    auto geom1 = readWKT("POLYGON((0 0,100 0, 100 100, 0 100,0 0))");
    auto geom2 = readWKT("POLYGON((200 0,300 0, 300 100, 200 100,200 0))");
    ensure(!geos::geom::GeometryCollection::envelopeIntersects(geom1.get(), geom2.get()));

    geom1 = readWKT("POLYGON((0 0,100 0, 100 100, 0 100,0 0))");
    geom2 = readWKT("POLYGON((100 0,200 0, 200 100, 100 100,100 0))");
    ensure(geos::geom::GeometryCollection::envelopeIntersects(geom1.get(), geom2.get()));

    //multipolygon-polygon, and polygon-multipolygon
    geom1 = readWKT("MULTIPOLYGON(((0 0,100 0, 100 100, 0 100,0 0)),((400 0,500 0, 500 100, 400 100,400 0)))");
    geom2 = readWKT("POLYGON((200 0,300 0, 300 100, 200 100,200 0))");
    ensure(!geos::geom::GeometryCollection::envelopeIntersects(geom1.get(), geom2.get()));
    ensure(!geos::geom::GeometryCollection::envelopeIntersects(geom2.get(), geom1.get()));

    geom1 = readWKT("MULTIPOLYGON(((0 0,100 0, 100 100, 0 100,0 0)),((400 0,500 0, 500 100, 400 100,400 0)))");
    geom2 = readWKT("POLYGON((300 0,400 0, 400 100, 300 100,300 0))");
    ensure(geos::geom::GeometryCollection::envelopeIntersects(geom1.get(), geom2.get()));
    ensure(geos::geom::GeometryCollection::envelopeIntersects(geom2.get(), geom1.get()));

    //multipolygon-multipolygon
    geom1 = readWKT("MULTIPOLYGON(((0 0,100 0, 100 100, 0 100,0 0)),((400 0,500 0, 500 100, 400 100,400 0)))");
    geom2 = readWKT("MULTIPOLYGON(((200 0,300 0, 300 100, 200 100,200 0)),((600 0,700 0, 700 100, 600 100,600 0)))");
    //technically these envelopes don't intersect, but to calculate that would require a O(NxM) algorithm
    ensure(geos::geom::GeometryCollection::envelopeIntersects(geom1.get(), geom2.get()));
    ensure(geos::geom::GeometryCollection::envelopeIntersects(geom2.get(), geom1.get()));

    geom1 = readWKT("MULTIPOLYGON(((0 0,100 0, 100 100, 0 100,0 0)),((400 0,500 0, 500 100, 400 100,400 0)))");
    geom2 = readWKT("MULTIPOLYGON(((300 0,400 0, 400 100, 300 100,300 0)),((600 0,700 0, 700 100, 600 100,600 0)))");
    ensure(geos::geom::GeometryCollection::envelopeIntersects(geom1.get(), geom2.get()));
    ensure(geos::geom::GeometryCollection::envelopeIntersects(geom2.get(), geom1.get()));

    geom1 = readWKT("MULTIPOLYGON(((0 0,100 0, 100 100, 0 100,0 0)),((400 0,500 0, 500 100, 400 100,400 0)))");
    geom2 = readWKT("MULTIPOLYGON(((200 0,300 0, 300 100, 200 100,200 0)),((200 200,300 200, 300 300, 200 300,200 200)))");
    ensure(!geos::geom::GeometryCollection::envelopeIntersects(geom1.get(), geom2.get()));
    ensure(!geos::geom::GeometryCollection::envelopeIntersects(geom2.get(), geom1.get()));

    geom1 = readWKT("MULTIPOLYGON(((0 0,100 0, 100 100, 0 100,0 0)),((400 0,500 0, 500 100, 400 100,400 0)))");
    geom2 = readWKT("MULTIPOLYGON(((200 -200,300 -200, 300 -100, 200 -100,200 -200)),((200 200,300 200, 300 300, 200 300,200 200)))");
    ensure(!geos::geom::GeometryCollection::envelopeIntersects(geom1.get(), geom2.get()));
    ensure(!geos::geom::GeometryCollection::envelopeIntersects(geom2.get(), geom1.get()));
}

} // namespace tut
