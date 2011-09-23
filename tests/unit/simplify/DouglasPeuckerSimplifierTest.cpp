// $Id: DouglasPeuckerSimplifierTest.cpp 2344 2009-04-09 21:46:30Z mloskot $
// 
// Test Suite for geos::simplify::DouglasPeuckerSimplifierTest

#include <tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/simplify/DouglasPeuckerSimplifier.h>
// std
#include <string>
#include <memory>

namespace tut
{
	using namespace geos::simplify;
	
	//
	// Test Group
	//

	// Common data used by tests
	struct test_dpsimp_data
	{
		geos::geom::PrecisionModel pm;
		geos::geom::GeometryFactory gf;
		geos::io::WKTReader wktreader;

		typedef geos::geom::Geometry::AutoPtr GeomPtr;

		test_dpsimp_data()
			:
			pm(1.0),
			gf(&pm),
			wktreader(&gf)
		{}
	};

	typedef test_group<test_dpsimp_data> group;
	typedef group::object object;

	group test_dpsimp_group("geos::simplify::DouglasPeuckerSimplifier");

	//
	// Test Cases
	//

	// 1 - PolygonNoReduction
	template<>
	template<>
	void object::test<1>()
	{         
		std::string wkt("POLYGON((20 220, 40 220, 60 220, 80 220, 100 220, \
					120 220, 140 220, 140 180, 100 180, 60 180, 20 180, 20 220))");

		GeomPtr g(wktreader.read(wkt));

		GeomPtr simplified = DouglasPeuckerSimplifier::simplify(
			g.get(), 10.0);

		ensure( simplified->isValid() );

		// topology is unchanged
		ensure( simplified->equals(g.get()) );
	}

	// 2 - PolygonReductionWithSplit
	template<>
	template<>
	void object::test<2>()
	{         
		std::string wkt_in("POLYGON ((40 240, 160 241, 280 240, 280 160, \
					160 240, 40 140, 40 240))");
		
		std::string wkt_ex("MULTIPOLYGON (((40.0 240.0, 160.0 240.0, 40.0 140.0, 40.0 240.0)), \
					((160.0 240.0, 280.0 240.0, 280.0 160.0, 160.0 240.0)))");

		GeomPtr g(wktreader.read(wkt_in));

		GeomPtr expected(wktreader.read(wkt_ex));

        // TODO: This test blows because if instability of geos.index.strtree::yComparator() predicate

		GeomPtr simplified = DouglasPeuckerSimplifier::simplify(
			g.get(), 10.0);

		ensure( simplified->isValid() );
		
		ensure( simplified->equalsExact(expected.get()) );

	}

	// 3 - PolygonReduction
	template<>
	template<>
	void object::test<3>()
	{         
		std::string wkt_in("POLYGON ((120 120, 121 121, 122 122, 220 120, \
					180 199, 160 200, 140 199, 120 120))");

		std::string wkt_ex("POLYGON ((120 120, 140 199, 160 200, 180 199, 220 120, 120 120))");

		GeomPtr g(wktreader.read(wkt_in));

		GeomPtr expected(wktreader.read(wkt_ex));

		GeomPtr simplified = DouglasPeuckerSimplifier::simplify(
			g.get(), 10.0);

		ensure( simplified->isValid() );
		
		ensure( simplified->equalsExact(expected.get()) );

	}

	// 4 - PolygonWithTouchingHole
	template<>
	template<>
	void object::test<4>()
	{         
		std::string wkt_in("POLYGON ((80 200, 240 200, 240 60, 80 60, 80 200), \
					(120 120, 220 120, 180 199, 160 200, 140 199, 120 120))");
		
		std::string wkt_ex("POLYGON ((80 200, 160 200, 240 200, 240 60, 80 60, 80 200), \
					(160 200, 140 199, 120 120, 220 120, 180 199, 160 200)))");

		GeomPtr g(wktreader.read(wkt_in));

		GeomPtr expected(wktreader.read(wkt_ex));

		GeomPtr simplified = DouglasPeuckerSimplifier::simplify(
			g.get(), 10.0);

		ensure( simplified->isValid() );

		ensure( simplified->isValid() );

		ensure( simplified->equalsExact(expected.get()) );

	}

