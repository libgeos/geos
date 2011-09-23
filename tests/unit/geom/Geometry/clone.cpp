// 
// Test Suite for Geometry's clone() 

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

struct test_geometry_clone_data
{
	typedef std::auto_ptr<geos::geom::Geometry> GeomAutoPtr;
	geos::geom::GeometryFactory factory;
	geos::io::WKTReader reader;

	test_geometry_clone_data()
	    : reader(&factory)
	{}
};

typedef test_group<test_geometry_clone_data> group;
typedef group::object object;

group test_geometry_clone_data("geos::geom::Geometry::clone");

//
// Test Cases
//

template<>
template<>
void object::test<1>()
{
	GeomAutoPtr g1(reader.read(
		"POINT (0 100)"
	));
	g1->setSRID(66);
	GeomAutoPtr g2(g1->clone());

	ensure( g1->equalsExact(g2.get()) );
	ensure_equals( g1->getSRID(), 66 );
	ensure_equals( g1->getSRID(), g2->getSRID() );

}

template<>
template<>
void object::test<2>()
{
	GeomAutoPtr g1(reader.read(
		"LINESTRING (0 0, 0 100, 100 100, 100 0)"
	));
	g1->setSRID(66);
	GeomAutoPtr g2(g1->clone());

	ensure( g1->equalsExact(g2.get()) );
	ensure_equals( g1->getSRID(), 66 );
	ensure_equals( g1->getSRID(), g2->getSRID() );

}

template<>
template<>
void object::test<3>()
{
	GeomAutoPtr g1(reader.read(
		"POLYGON ((0 0, 0 100, 100 100, 100 0, 0 0))"
	));
	g1->setSRID(66);
	GeomAutoPtr g2(g1->clone());

	ensure( g1->equalsExact(g2.get()) );
	ensure_equals( g1->getSRID(), 66 );
	ensure_equals( g1->getSRID(), g2->getSRID() );

}

template<>
template<>
void object::test<4>()
{
	GeomAutoPtr g1(reader.read(
		"MULTIPOINT (0 100, 5 6)"
	));
	g1->setSRID(66);
	GeomAutoPtr g2(g1->clone());

	ensure( g1->equalsExact(g2.get()) );
	ensure_equals( g1->getSRID(), 66 );
	ensure_equals( g1->getSRID(), g2->getSRID() );

}

template<>
template<>
void object::test<5>()
{
	GeomAutoPtr g1(reader.read(
		"MULTILINESTRING ((0 0, 0 100, 100 100, 100 0), (15 25, 25 52))"
	));
	g1->setSRID(66);
	GeomAutoPtr g2(g1->clone());

	ensure( g1->equalsExact(g2.get()) );
	ensure_equals( g1->getSRID(), 66 );
	ensure_equals( g1->getSRID(), g2->getSRID() );

}

template<>
template<>
void object::test<6>()
{
	GeomAutoPtr g1(reader.read(
		"MULTIPOLYGON (((0 0, 0 100, 100 100, 100 0, 0 0)))"
	));
	g1->setSRID(66);
	GeomAutoPtr g2(g1->clone());
	ensure( g1->equalsExact(g2.get()) );
	ensure_equals( g1->getSRID(), 66 );
	ensure_equals( g1->getSRID(), g2->getSRID() );
}

template<>
template<>
void object::test<7>()
{
	GeomAutoPtr g1(reader.read(
		"GEOMETRYCOLLECTION(MULTIPOLYGON (((0 0, 0 100, 100 100, 100 0, 0 0))),POINT(3 4))"
	));
	g1->setSRID(66);
	GeomAutoPtr g2(g1->clone());
	ensure( g1->equalsExact(g2.get()) );
	ensure_equals( g1->getSRID(), 66 );
	ensure_equals( g1->getSRID(), g2->getSRID() );
}


} // namespace tut

