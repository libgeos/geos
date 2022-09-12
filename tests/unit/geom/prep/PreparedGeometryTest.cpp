//
// Test Suite for PreparedGeometry methods

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>

using namespace geos::geom;
using geos::geom::prep::PreparedGeometry;

namespace tut {

//
// Test Group
//

struct test_preparedgeometry_data {
    typedef geos::geom::GeometryFactory GeometryFactory;

    geos::geom::GeometryFactory::Ptr factory;
    geos::io::WKTReader reader;
    std::unique_ptr<geos::geom::Geometry> g1;
    std::unique_ptr<geos::geom::Geometry> g2;
    std::unique_ptr<PreparedGeometry> pg1;
    std::unique_ptr<PreparedGeometry> pg2;

    test_preparedgeometry_data()
        : factory(GeometryFactory::create())
        , reader(factory.get())
        , g1(nullptr)
        , g2(nullptr)
        , pg1(nullptr)
        , pg2(nullptr)
    {}
};

typedef test_group<test_preparedgeometry_data> group;
typedef group::object object;

group test_preparedgeometry_data("geos::geom::prep::PreparedGeometry");

//
// Test Cases
//

// 1 - Check EMPTY elements are handled correctly
// See https://trac.osgeo.org/postgis/ticket/5224
template<>
template<>
void object::test<1>
()
{
    g1 = reader.read( "MULTIPOLYGON (((9 9, 9 1, 1 1, 2 4, 7 7, 9 9)), EMPTY)" );
    g2 = reader.read( "MULTIPOLYGON (((7 6, 7 3, 4 3, 7 6)), EMPTY)" );

    pg1 = prep::PreparedGeometryFactory::prepare(g1.get());

    ensure( pg1->intersects(g2.get()));
    ensure( pg1->contains(g2.get()));
    ensure( pg1->covers(g2.get()));
}

} // namespace tut
