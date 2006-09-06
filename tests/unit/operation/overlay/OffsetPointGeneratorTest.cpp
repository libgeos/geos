// $Id$
// 
// Test Suite for geos::operation::overlay::OffsetPointGenerator class.

// TUT
#include <tut.h>
// GEOS
#include <geos/operation/overlay/OffsetPointGenerator.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/Coordinate.h>
#include <memory>
#include <vector>

namespace tut
{
	//
	// Test Group
	//

	// Common data used by tests
	struct test_offsetpointgenerator_data
	{
		geos::geom::GeometryFactory gf;
		geos::io::WKTReader wktreader;
		geos::algorithm::PointLocator locator;

		typedef geos::geom::Geometry::AutoPtr GeomPtr;

		GeomPtr g;

		test_offsetpointgenerator_data()
			:
			gf(),
			wktreader(&gf)
		{
		}

	};
	

	typedef test_group<test_offsetpointgenerator_data> group;
	typedef group::object object;

	group test_offsetpointgenerator_group("geos::operation::overlay::OffsetPointGenerator");

	//
	// Test Cases
	//

	template<>
	template<>
	void object::test<1>()
	{
		using geos::operation::overlay::OffsetPointGenerator;
		using geos::geom::Coordinate;
		using geos::algorithm::PointLocator;
		using std::auto_ptr;
		using std::vector;

		std::string wkt("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))");
		g.reset(wktreader.read(wkt));

		OffsetPointGenerator gen(*g, 10);

		auto_ptr< vector<Coordinate> > coords(gen.getPoints());

		ensure_equals(coords->size(), (g->getNumPoints()-1)*2);

	}

	template<>
	template<>
	void object::test<2>()
	{
		using geos::operation::overlay::OffsetPointGenerator;
		using geos::geom::Location;
		using geos::geom::Coordinate;
		using std::auto_ptr;
		using std::vector;

		std::string wkt("POLYGON((0 0, 10 0, 10 5, 10 10, 0 10, 0 0))");
		g.reset(wktreader.read(wkt));

		double dist = 0.0003;

		OffsetPointGenerator gen(*g, dist);

		auto_ptr< vector<Coordinate> > coords(gen.getPoints());

		ensure_equals(coords->size(), (g->getNumPoints()-1)*2);
	}



} // namespace tut

