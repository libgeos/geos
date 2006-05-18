// $Id$
// 
// Test Suite for Geometry::isRectangle() function

// TUT
#include <tut.h>
// STL
#include <string>
// GEOS
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/io/WKTReader.h>

using namespace tut;

//
// Test Group
//

struct test_contains_data
{
	typedef std::auto_ptr<geos::geom::Geometry> GeomAutoPtr;
	geos::geom::GeometryFactory factory;
	geos::io::WKTReader reader;

	test_contains_data()
	    : reader(&factory)
	{}
};

typedef test_group<test_contains_data> group;
typedef group::object object;

group test_contains_data("geos::geom::Geometry::contains");

//
// Test Cases
//

// 1 - Boundaries touch, still g1 covers g2
template<>
template<>
void object::test<1>()
{
	GeomAutoPtr g1(reader.read(
		"POLYGON ((0 0, 0 100, 100 100, 100 0, 0 0))"
	));
	GeomAutoPtr g2(reader.read(
		"POLYGON ((0 0, 0 100, 90 90, 90 0, 0 0))"
	));

	ensure( g1->covers(g2.get()) );

}

// 2 - Polygon covers line (laying on it's boundary)
template<>
template<>
void object::test<2>()
{
	GeomAutoPtr g1(reader.read(
		"POLYGON ((0 0, 0 100, 100 100, 100 0, 0 0))"
	));
	GeomAutoPtr g2(reader.read(
		"LINESTRING (0 0, 0 100)"
	));

	ensure( g1->covers(g2.get()) );
	ensure( ! g1->contains(g2.get()) );

}

// 3 - Line covers point (laying on it's boundary or interior)
template<>
template<>
void object::test<3>()
{
	GeomAutoPtr g1(reader.read(
		"LINESTRING (0 0, 0 100)"
	));
	GeomAutoPtr g2(reader.read(
		"POINT(0 0)"
	));

	ensure( g1->covers(g2.get()) );
	ensure( ! g1->contains(g2.get()) );

	g2.reset(reader.read(
		"POINT(0 2)"
	));

	ensure( g1->covers(g2.get()) );
	ensure( g1->contains(g2.get()) );

}



