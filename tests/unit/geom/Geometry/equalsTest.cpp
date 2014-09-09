// 
// Test Suite for Geometry's equals() function

// tut
#include <tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>
#include <string>

namespace tut {

//
// Test Group
//

struct test_equals_data
{
	typedef std::auto_ptr<geos::geom::Geometry> GeomAutoPtr;
	geos::geom::GeometryFactory factory;
	geos::io::WKTReader reader;

	test_equals_data()
	    : reader(&factory)
	{}
};

typedef test_group<test_equals_data> group;
typedef group::object object;

group test_equals_data("geos::geom::Geometry::equals");

//
// Test Cases
//

// Empty equals empty
// See http://trac.osgeo.org/geos/ticket/703

template<> template<> void object::test<1>() {

	GeomAutoPtr g1(reader.read("POINT EMPTY"));
	ensure( g1->equals(g1.get()) );

	GeomAutoPtr g2(reader.read("LINESTRING EMPTY"));
	ensure( g2->equals(g2.get()) );
	ensure( g2->equals(g1.get()) );

	GeomAutoPtr g3(reader.read("POLYGON EMPTY"));
	ensure( g3->equals(g3.get()) );
	ensure( g3->equals(g2.get()) );
	ensure( g3->equals(g1.get()) );

	GeomAutoPtr g4(reader.read("GEOMETRYCOLLECTION EMPTY"));
	ensure( g4->equals(g4.get()) );
	ensure( g4->equals(g3.get()) );
	ensure( g4->equals(g2.get()) );
	ensure( g4->equals(g1.get()) );

}


} // namespace tut

