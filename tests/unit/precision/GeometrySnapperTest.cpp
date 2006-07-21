// $Id$
// 
// Test Suite for geos::precision::GeometrySnapper class.

// TUT
#include <tut.h>
// GEOS
#include <geos/precision/GeometrySnapper.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/io/WKTReader.h>

// STL
#include <string>
#include <vector>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_geometrysnapper_data
    {
	typedef std::auto_ptr<geos::geom::Geometry> GeomAutoPtr;

	geos::geom::GeometryFactory factory;

	geos::io::WKTReader reader;

	GeomAutoPtr src;

	geos::precision::GeometrySnapper snapper;

        test_geometrysnapper_data()
		:
		factory(), // initialize before use!
		reader(&factory),
		src(reader.read(
			"POLYGON ((0 0, 0 100, 100 100, 100 0, 0 0))"
		)),
		snapper( *(src.get()) )

	{
	}
    };

    typedef test_group<test_geometrysnapper_data> group;
    typedef group::object object;

    group test_geometrysnapper_group("geos::precision::GeometrySnapper");

    //
    // Test Cases
    //

    // Test vertices snapping
    template<>
    template<>
    void object::test<1>()
    {
	GeomAutoPtr snap(reader.read(
		"MULTIPOINT ((0 0), (0 100.0000001), (100 100), (100 0))"
	));

	// NOTE: we're assuming that GeometrySnapper tolerance is 0.000001
	GeomAutoPtr expected(reader.read(
		"POLYGON ((0 0, 0 100.0000001, 100 100, 100 0, 0 0))"
	));

	GeomAutoPtr ret(snapper.snapTo( *(snap.get()) ));

	ensure( ret->equalsExact(expected.get(),0) );

    }

    // Test vertices snapping
    template<>
    template<>
    void object::test<2>()
    {
	GeomAutoPtr snap(reader.read(
		"MULTIPOINT ((0.0000001 50))"
	));

	// NOTE: we're assuming that GeometrySnapper tolerance is 0.000001
	GeomAutoPtr expected(reader.read(
		"POLYGON ((0 0, 0.0000001 50, 0 100, 100 100, 100 0, 0 0))"
	));

	GeomAutoPtr ret(snapper.snapTo( *(snap.get()) ));

	ensure( ret->equalsExact(expected.get(),0) );
    }



} // namespace tut
