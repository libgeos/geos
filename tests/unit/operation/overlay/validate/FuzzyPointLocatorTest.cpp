// $Id: FuzzyPointLocatorTest.cpp 2462 2009-05-04 21:28:26Z strk $
// 
// Test Suite for geos::operation::overlay::validate::FuzzyPointLocator class.


// tut
#include <tut.hpp>
// geos
#include <geos/operation/overlay/validate/FuzzyPointLocator.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKBReader.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Location.h>
// std
#include <memory>
#include <sstream>

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
		geos::io::WKBReader wkbreader;

		typedef geos::geom::Geometry::AutoPtr GeomPtr;

		GeomPtr g;

		test_fuzzypointlocator_data()
			:
			gf(),
			wktreader(&gf),
			wkbreader(gf)
		{
			std::string wkt("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
			g.reset(wktreader.read(wkt));
		}
	};
	

	typedef test_group<test_fuzzypointlocator_data> group;
	typedef group::object object;

	group test_fuzzypointlocator_group("geos::operation::overlay::validate::FuzzyPointLocator");

	//
	// Test Cases
	//

	template<>
	template<>
	void object::test<1>()
	{
		using geos::operation::overlay::validate::FuzzyPointLocator;
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
		using geos::operation::overlay::validate::FuzzyPointLocator;
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
		using geos::operation::overlay::validate::FuzzyPointLocator;
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
		using geos::operation::overlay::validate::FuzzyPointLocator;
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


	template<>
	template<>
	void object::test<5>()
	{
		using geos::operation::overlay::validate::FuzzyPointLocator;
		using geos::geom::Location;
		geos::geom::Coordinate pt(160.000000125, 119.500004998);

		std::string wkt0("POLYGON((20 40,20 200,180 200,180 120,140 120,180 119,180 40,20 40),(140 160,80 120,140 80,140 160))");
		GeomPtr g0(wktreader.read(wkt0));

		FuzzyPointLocator locator(*g0, 0.00001);

		// On the boundary ?
		ensure_equals(locator.getLocation(pt), Location::BOUNDARY); 

	}

	template<>
	template<>
	void object::test<6>()
	{
		using geos::operation::overlay::validate::FuzzyPointLocator;
		using geos::geom::Location;
		// this would return the expected result
		//geos::geom::Coordinate pt(160, 120);
		geos::geom::Coordinate pt(160, 120.000005);

		std::stringstream wkb0("0103000000020000000800000000000000000034400000000000004440000000000000344000000000000069400000000000806640000000000000694000000000008066400000000000005E4000000000008061400000000000005E4000000000008066400000000000C05D400000000000806640000000000000444000000000000034400000000000004440040000000000000000806140000000000000644000000000000054400000000000005E400000000000806140000000000000544000000000008061400000000000006440");
		GeomPtr g0(wkbreader.readHEX(wkb0));

		FuzzyPointLocator locator(*g0, 0.000001);

		// On the boundary ?
		ensure_equals(locator.getLocation(pt), Location::INTERIOR); 

	}

	template<>
	template<>
	void object::test<7>()
	{
		using geos::operation::overlay::validate::FuzzyPointLocator;
		using geos::geom::Location;
		// this would return the expected result
		//geos::geom::Coordinate pt(160, 120);
		geos::geom::Coordinate pt(160, 120.000005);

		std::stringstream wkb0("0106000000020000000103000000020000000A000000000000000000344000000000000044400000000000003440000000000000694000000000008066400000000000006940000000000080664000000000000064400000000000C0624000000000000064400000000000C062400000000000005E400000000000C0624000000000000054400000000000806640000000000000544000000000008066400000000000004440000000000000344000000000000044400500000000000000000054400000000000005E400000000000806140000000000000544000000000008061400000000000005E400000000000806140000000000000644000000000000054400000000000005E40010300000001000000080000000000000000C062400000000000005E4000000000008066400000000000005E400000000000806640000000000000644000000000000069400000000000006440000000000000694000000000000054400000000000806640000000000000544000000000008066400000000000C05D400000000000C062400000000000005E40");
		GeomPtr g0(wkbreader.readHEX(wkb0));

		FuzzyPointLocator locator(*g0, 0.000001);

		// On the boundary ?
		ensure_equals(locator.getLocation(pt), Location::EXTERIOR); 

	}

} // namespace tut

