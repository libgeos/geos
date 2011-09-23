// $Id: OverlayResultValidatorTest.cpp 2462 2009-05-04 21:28:26Z strk $
// 
// Test Suite for geos::operation::overlay::validate::OverlayResultValidator class.

// tut
#include <tut.hpp>
// geos
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/operation/overlay/validate/OverlayResultValidator.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/Coordinate.h>
// std
#include <memory>
#include <vector>

namespace tut
{
	using namespace geos::operation::overlay;
	using namespace geos::operation::overlay::validate;

	//
	// Test Group
	//

	// Common data used by tests
	struct test_overlayresultvalidator_data
	{
		geos::geom::GeometryFactory gf;
		geos::io::WKTReader wktreader;
		geos::algorithm::PointLocator locator;

		typedef geos::geom::Geometry::AutoPtr GeomPtr;

		GeomPtr g0, g1, gres;

		test_overlayresultvalidator_data()
			:
			gf(),
			wktreader(&gf)
		{
		}

	};
	

	typedef test_group<test_overlayresultvalidator_data> group;
	typedef group::object object;

	group test_overlayresultvalidator_group("geos::operation::overlay::validate::OverlayResultValidator");

	//
	// Test Cases
	//

	template<>
	template<>
	void object::test<1>()
	{

		std::string wkt0("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
		GeomPtr g0(wktreader.read(wkt0));

		std::string wkt1("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
		GeomPtr g1(wktreader.read(wkt1));

		std::string wktres("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
		GeomPtr gres(wktreader.read(wkt1));

		OverlayResultValidator validator(*g0, *g1, *gres);

		ensure( validator.isValid(OverlayOp::opUNION) );
	}

	template<>
	template<>
	void object::test<2>()
	{

		std::string wkt0("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
		GeomPtr g0(wktreader.read(wkt0));

		std::string wkt1("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
		GeomPtr g1(wktreader.read(wkt1));

		std::string wktres("POLYGON((0 0, 10.001 0, 10 10, 0 10, 0 0))");
		GeomPtr gres(wktreader.read(wktres));

		OverlayResultValidator validator(*g0, *g1, *gres);

		ensure (! validator.isValid(OverlayOp::opUNION) );
	}

	template<>
	template<>
	void object::test<3>()
	{

		std::string wkt0("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
		GeomPtr g0(wktreader.read(wkt0));

		std::string wkt1("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
		GeomPtr g1(wktreader.read(wkt1));

		std::string wktres("POLYGON((0 0, 9.999 0, 10 10, 0 10, 0 0))");
		GeomPtr gres(wktreader.read(wktres));

		OverlayResultValidator validator(*g0, *g1, *gres);

		ensure (! validator.isValid(OverlayOp::opUNION) );
	}

	// Result of union has an hole not in input
	template<>
	template<>
	void object::test<4>()
	{

		std::string wkt0("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
		GeomPtr g0(wktreader.read(wkt0));

		std::string wkt1("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
		GeomPtr g1(wktreader.read(wkt1));

		std::string wktres("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(5 5, 5 6, 6 6, 5 5))");
		GeomPtr gres(wktreader.read(wktres));

		OverlayResultValidator validator(*g0, *g1, *gres);

		ensure (! validator.isValid(OverlayOp::opUNION) );
	}


	template<>
	template<>
	void object::test<5>()
	{

		std::string wkt0("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
		GeomPtr g0(wktreader.read(wkt0));

		std::string wkt1("POLYGON((5 0, 1500 0, 1500 10, 5 10, 5 0))");
		GeomPtr g1(wktreader.read(wkt1));

		// small shift
		std::string wktres("POLYGON((0 0, 750 0.0001, 150 0, 150 10, 0 10, 0 0))");
		GeomPtr gres(wktreader.read(wktres));

		OverlayResultValidator validator(*g0, *g1, *gres);

		ensure (! validator.isValid(OverlayOp::opUNION) );
	}

	template<>
	template<>
	void object::test<6>() 
	{

		std::string wkt0("POLYGON ((20.0 40.0, 20.0 200.0, 180.0 200.0, 180.0 120.0, 140.0 120.0, 180.0 119.0, 180.0 40.0, 20.0 40.0), (140.0 160.0, 80.0 120.0, 140.0 80.0, 140.0 160.0))");
		GeomPtr g0(wktreader.read(wkt0));

		std::string wkt1("POLYGON ((200.0 160.0, 150.0 160.0, 150.0 80.0, 200.0 80.0, 200.0 160.0))");
		GeomPtr g1(wktreader.read(wkt1));

		std::string wktres("MULTIPOLYGON (((20.0 40.0, 20.0 200.0, 180.0 200.0, 180.0 160.0, 150.0 160.0, 150.0 120.0, 150.0 80.0, 180.0 80.0, 180.0 40.0, 20.0 40.0), (80.0 120.0, 140.0 80.0, 140.0 120.0, 140.0 160.0, 80.0 120.0)), ((150.0 120.0, 180.0 120.0, 180.0 160.0, 200.0 160.0, 200.0 80.0, 180.0 80.0, 180.0 119.0, 150.0 120.0)))");
		GeomPtr gres(wktreader.read(wktres));

		OverlayResultValidator validator(*g0, *g1, *gres);

		ensure ( ! validator.isValid(OverlayOp::opSYMDIFFERENCE) );
	}


	//
	// TODO: add tests for other operations
	//       and other geometry types (IFF we drop the limit of the
	//       class to only work with areal geoms)


} // namespace tut

