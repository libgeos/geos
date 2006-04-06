// $Id$
// 
// Test Suite for geos::geom::Polygon class.

// TUT
#include <tut.h>
// GEOS
#include <geos/geom/Polygon.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/LinearRing.h>
#include <geos/geom/Point.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
#include <geos/util/IllegalArgumentException.h>
// STL
#include <cmath>
#include <string>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_polygon_data
    {
		// Typedefs used as short names by test cases
		typedef geos::geom::Geometry* GeometryPtr;
		typedef geos::geom::Geometry const* GeometryCPtr;

		typedef geos::geom::Coordinate* CoordinatePtr;
		typedef geos::geom::Coordinate const* CoordinateCPtr;
		typedef geos::geom::CoordinateSequence* CoordSeqPtr;
		typedef geos::geom::CoordinateSequence const* CoordSeqCPtr;
		typedef geos::geom::CoordinateArraySequence* CoordArrayPtr;
		typedef geos::geom::CoordinateArraySequence const* CoordArrayCPtr;

		typedef geos::geom::Envelope* EnvelopePtr;
		typedef geos::geom::Envelope const* EnvelopeCPtr;
		typedef geos::geom::Point* PointPtr;
		typedef geos::geom::Point const* PointCPtr;
		typedef geos::geom::LineString* LineStringPtr;
		typedef geos::geom::LineString const* LineStringCPtr;
		typedef geos::geom::LinearRing* LinearRingPtr;
		typedef geos::geom::LinearRing const* LinearRingCPtr;
		typedef geos::geom::Polygon* PolygonPtr;
		typedef geos::geom::Polygon const* PolygonCPtr;

		geos::geom::PrecisionModel pm_;
		geos::geom::GeometryFactory factory_;
		geos::io::WKTReader reader_;

		geos::geom::Polygon empty_poly_;
		PolygonPtr poly_;
		const int poly_size_;

		test_polygon_data() 
			: pm_(1), factory_(&pm_, 0), reader_(&factory_), empty_poly_(0, 0, &factory_), poly_size_(7)
		{
			// Create non-empty LinearRing
			GeometryPtr geo = 0;
			geo = reader_.read("POLYGON((0 10, 5 5, 10 5, 15 10, 10 15, 5 15, 0 10))");
			poly_ = static_cast<PolygonPtr>(geo);
		}

		~test_polygon_data() 
		{
			// FREE MEMORY
			factory_.destroyGeometry(poly_);
		}
    };

    typedef test_group<test_polygon_data> group;
    typedef group::object object;

    group test_polygon_group("geos::geom::Polygon");

    //
    // Test Cases
    //

    // Test of user's constructor to create non-empty valid Polygon
    template<>
    template<>
    void object::test<1>()
    {
		using geos::geom::Coordinate;

		// Create non-empty Coordiantes sequence for Exterior LinearRing
		const int size = 7;
		CoordArrayPtr coords = new geos::geom::CoordinateArraySequence();
		ensure( "sequence is null pointer.", coords != 0 );

		coords->add(Coordinate(0, 10));
		coords->add(Coordinate(5, 5));
		coords->add(Coordinate(10, 5));
		coords->add(Coordinate(15, 10));
		coords->add(Coordinate(10, 15));
		coords->add(Coordinate(5, 15));
		coords->add(Coordinate(0, 10));

		ensure_equals( coords->size(), size );

		try
		{
			// Create non-empty LinearRing instance
			geos::geom::LinearRing ring(coords, &factory_);
			ensure( !ring.isEmpty() );
			ensure( ring.isClosed() );
			ensure( ring.isRing() );
			ensure( ring.isSimple() );

			// Exterior (clone is required here because Polygon takes ownership)
			GeometryPtr geo = ring.clone();
			LinearRingPtr exterior = static_cast<LinearRingPtr>(geo);
			
			// Create non-empty Polygon
			geos::geom::Polygon poly(exterior, 0, &factory_);
			
			ensure( !poly.isEmpty() );
			ensure( poly.isSimple() );
			ensure( poly.isValid() );

			ensure_equals( poly.getNumGeometries(), 1 );
			ensure_equals( poly.getNumInteriorRing(), 0 );
			ensure_equals( poly.getNumPoints(), size );
		}
		catch (geos::util::IllegalArgumentException const& e)
		{
			fail(e.what());
		}

    }

    // Test of copy constructor
    template<>
    template<>
    void object::test<2>()
    {
		geos::geom::Polygon copy(empty_poly_);

		ensure( copy.isEmpty() );
    }

	// Test of isEmpty() for empty Polygon
	template<>
	template<>
	void object::test<3>()
	{
		ensure( empty_poly_.isEmpty() );
	}

	// Test of isSimple() for empty Polygon
	template<>
	template<>
	void object::test<4>()
	{
		ensure( empty_poly_.isSimple() );
	}

	// Test of isValid() for empty Polygon
	template<>
	template<>
	void object::test<5>()
	{
		inform("Test is waiting for Bug #87 resolution.");

		// TODO - mloskot - is empty valid or not?
		//ensure( !ring.isValid() );
	}


	// Test of getEnvelope() for empty Polygon
	template<>
	template<>
	void object::test<6>()
	{
		inform( "Test waiting for resolution of getEnvelope() issue." );
		//http://geos.refractions.net/pipermail/geos-devel/2006-April/002123.html

		//EnvelopeCPtr envelope = empty_poly_.getEnvelopeInternal();
		//ensure( envelope != 0 );
		//ensure( envelope->isNull() );
	}

	// Test of getBoundary() for empty Polygon
	template<>
	template<>
	void object::test<7>()
	{
		GeometryPtr boundary = empty_poly_.getBoundary();	
		ensure( boundary != 0 );
		ensure( boundary->isEmpty() );
		factory_.destroyGeometry(boundary);
	}

	// Test of convexHull() for empty Polygon
	template<>
	template<>
	void object::test<8>()
	{
		GeometryPtr hull = empty_poly_.convexHull();	
		ensure( hull != 0 );
		ensure( hull->isEmpty() );
		factory_.destroyGeometry(hull);
	}

	// Test of getGeometryTypeId() for empty Polygon
	template<>
	template<>
	void object::test<9>()
	{
		ensure_equals( empty_poly_.getGeometryTypeId(), geos::geom::GEOS_POLYGON );
	}

	// Test of getDimension() for empty Polygon
	template<>
	template<>
	void object::test<10>()
	{
		ensure_equals( empty_poly_.getDimension(), geos::geom::Dimension::A );
	}

	// Test of getBoundaryDimension() for empty Polygon
	template<>
	template<>
	void object::test<11>()
	{
		ensure_equals( empty_poly_.getBoundaryDimension(), geos::geom::Dimension::L );
	}	

	// Test of getNumPoints() for empty Polygon
	template<>
	template<>
	void object::test<12>()
	{
		ensure_equals( empty_poly_.getNumPoints(), 0 );
	}

	// Test of getLength() for empty Polygon
	template<>
	template<>
	void object::test<13>()
	{
		ensure_equals( empty_poly_.getLength(), 0 );
	}

	// Test of getArea() for empty Polygon
	template<>
	template<>
	void object::test<14>()
	{
		ensure_equals( empty_poly_.getArea(), 0 );
	}
    
    // Test of isEmpty() for non-empty Polygon
    template<>
    template<>
    void object::test<15>()
	{
		ensure( poly_ != 0 );
		ensure( !poly_->isEmpty() );
	}

    // Test of getEnvelope() for non-empty Polygon
    template<>
    template<>
    void object::test<17>()
	{
		ensure( poly_ != 0 );

		GeometryPtr envelope = poly_->getEnvelope();	
		ensure( envelope != 0 );
		ensure( !envelope->isEmpty() );
		ensure_equals( envelope->getDimension(), geos::geom::Dimension::A );

		// FREE MEMORY
		factory_.destroyGeometry(envelope);
	}

	// Test of getBoundary() for non-empty Polygon
    template<>
    template<>
    void object::test<18>()
	{
		ensure( poly_ != 0 );

		GeometryPtr boundary = poly_->getBoundary();	
		ensure( boundary != 0 );

		// OGC 05-126, Version: 1.1.0, Chapter 6.1.10 Surface
		ensure( "[OGC] The boundary of Polygin is the set of closed Curves.", !boundary->isEmpty() );

		// FREE MEMORY
		factory_.destroyGeometry(boundary);
	}

	// Test of convexHull() for non-empty Polygon
    template<>
    template<>
    void object::test<19>()
	{
		ensure( poly_ != 0 );

		GeometryPtr hull = poly_->convexHull();	
		ensure( hull != 0 );
		ensure( !hull->isEmpty() );
		ensure_equals( hull->getGeometryTypeId(), geos::geom::GEOS_POLYGON );
		ensure_equals( hull->getDimension(), geos::geom::Dimension::A );

		// FREE MEMORY
		factory_.destroyGeometry(hull);
	}

	// Test of getGeometryTypeId() for non-empty Polygon
    template<>
    template<>
    void object::test<20>()
	{
		ensure( poly_ != 0 );
		ensure_equals( poly_->getGeometryTypeId(), geos::geom::GEOS_POLYGON );
	}

	// Test of getDimension() for non-empty Polygon
    template<>
    template<>
    void object::test<21>()
	{
		ensure( poly_ != 0 );
		ensure_equals( poly_->getDimension(), geos::geom::Dimension::A );
	}

	// Test of getBoundaryDimension() for non-empty Polygon
    template<>
    template<>
    void object::test<22>()
	{
		ensure( poly_ != 0 );
		ensure_equals( poly_->getBoundaryDimension(), geos::geom::Dimension::L );
	}

	// Test of getNumPoints() for non-empty Polygon
    template<>
    template<>
    void object::test<23>()
	{
		ensure( poly_ != 0 );
		ensure_equals( poly_->getNumPoints(), poly_size_ );
	}

	// Test of getLength() for non-empty Polygon
    template<>
    template<>
    void object::test<24>()
	{
		ensure( poly_ != 0 );
		ensure_not_equals( poly_->getLength(), 0 );

		const double tolerance = 0.0001;
		const double expected = 38.284271247461902;
		const double diff = std::fabs(poly_->getLength() - expected);
		ensure( diff <= tolerance );
	}

	// Test of getArea() for non-empty Polygon
    template<>
    template<>
    void object::test<25>()
	{
		ensure( poly_ != 0 );
		ensure_not_equals( poly_->getArea(), 0 );
	}
	
	// Test of getCoordinates() for non-empty Polygon
    template<>
    template<>
    void object::test<26>()
	{
		ensure( poly_ != 0 );

		// Caller takes ownership of 'coords'
		CoordSeqPtr coords = poly_->getCoordinates();
		ensure( coords != 0 );
		ensure( !coords->isEmpty() );
		ensure_equals( coords->getSize(), poly_->getNumPoints() );

		// FREE MEMORY
		delete coords;
	}

	// Test of clone() and equals() for non-empty Polygon
    template<>
    template<>
    void object::test<27>()
	{
		ensure( poly_ != 0 );

		GeometryPtr geo = poly_->clone();
		ensure( geo != 0 );
		ensure( geo->equals(poly_) );

		factory_.destroyGeometry(geo);
	}

	// Test of getExteriorRing() for non-empty Polygon
    template<>
    template<>
    void object::test<28>()
	{
		ensure( poly_ != 0 );

		LineStringCPtr ring = poly_->getExteriorRing();
		ensure( ring != 0 );
		ensure( ring->isRing() );
		ensure_equals( ring->getNumPoints(), poly_size_ );
	}

	// Test of getNumInteriorRing() for non-empty Polygon but without interior rings
    template<>
    template<>
    void object::test<29>()
	{
		ensure( poly_ != 0 );
		ensure_equals( poly_->getNumInteriorRing(), 0 );
	}

	// Test of getInteriorRingN() for non-empty Polygon with interior rings
    template<>
    template<>
    void object::test<30>()
	{
		const int holesNum = 1;

		GeometryPtr geo = 0;
		geo = reader_.read("POLYGON ((0 0, 100 0, 100 100, 0 100, 0 0), (1 1, 1 10, 10 10, 10 1, 1 1) )");
		ensure( geo != 0 );
		ensure_equals( geo->getGeometryTypeId(), geos::geom::GEOS_POLYGON );
		
		PolygonPtr poly = static_cast<PolygonPtr>(geo);
		ensure( poly != 0 );
		ensure_equals( poly->getNumInteriorRing(), holesNum );

		LineStringCPtr interior = poly->getInteriorRingN(0);
		ensure( interior != 0 );
		ensure( interior->isRing() );

		ensure_equals( interior->getGeometryTypeId(), geos::geom::GEOS_LINEARRING );

		factory_.destroyGeometry(geo);
	}

	// Test of getCoordiante() for non-empty Polygon
    template<>
    template<>
    void object::test<31>()
	{
		ensure( poly_ != 0 );
		// "POLYGON((0 10, 5 5, 10 5, 15 10, 10 15, 5 15, 0 10))"

		CoordinateCPtr coord = poly_->getCoordinate();
		ensure( coord != 0 );
		ensure_equals( coord->x, 0 );
		ensure_equals( coord->y, 10 );
	}

	// Test of getCoordiantes() for non-empty Polygon
    template<>
    template<>
    void object::test<32>()
	{
		ensure( poly_ != 0 );
		// "POLYGON((0 10, 5 5, 10 5, 15 10, 10 15, 5 15, 0 10))"

		CoordSeqPtr coords = poly_->getCoordinates();
		ensure( coords != 0 );
		ensure_equals( coords->getSize(), poly_size_ );

		// Check first and last coordinates
		const int lastPos = poly_size_ - 1;
		ensure_equals( coords->getAt(0), coords->getAt(lastPos) );

		// Check coordinate	from the middle of ring
		const int middlePos = 3;
		ensure_equals( coords->getAt(middlePos).x, 15 );
		ensure_equals( coords->getAt(middlePos).y, 10 );

		// FREE MEMORY
		delete coords;
	}

	// Test of getGeometryType() for non-empty Polygon
    template<>
    template<>
    void object::test<33>()
	{
		ensure( poly_ != 0 );

		const std::string type("Polygon");
		ensure_equals( poly_->getGeometryType(), type );
	}

	// Test of Point* getCentroid() const for empty Polygon
    template<>
    template<>
    void object::test<34>()
	{		
		PointPtr point = empty_poly_.getCentroid();
		ensure( point == 0 );
	}

	// Test of Geometry::getCentroid(Coordinate& ret) const for empty Polygon
    template<>
    template<>
    void object::test<35>()
	{	
		geos::geom::Coordinate centroid;
		bool isCentroid = empty_poly_.getCentroid(centroid);
		ensure( !isCentroid );
	}

	// Test of getCentroid() for non-empty Polygon
    template<>
    template<>
    void object::test<36>()
	{
		PointPtr point = poly_->getCentroid();
		ensure( point != 0 );
		ensure( !point->isEmpty() );
		ensure_equals( point->getGeometryTypeId(), geos::geom::GEOS_POINT );

		// FREE MEMORY
		factory_.destroyGeometry(point);
	}

	// Test of Geometry::getCentroid(Coordinate& ret) const for non-empty Polygon
    template<>
    template<>
    void object::test<37>()
	{	
		geos::geom::Coordinate centroid;
		bool isCentroid = poly_->getCentroid(centroid);
		ensure( isCentroid );
	}

	// Test of comparison of centroids returned by two versions of getCentroid() for non-empty Polygon
    template<>
    template<>
    void object::test<38>()
	{
		// First centroid
		PointPtr point = poly_->getCentroid();
		ensure( point != 0 );
		ensure( !point->isEmpty() );
		ensure_equals( point->getGeometryTypeId(), geos::geom::GEOS_POINT );

		CoordinateCPtr pointCoord = point->getCoordinate();
		ensure( pointCoord != 0 );
		geos::geom::Coordinate pointCentr(*pointCoord);
		// FREE MEMORY
		factory_.destroyGeometry(point);

		// Second centroid
		geos::geom::Coordinate coordCentr;
		bool isCentroid = poly_->getCentroid(coordCentr);
		ensure( isCentroid );

		// TODO
		inform("[TODO] Check Polygon::getCentroid() functions.");
		
		// Comparison of two centroids
		ensure_equals( coordCentr, pointCentr );
	}


} // namespace tut
