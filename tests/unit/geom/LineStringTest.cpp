// 
// Test Suite for geos::geom::LineString class.

// tut
#include <tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/LineString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/util/GEOSException.h>
#include <geos/util/IllegalArgumentException.h>
// std 
#include <string>
#include <cmath>
#include <cassert>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_linestring_data
    {
		// Typedefs used as short names by test cases
		typedef std::auto_ptr<geos::geom::LineString> LineStringAutoPtr;

		geos::geom::PrecisionModel pm_;
		geos::geom::GeometryFactory factory_;
		geos::io::WKTReader reader_;
		
		LineStringPtr empty_line_;

		test_linestring_data()
			: pm_(1000), factory_(&pm_, 0), reader_(&factory_),
			empty_line_(factory_.createLineString(new geos::geom::CoordinateArraySequence()))
		{
            assert(0 != empty_line_);
        }
		
        ~test_linestring_data()
        {
            factory_.destroyGeometry(empty_line_);
            empty_line_ = 0;
        }
    };

    typedef test_group<test_linestring_data> group;
    typedef group::object object;

    group test_linestring_group("geos::geom::LineString");

    //
    // Test Cases
    //

    // Test of empty LineString created with user's constructor
    template<>
    template<>
    void object::test<1>()
    {
		using geos::geom::Coordinate;

		// Empty sequence of coordiantes
		CoordArrayPtr pseq = new geos::geom::CoordinateArraySequence();
		ensure( "sequence is null pointer.", pseq != 0 );

		// Create empty linstring instance
		LineStringAutoPtr ls(factory_.createLineString(pseq));

		ensure( ls->isEmpty() );
		ensure( ls->isSimple() );
		ensure( ls->isValid() );
    }

    // Test of non-tempty LineString created with user's constructor
    template<>
    template<>
    void object::test<2>()
    {
		using geos::geom::Coordinate;

		// Non-empty sequence of coordiantes
		const size_t size3 = 3;

		CoordArrayPtr pseq = new geos::geom::CoordinateArraySequence();
		ensure( "sequence is null pointer.", pseq != 0 );
		
		pseq->add(Coordinate(0, 0, 0));
		pseq->add(Coordinate(5, 5, 5));
		pseq->add(Coordinate(10, 10, 10));

		ensure_equals( pseq->size(), size3 );

		// Create non-empty linstring instance
		LineStringAutoPtr ls(factory_.createLineString(pseq));

		ensure( !ls->isEmpty() );
		ensure( ls->isSimple() );
		ensure( ls->isValid() );

		ensure( !ls->isClosed() );
		ensure( !ls->isRing() );

		GeometryPtr geo = 0;
		geo = ls->getEnvelope();
		ensure( geo != 0 );
		ensure( !geo->isEmpty() );
		factory_.destroyGeometry(geo);

		geo = ls->getBoundary();
		ensure( geo != 0 );
		ensure( !geo->isEmpty() );
		factory_.destroyGeometry(geo);

		geo = ls->convexHull();
		ensure( geo != 0 );
		ensure( !geo->isEmpty() );
		factory_.destroyGeometry(geo);

		ensure_equals( ls->getGeometryTypeId(), geos::geom::GEOS_LINESTRING );
		ensure_equals( ls->getDimension(), geos::geom::Dimension::L );
		ensure_equals( ls->getBoundaryDimension(), geos::geom::Dimension::P );
        ensure_equals( ls->getCoordinateDimension(), 3 );
		ensure_equals( ls->getNumPoints(), size3 );
		ensure_equals( ls->getArea(), 0.0 );
		ensure( ls->getLength() != 0.0 );
	}

	// Test of incomplete LineString user's constructor throwing the IllegalArgumentException
    template<>
    template<>
    void object::test<3>()
    {
		// Single-element sequence of coordiantes
		CoordArrayPtr pseq = 0;
		try
		{
			pseq = new geos::geom::CoordinateArraySequence();
			ensure( "sequence is null pointer.", pseq != 0 );
			pseq->add(geos::geom::Coordinate(0, 0, 0));
			ensure_equals( pseq->size(), 1u );

			// Create incomplete linstring
			LineStringAutoPtr ls(factory_.createLineString(pseq));
			fail("IllegalArgumentException expected.");
		}
		catch (geos::util::IllegalArgumentException const& e)
		{
			// TODO - mloskot - is waiting for "exception-safety" and bugs resolution
			// If removed, memory 3 leaks occur
			//delete pseq;

			const char* msg = e.what(); // OK
			ensure( msg != 0 );
		}
	}

    // Test of copy constructor
    template<>
    template<>
    void object::test<4>()
    {
		using geos::geom::Coordinate;

		// Non-empty sequence of coordiantes
		const size_t size = 3;

		CoordArrayPtr pseq = new geos::geom::CoordinateArraySequence();
		ensure( "sequence is null pointer.", pseq != 0 );
		
		pseq->add(Coordinate(0, 0, 0));
		pseq->add(Coordinate(5, 5, 5));
		pseq->add(Coordinate(10, 10, 10));

		ensure_equals( pseq->size(), size );

		// Create examplar of linstring instance
		LineStringAutoPtr examplar(factory_.createLineString(pseq));

		// Create copy
		LineStringAutoPtr copy(dynamic_cast<geos::geom::LineString*>(examplar->clone()));

		ensure( 0 != copy.get() );

		ensure( !copy->isEmpty() );
		ensure( copy->isSimple() );
		ensure( copy->isValid() );

		ensure( !copy->isClosed() );
		ensure( !copy->isRing() );

		GeometryPtr geo = 0;
		geo = copy->getEnvelope();
		ensure( geo != 0 );
		ensure( !geo->isEmpty() );
		factory_.destroyGeometry(geo);

		geo = copy->getBoundary();
		ensure( geo != 0 );
		ensure( !geo->isEmpty() );
		factory_.destroyGeometry(geo);

		geo = copy->convexHull();
		ensure( geo != 0 );
		ensure( !geo->isEmpty() );
		factory_.destroyGeometry(geo);

		ensure_equals( copy->getGeometryTypeId(), geos::geom::GEOS_LINESTRING );
		ensure_equals( copy->getDimension(), geos::geom::Dimension::L );
		ensure_equals( copy->getBoundaryDimension(), geos::geom::Dimension::P );
		ensure_equals( copy->getNumPoints(), size );
		ensure_equals( copy->getArea(), 0.0 );
		ensure( copy->getLength() != 0.0 );
	}

    // Test of isClosed() and isRing() for empty linestring
    template<>
    template<>
    void object::test<5>()
	{
		ensure( !empty_line_->isClosed() );
		ensure( !empty_line_->isRing() );
	}

    // Test of getEnvelope() for empty linestring
    template<>
    template<>
    void object::test<6>()
	{
		GeometryPtr geo = 0;
		geo = empty_line_->getEnvelope();	
		ensure( geo != 0 );
		ensure( geo->isEmpty() );
		factory_.destroyGeometry(geo);
	}

    // Test of getBoundary() for empty linestring
    template<>
    template<>
    void object::test<7>()
	{
		GeometryPtr geo = 0;
		geo = empty_line_->getBoundary();	
		ensure( geo != 0 );
		ensure( geo->isEmpty() );
		factory_.destroyGeometry(geo);
	}

    // Test of convexHull() for empty linestring
    template<>
    template<>
    void object::test<8>()
	{
		GeometryPtr geo = 0;
		geo = empty_line_->convexHull();	
		ensure( geo != 0 );
		ensure( geo->isEmpty() );
		factory_.destroyGeometry(geo);
	}

    // Test of getGeometryTypeId() for empty linestring
    template<>
    template<>
    void object::test<9>()
	{
		ensure_equals( empty_line_->getGeometryTypeId(), geos::geom::GEOS_LINESTRING );
	}

	// Test of getDimension() for empty linestring
    template<>
    template<>
    void object::test<10>()
	{
		ensure_equals( empty_line_->getDimension(), geos::geom::Dimension::L );
	}

	// Test of getBoundaryDimension() for empty linestring
    template<>
    template<>
    void object::test<11>()
	{
		ensure_equals( empty_line_->getBoundaryDimension(), geos::geom::Dimension::P );
	}	

	// Test of getNumPoints() for empty linestring
    template<>
    template<>
    void object::test<12>()
	{
		ensure_equals( empty_line_->getNumPoints(), 0u );
	}

	// Test of getLength() for empty linestring
    template<>
    template<>
    void object::test<13>()
	{
		ensure_equals( empty_line_->getLength(), 0.0 );
	}

	// Test of getArea() for empty linestring
    template<>
    template<>
    void object::test<14>()
	{
		ensure_equals( empty_line_->getArea(), 0.0 );
	}

    // Test of isClosed() and isRing() for non-empty linestring
    template<>
    template<>
    void object::test<15>()
	{
		GeometryPtr geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
		ensure( geo != 0 );

		LineStringPtr line = dynamic_cast<LineStringPtr>(geo);
		ensure(line != 0);

		ensure( !line->isEmpty() );
		ensure( !line->isClosed() );
		ensure( !line->isRing() );
        ensure( line->getCoordinateDimension() == 2 );
		
		// FREE TESTED LINESTRING
		factory_.destroyGeometry(line);
	}

    // Test of getEnvelope() for non-empty linestring
    template<>
    template<>
    void object::test<16>()
	{
		GeometryPtr geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
		ensure( geo != 0 );

		LineStringPtr line = dynamic_cast<LineStringPtr>(geo);
		ensure(line != 0);
		
		GeometryPtr envelope = line->getEnvelope();	
		ensure( envelope != 0 );
		ensure( !envelope->isEmpty() );
		ensure_equals( envelope->getDimension(), geos::geom::Dimension::A );

		factory_.destroyGeometry(envelope);

		// FREE TESTED LINESTRING
		factory_.destroyGeometry(line);
	}

	// Test of getBoundary() for non-empty linestring
    template<>
    template<>
    void object::test<17>()
	{
		GeometryPtr geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
		ensure( geo != 0 );

		LineStringPtr line = dynamic_cast<LineStringPtr>(geo);
		ensure(line != 0);
		
		GeometryPtr boundary = line->getBoundary();	
		ensure( boundary != 0 );
		ensure( !boundary->isEmpty() );
		ensure_equals( boundary->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT );
		ensure_equals( boundary->getDimension(), geos::geom::Dimension::P );
		factory_.destroyGeometry(boundary);

		// FREE TESTED LINESTRING
		factory_.destroyGeometry(line);
	}

	// Test of convexHull() for non-empty linestring
    template<>
    template<>
    void object::test<18>()
	{
		GeometryPtr geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
		ensure( geo != 0 );

		LineStringPtr line = dynamic_cast<LineStringPtr>(geo);
		ensure(line != 0);
		
		GeometryPtr hull = line->convexHull();	
		ensure( hull != 0 );
		ensure( !hull->isEmpty() );
		ensure_equals( hull->getGeometryTypeId(), geos::geom::GEOS_POLYGON );
		ensure_equals( hull->getDimension(), geos::geom::Dimension::A );
		factory_.destroyGeometry(hull);

		// FREE TESTED LINESTRING
		factory_.destroyGeometry(line);
	}

	// Test of getGeometryTypeId() for non-empty linestring
    template<>
    template<>
    void object::test<19>()
	{
		GeometryPtr geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
		ensure( geo != 0 );

		ensure_equals( geo->getGeometryTypeId(), geos::geom::GEOS_LINESTRING );

		// FREE TESTED LINESTRING
		factory_.destroyGeometry(geo);
	}

	// Test of getDimension() for non-empty linestring
    template<>
    template<>
    void object::test<20>()
	{
		GeometryPtr geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
		ensure( geo != 0 );

		ensure_equals( geo->getDimension(), geos::geom::Dimension::L );

		// FREE TESTED LINESTRING
		factory_.destroyGeometry(geo);
	}

	// Test of getBoundaryDimension() for non-empty linestring
    template<>
    template<>
    void object::test<21>()
	{
		GeometryPtr geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
		ensure( geo != 0 );

		ensure_equals( geo->getBoundaryDimension(), geos::geom::Dimension::P );

		// FREE TESTED LINESTRING
		factory_.destroyGeometry(geo);
	}

	// Test of getNumPoints() for non-empty linestring
    template<>
    template<>
    void object::test<22>()
	{
		const size_t size = 4;
		GeometryPtr geo = reader_.read("LINESTRING (0 0, 5 5, 10 5, 10 10)");
		ensure( geo != 0 );

		ensure_equals( geo->getNumPoints(), size );

		// FREE TESTED LINESTRING
		factory_.destroyGeometry(geo);
	}

	// Test of getLength() for non-empty linestring
    template<>
    template<>
    void object::test<23>()
	{
		const double tolerance = 0.0001;
		const double expected = 2 * 14.142135600000000;
		GeometryPtr geo = reader_.read("LINESTRING (0 0, 10 10, 20 0)");
		ensure( geo != 0 );

		ensure( geo->getLength() != 0.0 );

		const double diff = std::fabs(geo->getLength() - expected);
		ensure( diff <= tolerance );

		// FREE TESTED LINESTRING
		factory_.destroyGeometry(geo);
	}

	// Test of getArea() for non-empty linestring
    template<>
    template<>
    void object::test<24>()
	{
		GeometryPtr geo = reader_.read("LINESTRING (0 0, 10 10, 20 0)");
		ensure( geo != 0 );

		ensure_equals( geo->getArea(), 0.0 );

		// FREE TESTED LINESTRING
		factory_.destroyGeometry(geo);
	}

	// Test of getGeometryType() for non-empty Polygon
    template<>
    template<>
    void object::test<25>()
	{
		GeometryPtr geo = reader_.read("LINESTRING (0 0, 10 10, 20 0)");
		ensure( geo != 0 );

		const std::string type("LineString");
		ensure_equals( geo->getGeometryType(), type );

		// FREE MEMORY
		factory_.destroyGeometry(geo);
	}

} // namespace tut

