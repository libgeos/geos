// $Id$
// 
// Test Suite for geos::operation::overlay::FuzzyPointLocator class.

// TUT
#include <tut.h>
// GEOS
#include <geos/operation/overlay/FuzzyPointLocator.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Location.h>
#include <memory>

namespace tut
{
	//
	// Test Group
	//

	// Common data used by tests
	struct test_fuzzypointlocator_data
	{
		geos::geom::GeometryFactory gf;
		geos::io::WKTReader wktreader;

		typedef geos::geom::Geometry::AutoPtr GeomPtr;

		GeomPtr g;

		test_fuzzypointlocator_data()
			:
			gf(),
			wktreader(&gf)
		{
			std::string wkt("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
			g.reset(wktreader.read(wkt));
		}
	};
	

	typedef test_group<test_fuzzypointlocator_data> group;
	typedef group::object object;

	group test_fuzzypointlocator_group("geos::operation::overlay::FuzzyPointLocator");

	//
	// Test Cases
	//

	template<>
	template<>
	void object::test<1>()
	{
		using geos::operation::overlay::FuzzyPointLocator;
		using geos::geom::Location;
		using geos::geom::Coordinate;

		FuzzyPointLocator locator(*g, 10);

		ensure_equals(locator.getLocation(Coordinate(10, 0)),
			Location::BOUNDARY); 

		// 8 units away from boundary
		ensure_equals(locator.getLocation(Coordinate(18, 5)),
			Location::BOUNDARY); 
	}

	template<>
	template<>
	void object::test<2>()
	{
		using geos::operation::overlay::FuzzyPointLocator;
		using geos::geom::Location;
		using geos::geom::Coordinate;

		FuzzyPointLocator locator(*g, 10);

		//
		// These are corner cases. I think they should
		// return Location::BOUNDARY, but need to discuss
		// this with Martin Davis
		//

		// 10 units away from boundary
		ensure_equals(locator.getLocation(Coordinate(20, 2)),
			Location::EXTERIOR); 

		// 10 units away from boundary
		ensure_equals(locator.getLocation(Coordinate(-10, 5)),
			Location::EXTERIOR); 
	}

	template<>
	template<>
	void object::test<3>()
	{
		using geos::operation::overlay::FuzzyPointLocator;
		using geos::geom::Location;
		using geos::geom::Coordinate;

		FuzzyPointLocator locator(*g, 8);

		// 5 units away from boundary
		ensure_equals(locator.getLocation(Coordinate(5, 5)),
			Location::BOUNDARY); 

		// 2 units away from boundary
		ensure_equals(locator.getLocation(Coordinate(2, 8)),
			Location::BOUNDARY); 
	}

	template<>
	template<>
	void object::test<4>()
	{
		using geos::operation::overlay::FuzzyPointLocator;
		using geos::geom::Location;
		using geos::geom::Coordinate;

		FuzzyPointLocator locator(*g, 2);

		// 5 units away from boundary
		ensure_equals(locator.getLocation(Coordinate(5, 5)),
			Location::INTERIOR); 

		// 3 units away from boundary
		ensure_equals(locator.getLocation(Coordinate(3, 7)),
			Location::INTERIOR); 
	}



} // namespace tut

