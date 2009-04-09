// $Id$
// 
// Test Suite for geos::operation::distance::DistanceOp class.

// tut
#include <tut.hpp>
// geos
#include <geos/operation/distance/DistanceOp.h>
#include <geos/platform.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/algorithm/PointLocator.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/CoordinateSequence.h>
// std
#include <memory>
#include <string>
#include <vector>

namespace tut
{
	//
	// Test Group
	//

	// Common data used by tests
	struct test_distanceop_data
	{
		geos::geom::GeometryFactory gf;
		geos::io::WKTReader wktreader;

		typedef geos::geom::Geometry::AutoPtr GeomPtr;

		test_distanceop_data()
            : gf(), wktreader(&gf)
		{}
	};

	typedef test_group<test_distanceop_data> group;
	typedef group::object object;

	group test_distanceop_group("geos::operation::distance::DistanceOp");

	//
	// Test Cases
	//
	template<>
	template<>
	void object::test<1>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("POINT(0 0)");
		std::string wkt1("POINT(10 0)");
		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 10);

		// TODO: test closestPoints() and
		//       closestLocations()
        geos::geom::CoordinateSequence* coords = dist.closestPoints();
        ensure(0 != coords);
	}

	template<>
	template<>
	void object::test<2>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("POINT(0 0)");
		std::string wkt1("MULTIPOINT(10 0, 50 30)");
		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 10);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<3>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("POINT(3 0)");
		std::string wkt1("LINESTRING(0 10, 50 10, 100 50)");
		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 10);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<4>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("POINT(3 0)");
		std::string wkt1("MULTILINESTRING((34 54, 60 34),(0 10, 50 10, 100 50))");
		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 10);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<5>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("POINT(35 60)");
		std::string wkt1("POLYGON((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50))");
		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 6);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<6>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("POINT(35 60)");
		std::string wkt1("MULTIPOLYGON(((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50)),( (100 100, 150 100, 150 150, 100 150, 100 100),(120 120, 120 130, 130 130, 130 120, 120 120)) ))");
		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 6);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<7>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("POINT(35 60)");

		// This is an invalid geom... anyway
		std::string wkt1("GEOMETRYCOLLECTION(MULTIPOLYGON(((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50)),( (100 100, 150 100, 150 150, 100 150, 100 100),(120 120, 120 130, 130 130, 130 120, 120 120)) )), POLYGON((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50)), MULTILINESTRING((34 54, 60 34),(0 10, 50 10, 100 50)), LINESTRING(0 10, 50 10, 100 50), MULTIPOINT(10 0, 50 30), POINT(10 0))");

		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 6);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<8>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("POINT(35 60)");

		// This is an invalid geom... anyway
		std::string wkt1("GEOMETRYCOLLECTION EMPTY");

		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), DoubleInfinity);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<9>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("MULTIPOINT(10 0, 50 30)");
		std::string wkt1("POINT(10 0)");
		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 0);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<10>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("MULTIPOINT(10 0, 50 30)");
		std::string wkt1("MULTIPOINT(0 0, 150 30)");
		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 10);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<11>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("MULTIPOINT(3 0, 200 30)");
		std::string wkt1("LINESTRING(0 10, 50 10, 100 50)");
		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 10);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<12>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("MULTIPOINT(3 0, -50 30)");
		std::string wkt1("MULTILINESTRING((34 54, 60 34),(0 10, 50 10, 100 50))");
		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 10);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<13>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("MULTIPOINT(-100 0, 35 60)");
		std::string wkt1("POLYGON((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50))");
		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 6);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<14>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("MULTIPOINT(-100 0, 35 60)");
		std::string wkt1("MULTIPOLYGON(((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50)),( (100 100, 150 100, 150 150, 100 150, 100 100),(120 120, 120 130, 130 130, 130 120, 120 120)) ))");
		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 6);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<15>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("MULTIPOINT(-100 0, 35 60)");

		// This is an invalid geom... anyway
		std::string wkt1("GEOMETRYCOLLECTION(MULTIPOLYGON(((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50)),( (100 100, 150 100, 150 150, 100 150, 100 100),(120 120, 120 130, 130 130, 130 120, 120 120)) )), POLYGON((34 54, 60 34, 60 54, 34 54),(50 50, 52 50, 52 48, 50 48, 50 50)), MULTILINESTRING((34 54, 60 34),(0 10, 50 10, 100 50)), LINESTRING(0 10, 50 10, 100 50), MULTIPOINT(10 0, 50 30), POINT(10 0))");

		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), 6);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

	template<>
	template<>
	void object::test<16>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("MULTIPOINT(-100 0, 35 60)");

		// This is an invalid geom... anyway
		std::string wkt1("GEOMETRYCOLLECTION EMPTY");

		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());

		ensure_equals(dist.distance(), DoubleInfinity);

		// TODO: test closestPoints() and
		//       closestLocations()

	}

    // Test for crash reported in Ticket #236:
    // http://trac.osgeo.org/geos/ticket/236
	template<>
	template<>
	void object::test<17>()
	{
		using geos::operation::distance::DistanceOp;

		std::string wkt0("POLYGON((0 0, 0 1, 1 1, 1 0, 0 0))");
		std::string wkt1("POLYGON((1.25 0.25, 1.25 0.75, 1.75 0.75, 1.75 0.25, 1.25 0.25))");

		GeomPtr g0(wktreader.read(wkt0));
		GeomPtr g1(wktreader.read(wkt1));

		DistanceOp dist(g0.get(), g1.get());
		ensure_equals(dist.distance(), 0.25);

        // FIXME: This operation crashes becasue of both GeometryLocation's are NULL
        //        in DistanceOp::closestPoints()
        //geos::geom::CoordinateSequence* cs dist.closestPoints();
	}

	// TODO: finish the tests by adding:
	// 	LINESTRING - *all*
	// 	MULTILINESTRING - *all*
	// 	POLYGON - *all*
	// 	MULTIPOLYGON - *all*
	// 	COLLECTION - *all*

} // namespace tut