	// 5 - FlattishPolygon
	template<>
	template<>
	void object::test<5>()
	{         
		std::string wkt_in("POLYGON ((0 0, 50 0, 53 0, 55 0, 100 0, 70 1, 60 1, 50 1, 40 1, 0 0))");
		std::string wkt_ex("POLYGON EMPTY");

		GeomPtr g(wktreader.read(wkt_in));

		GeomPtr expected(wktreader.read(wkt_ex));

		GeomPtr simplified = DouglasPeuckerSimplifier::simplify(
			g.get(), 10.0);

		ensure( simplified->isValid() );

		ensure( simplified->equalsExact(expected.get()) );
		//ensure_equals( *simplified, *expected );

	}

	// 6 - TinySquare
	template<>
	template<>
	void object::test<6>()
	{         
		std::string wkt_in("POLYGON ((0 5, 5 5, 5 0, 0 0, 0 1, 0 5))");
		std::string wkt_ex("POLYGON EMPTY");


		GeomPtr g(wktreader.read(wkt_in));

		GeomPtr expected(wktreader.read(wkt_ex));

		GeomPtr simplified = DouglasPeuckerSimplifier::simplify(
			g.get(), 10.0);

		ensure( simplified->isValid() );

		ensure( simplified->equalsExact(expected.get()) );

	}

	// 7 - TinyLineString
	template<>
	template<>
	void object::test<7>()
	{         
		std::string wkt_in("LINESTRING (0 5, 1 5, 2 5, 5 5)");
		std::string wkt_ex("LINESTRING (0 5, 5 5)");

		GeomPtr g(wktreader.read(wkt_in));

		GeomPtr expected(wktreader.read(wkt_ex));

		GeomPtr simplified = DouglasPeuckerSimplifier::simplify(
			g.get(), 10.0);

		ensure( simplified->isValid() );

		ensure( simplified->equalsExact(expected.get()) );

	}

	// 8 - MultiPoint
	template<>
	template<>
	void object::test<8>()
	{         
		std::string wkt_in("MULTIPOINT(80 200, 240 200, 240 60, 80 60, 80 200, 140 199, 120 120)");

		GeomPtr g(wktreader.read(wkt_in));

		GeomPtr simplified = DouglasPeuckerSimplifier::simplify(
			g.get(), 10.0);

		// MultiPoint is *not* simplified
		ensure( simplified->equalsExact(g.get()) );
	}

	// 9 - MultiLineString
	template<>
	template<>
	void object::test<9>()
	{         
		std::string wkt_in("MULTILINESTRING( (0 0, 50 0, 70 0, 80 0, 100 0), \
					(0 0, 50 1, 60 1, 100 0) )");
		
		std::string wkt_ex("MULTILINESTRING( (0 0, 100 0), (0 0, 100 0) )");

		GeomPtr g(wktreader.read(wkt_in));

		GeomPtr expected(wktreader.read(wkt_ex));

		GeomPtr simplified = DouglasPeuckerSimplifier::simplify(
			g.get(), 10.0);

		ensure( simplified->isValid() );

		ensure( simplified->equalsExact(expected.get()) );
	}

	// 10 - GeometryCollection
	template<>
	template<>
	void object::test<10>()
	{         
		std::string wkt_in("GEOMETRYCOLLECTION ( \
					MULTIPOINT (80 200, 240 200, 240 60, 80 60, 80 200, 140 199, 120 120), \
					POLYGON ((80 200, 240 200, 240 60, 80 60, 80 200)), \
					LINESTRING (80 200, 240 200, 240 60, 80 60, 80 200, 140 199, 120 120) )");

		std::string wkt_ex("MULTILINESTRING( (0 0, 100 0), (0 0, 100 0) )");

		GeomPtr g(wktreader.read(wkt_in));

		GeomPtr expected(wktreader.read(wkt_ex));

		GeomPtr simplified = DouglasPeuckerSimplifier::simplify(
			g.get(), 10.0);

		ensure( simplified->isValid() );

		// Non simplification occurs
		ensure( simplified->equalsExact(g.get()) );
	}

} // namespace tut

