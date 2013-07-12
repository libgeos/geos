// 
// Test Suite for Geometry's covers(), coveredBy() and contains() functions

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

group test_contains_data("geos::geom::Geometry::covers");

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
	ensure( g2->coveredBy(g1.get()) );

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
	ensure( g2->coveredBy(g1.get()) );
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
	ensure( g2->coveredBy(g1.get()) );
	ensure( ! g1->contains(g2.get()) );

	g2.reset(reader.read(
		"POINT(0 2)"
	));

	ensure( g1->covers(g2.get()) );
	ensure( g2->coveredBy(g1.get()) );
	ensure( g1->contains(g2.get()) );

}

// 4 - http://trac.osgeo.org/geos/ticket/580
template<>
template<>
void object::test<4>()
{
	GeomAutoPtr g1(reader.read(
		"POLYGON ((-1.183864 52.951915, -1.183862 52.951903, -1.183890 52.951900, -1.183924 52.951897, -1.183958 52.951894, -1.183954 52.951880, -1.183954 52.951878, -1.183932 52.951841, -1.183904 52.951844, -1.183870 52.951847, -1.183832 52.951852, -1.183824 52.951838, -1.183820 52.951830, -1.183870 52.951819, -1.183886 52.951815, -1.183890 52.951819, -1.183929 52.951810, -1.183909 52.951776, -1.183861 52.951787, -1.183853 52.951788, -1.183842 52.951770, -1.183970 52.951742, -1.183983 52.951763, -1.183963 52.951768, -1.183975 52.951788, -1.183994 52.951785, -1.184009 52.951807, -1.184002 52.951808, -1.184009 52.951835, -1.183990 52.951836, -1.183990 52.951836, -1.183990 52.951838, -1.184001 52.951880, -1.184018 52.951954, -1.184020 52.951956, -1.183998 52.951957, -1.183998 52.951956, -1.183996 52.951948, -1.183970 52.951906, -1.183936 52.951909, -1.183864 52.951915))"
	));
	GeomAutoPtr g2(reader.read(
		"POINT (-1.183972 52.951871)"
	));

	ensure( g1->covers(g2.get()) );
	ensure( g2->coveredBy(g1.get()) );
	ensure( g1->contains(g2.get()) );

}


} // namespace tut

