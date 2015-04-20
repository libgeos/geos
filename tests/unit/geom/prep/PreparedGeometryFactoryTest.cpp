// 
// Test Suite for geos::geom::prep::PreparedGeometryFactory class.

// tut
#include <tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h>
// std
#include <vector>
#include <cstring> // std::size_t

using namespace geos::geom;

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_preparedgeometryfactory_data
    {
        GeometryPtr g_;
        PreparedGeometryPtr pg_;
        geos::geom::PrecisionModel pm_;
        geos::geom::GeometryFactory factory_;
        geos::io::WKTReader reader_;

        test_preparedgeometryfactory_data()
            : g_(0), pg_(0), pm_(1.0), factory_(&pm_), reader_(&factory_)
        {
            assert(0 == g_);
            assert(0 == pg_);
        }
        
        ~test_preparedgeometryfactory_data()
        {
            // FREE MEMORY per test case
            prep::PreparedGeometryFactory::destroy(pg_);
            factory_.destroyGeometry(g_);
            pg_ = 0;
            g_ = 0;
        }

    };

    typedef test_group<test_preparedgeometryfactory_data> group;
    typedef group::object object;

    group test_preparedgeometryfactory_data("geos::geom::prep::PreparedGeometryFactory");

    //
    // Test Cases
    //

    // Test of default constructor
    template<>
    template<>
    void object::test<1>()
    {
        prep::PreparedGeometryFactory pgf;
        ::geos::ignore_unused_variable_warning(pgf);
    }

    // Test passing null-pointer to prepare static method
    template<>
    template<>
    void object::test<2>()
    {
        try
        {
            GeometryPtr nullgeom = 0; // intentionally nullptr

            prep::PreparedGeometryFactory::prepare(nullgeom);

            fail("IllegalArgumentException expected");
        }
        catch (geos::util::IllegalArgumentException const& e)
        {
            const char* msg = e.what(); // ok
            ensure( msg != 0 );
        }
    }

    // Test passing null-pointer to create method
    template<>
    template<>
    void object::test<3>()
    {
        try
        {
            GeometryPtr nullgeom = 0; // intentionally nullptr

            prep::PreparedGeometryFactory pgf;
            pgf.create(nullgeom);

            fail("IllegalArgumentException expected");
        }
        catch (geos::util::IllegalArgumentException const& e)
        {
            const char* msg = e.what(); // ok
            ensure( msg != 0 );
        }
    }

    // Test prepare empty GEOMETRY
    template<>
    template<>
    void object::test<4>()
    {
        g_ = factory_.createEmptyGeometry();
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );

        ensure_equals_geometry( g_, pg_ );
    }

    // Test create empty GEOMETRY
    template<>
    template<>
    void object::test<5>()
    {
        g_ = factory_.createEmptyGeometry();
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test prepare empty POINT
    template<>
    template<>
    void object::test<6>()
    {
        g_ = factory_.createPoint();
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }

    // Test create empty POINT
    template<>
    template<>
    void object::test<7>()
    {
        g_ = factory_.createPoint();
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        ensure( 0 != pg_ );

        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test prepare empty LINESTRING
    template<>
    template<>
    void object::test<8>()
    {
        g_ = factory_.createLineString();
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );

        ensure_equals_geometry( g_, pg_ );
    }

    // Test create empty LINESTRING
    template<>
    template<>
    void object::test<9>()
    {
        g_ = factory_.createLineString();
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        
        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test prepare empty POLYGON
    template<>
    template<>
    void object::test<10>()
    {
        g_ = factory_.createPolygon();
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }

    // Test create empty POLYGON
    template<>
    template<>
    void object::test<11>()
    {
        g_ = factory_.createPolygon();
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test prepare empty MULTIPOINT
    template<>
    template<>
    void object::test<12>()
    {
        g_ = factory_.createMultiPoint();
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }

    // Test create empty MULTIPOINT
    template<>
    template<>
    void object::test<13>()
    {
        g_ = factory_.createMultiPoint();
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test prepare empty MULTILINESTRING
    template<>
    template<>
    void object::test<14>()
    {
        g_ = factory_.createMultiLineString();
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }

    // Test create empty MULTILINESTRING
    template<>
    template<>
    void object::test<15>()
    {
        g_ = factory_.createMultiLineString();
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test prepare empty MULTIPOLYGON
    template<>
    template<>
    void object::test<16>()
    {
        g_ = factory_.createMultiPolygon();
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }

    // Test create empty MULTIPOLYGON
    template<>
    template<>
    void object::test<17>()
    {
        g_ = factory_.createMultiPolygon();
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }

    // Test prepare non-empty POINT
    template<>
    template<>
    void object::test<18>()
    {
        g_ = reader_.read("POINT(1.234 5.678)");
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test create non-empty POINT
    template<>
    template<>
    void object::test<19>()
    {
        g_ = reader_.read("POINT(1.234 5.678)");
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }

    // Test prepare non-empty LINESTRING
    template<>
    template<>
    void object::test<20>()
    {
		g_ = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );

        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test create non-empty LINESTRING
    template<>
    template<>
    void object::test<21>()
    {
		g_ = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test prepare non-empty LINESTRING
    template<>
    template<>
    void object::test<22>()
    {
        g_ = reader_.read("POLYGON((0 10, 5 5, 10 5, 15 10, 10 15, 5 15, 0 10))");
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );

        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test create non-empty LINESTRING
    template<>
    template<>
    void object::test<23>()
    {
        g_ = reader_.read("POLYGON((0 10, 5 5, 10 5, 15 10, 10 15, 5 15, 0 10))");
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }
  
    // Test prepare non-empty MULTIPOINT
    template<>
    template<>
    void object::test<24>()
    {
        g_ = reader_.read("MULTIPOINT(0 0, 5 5, 10 10, 15 15, 20 20)");
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test create non-empty MULTIPOINT
    template<>
    template<>
    void object::test<25>()
    {
        g_ = reader_.read("MULTIPOINT(0 0, 5 5, 10 10, 15 15, 20 20)");
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }

    // Test prepare non-empty MULTILINESTRING
    template<>
    template<>
    void object::test<26>()
    {
		g_ = reader_.read("MULTILINESTRING ((20 120, 120 20), (20 20, 120 120))");
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );

        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test create non-empty MULTILINESTRING
    template<>
    template<>
    void object::test<27>()
    {
		g_ = reader_.read("MULTILINESTRING ((20 120, 120 20), (20 20, 120 120))");
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test prepare non-empty POLYGON
    template<>
    template<>
    void object::test<28>()
    {
        g_ = reader_.read("MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 2 6, 6 4, 2 2)),((60 60, 60 50, 70 40, 60 60)))");
        ensure( 0 != g_ );
        
        pg_ = prep::PreparedGeometryFactory::prepare(g_);
        ensure( 0 != pg_ );

        ensure_equals_geometry( g_, pg_ );
    }
    
    // Test create non-empty POLYGON
    template<>
    template<>
    void object::test<29>()
    {
        g_ = reader_.read("MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 2 6, 6 4, 2 2)),((60 60, 60 50, 70 40, 60 60)))");
        ensure( 0 != g_ );
        
        prep::PreparedGeometryFactory pgf;
        pg_ = pgf.create(g_);
        ensure( 0 != pg_ );
        
        ensure_equals_geometry( g_, pg_ );
    }

} // namespace tut

