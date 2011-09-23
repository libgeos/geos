// $Id: TopologyPreservingSimplifierTest.cpp 2344 2009-04-09 21:46:30Z mloskot $
// 
// Test Suite for geos::simplify::TopologyPreservingSimplifier

#include <tut.hpp>
#include <utility.h>
// geos
#include <geos/io/WKTReader.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/simplify/TopologyPreservingSimplifier.h>
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
	struct test_tpsimp_data
	{
		geos::geom::PrecisionModel pm;
		geos::geom::GeometryFactory gf;
		geos::io::WKTReader wktreader;

		typedef geos::geom::Geometry::AutoPtr GeomPtr;

		test_tpsimp_data()
            : pm(1.0), gf(&pm), wktreader(&gf)
		{}
	};

	typedef test_group<test_tpsimp_data> group;
	typedef group::object object;

	group test_tpsimp_group("geos::simplify::TopologyPreservingSimplifier");

	//
	// Test Cases
	//

	// PolygonNoReduction
	template<>
	template<>
	void object::test<1>()
	{         
		std::string wkt("POLYGON((20 220, 40 220, 60 220, 80 220, \
                    100 220, 120 220, 140 220, 140 180, 100 180, \
                    60 180, 20 180, 20 220))");

		GeomPtr g(wktreader.read(wkt));
		GeomPtr simplified = TopologyPreservingSimplifier::simplify(g.get(), 10.0);

		ensure( "Simplified geometry is invalid!", simplified->isValid() );
		ensure( "Simplified and original geometry inequal", simplified->equals(g.get()) );
        ensure_equals_geometry( g.get(), simplified.get() );
	}

    // PolygonNoReductionWithConflicts
	template<>
	template<>
	void object::test<2>()
	{
        std::string wkt("POLYGON ((40 240, 160 241, 280 240, 280 160, \
                        160 240, 40 140, 40 240))");
    
        GeomPtr g(wktreader.read(wkt));
		GeomPtr simplified = TopologyPreservingSimplifier::simplify(g.get(), 10.0);

		ensure( "Simplified geometry is invalid!", simplified->isValid() );
		ensure( "Topology has been changed by simplification!", simplified->equals(g.get()) );
        ensure_equals_geometry( g.get(), simplified.get() );
    }

    // PolygonWithTouchingHole
	template<>
	template<>
	void object::test<3>()
	{
        std::string wkt("POLYGON ((80 200, 240 200, 240 60, 80 60, 80 200), \
                    (120 120, 220 120, 180 199, 160 200, 140 199, 120 120))");

        std::string wkt_expected("POLYGON ((80 200, 240 200, 240 60, 80 60, 80 200), \
                    (120 120, 220 120, 180 199, 160 200, 140 199, 120 120))");
        
        GeomPtr g(wktreader.read(wkt));
		GeomPtr simplified = TopologyPreservingSimplifier::simplify(g.get(), 10.0);

		ensure( "Simplified geometry is invalid!", simplified->isValid() );
        ensure_equals_geometry( g.get(), simplified.get() );

        GeomPtr g_expected(wktreader.read(wkt_expected));

        ensure( g_expected->equalsExact(simplified.get()) );
    }

    // FlattishPolygon
	template<>
	template<>
	void object::test<4>()
	{
        std::string wkt("POLYGON ((0 0, 50 0, 53 0, 55 0, 100 0, \
                    70 1, 60 1, 50 1, 40 1, 0 0))");

        GeomPtr g(wktreader.read(wkt));
		GeomPtr simplified = TopologyPreservingSimplifier::simplify(g.get(), 10.0);

		ensure( "Simplified geometry is invalid!", simplified->isValid() );
        ensure_equals_geometry( g.get(), simplified.get() );
    }

    // PolygonWithFlattishHole
    template<>
	template<>
	void object::test<5>()
	{
        std::string wkt("POLYGON ((0 0, 0 200, 200 200, 200 0, 0 0), \
                    (140 40, 90 95, 40 160, 95 100, 140 40))");

        GeomPtr g(wktreader.read(wkt));
        GeomPtr g_expected(wktreader.read(wkt));

		GeomPtr simplified = TopologyPreservingSimplifier::simplify(g.get(), 10.0);

		ensure( "Simplified geometry is invalid!", simplified->isValid() );
        ensure_equals_geometry( g.get(), simplified.get() );
        ensure( g_expected->equalsExact(simplified.get()) );
    }

    // TinySquare
    template<>
	template<>
	void object::test<6>()
	{
        std::string wkt("POLYGON ((0 5, 5 5, 5 0, 0 0, 0 1, 0 5))");

        GeomPtr g(wktreader.read(wkt));
		GeomPtr simplified = TopologyPreservingSimplifier::simplify(g.get(), 10.0);

		ensure( "Simplified geometry is invalid!", simplified->isValid() );
        ensure_equals_geometry( g.get(), simplified.get() );
    }

    // TinyLineString
    template<>
	template<>
	void object::test<7>()
	{
        std::string wkt("LINESTRING (0 5, 1 5, 2 5, 5 5)");

        GeomPtr g(wktreader.read(wkt));
		GeomPtr simplified = TopologyPreservingSimplifier::simplify(g.get(), 10.0);

		ensure( "Simplified geometry is invalid!", simplified->isValid() );
        ensure_equals_geometry(g.get(), simplified.get() );
    }

    // MultiPoint
    template<>
	template<>
	void object::test<8>()
	{
        std::string wkt("MULTIPOINT(80 200, 240 200, 240 60, \
                    80 60, 80 200, 140 199, 120 120)");

        GeomPtr g(wktreader.read(wkt));
		GeomPtr simplified = TopologyPreservingSimplifier::simplify(g.get(), 10.0);

		ensure( "Simplified geometry is invalid!", simplified->isValid() );
        ensure_equals_geometry(g.get(), simplified.get() );
    }

    // MultiLineString
    template<>
	template<>
	void object::test<9>()
	{
        std::string wkt("MULTILINESTRING((0 0, 50 0, 70 0, 80 0, 100 0), \
                    (0 0, 50 1, 60 1, 100 0))");

        GeomPtr g(wktreader.read(wkt));
		GeomPtr simplified = TopologyPreservingSimplifier::simplify(g.get(), 10.0);

		ensure( "Simplified geometry is invalid!", simplified->isValid() );
        ensure_equals_geometry(g.get(), simplified.get() );
    }
    
    // GeometryCollection
    template<>
	template<>
	void object::test<10>()
	{
        std::string wkt("GEOMETRYCOLLECTION ( \
                    MULTIPOINT (80 200, 240 200, 240 60, 80 60, 80 200, 140 199, 120 120), \
                    POLYGON ((80 200, 240 200, 240 60, 80 60, 80 200)), \
                    LINESTRING (80 200, 240 200, 240 60, 80 60, 80 200, 140 199, 120 120))");

        GeomPtr g(wktreader.read(wkt));
		GeomPtr simplified = TopologyPreservingSimplifier::simplify(g.get(), 10.0);

		ensure( "Simplified geometry is invalid!", simplified->isValid() );
        ensure_equals_geometry(g.get(), simplified.get() );
    }
} // namespace tut

