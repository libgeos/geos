// 
// Test Suite for geos::geom::MultiPoint class.

// tut
#include <tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/MultiPoint.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/ParseException.h>
#include <geos/io/WKTReader.h>
// std
#include <cmath>

namespace tut
{
	//
	// Test Group
	//

	// Common data used by tests
	struct test_multipoint_data
	{
		typedef std::auto_ptr<geos::geom::MultiPoint> MultiPointAutoPtr;

		geos::geom::PrecisionModel pm_;
		geos::geom::GeometryFactory factory_;
		geos::io::WKTReader reader_;

		MultiPointAutoPtr empty_mp_;
		MultiPointPtr mp_;
		const size_t mp_size_;

		test_multipoint_data()
			:
			pm_(1.0), factory_(&pm_, 0), reader_(&factory_),
			empty_mp_(factory_.createMultiPoint()), mp_size_(5)
		{
			// Create non-empty MultiPoint
			GeometryPtr geo = 0;
			geo = reader_.read("MULTIPOINT(0 0, 5 5, 10 10, 15 15, 20 20)");
			mp_ = dynamic_cast<MultiPointPtr>(geo);
		}

		~test_multipoint_data()
		{
			factory_.destroyGeometry(mp_);
		}

    private:
        // Declare type as noncopyable
        test_multipoint_data(const test_multipoint_data& other);
        test_multipoint_data& operator=(const test_multipoint_data& rhs);
	};

	typedef test_group<test_multipoint_data> group;
	typedef group::object object;

	group test_multipoint_group("geos::geom::MultiPoint");

	//
	// Test Cases
	//

	// Test of user's constructor
	template<>
	template<>
	void object::test<1>()
	{
		const size_t size0 = 0;
		MultiPointAutoPtr mp(factory_.createMultiPoint());
		
		ensure( mp->isEmpty() );
		ensure( mp->isSimple() );
		ensure( mp->isValid() );
		ensure( mp->getCentroid() == 0 );
		ensure_equals( mp->getNumPoints(), size0 );
		ensure_equals( mp->getNumGeometries(), size0 );
	}

	// Test of copy constructor
	template<>
	template<>
	void object::test<2>()
	{
		const size_t size0 = 0;
		MultiPointAutoPtr copy(dynamic_cast<geos::geom::MultiPoint*>(empty_mp_->clone()));
		ensure( 0 != copy.get() );
		
		ensure( copy->isEmpty() );
		ensure( copy->isSimple() );
		ensure( copy->isValid() );
		ensure( copy->getCentroid() == 0 );
		ensure_equals( copy->getNumPoints(), size0 );
		ensure_equals( copy->getNumGeometries(), size0 );
	}

	// Test of empty MultiPoint constructed by WKTReader
	template<>
	template<>
	void object::test<3>()
	{
		const size_t size0 = 0;
		GeometryPtr geo = reader_.read("MULTIPOINT EMPTY");
		MultiPointPtr mp = dynamic_cast<MultiPointPtr>(geo);

		ensure( mp->isEmpty() );
		ensure( mp->isSimple() );
		ensure( mp->isValid() );
		ensure( mp->getCentroid() == 0 );
		ensure_equals( mp->getNumPoints(), size0 );
		ensure_equals( mp->getNumGeometries(), size0 );

		// FREE MEMORY
		factory_.destroyGeometry(geo);
	}

	// Test of isEmpty() for empty MultiPoint
	template<>
	template<>
	void object::test<4>()
	{
		ensure( empty_mp_->isEmpty() );
	}

	// Test of isSimple() for empty MultiPoint
	template<>
	template<>
	void object::test<5>()
	{
		ensure( empty_mp_->isSimple() );
	}

	// Test of isValid() for empty MultiPoint
	template<>
	template<>
	void object::test<6>()
	{
		ensure( empty_mp_->isValid() );
	}

	// Test of getEnvelope() for empty MultiPoint
	template<>
	template<>
	void object::test<7>()
	{
		GeometryPtr envelope = empty_mp_->getEnvelope();	
		ensure( envelope != 0 );
		ensure( envelope->isEmpty() );
		factory_.destroyGeometry(envelope);
	}

	// Test of getBoundary() for empty MultiPoint
	template<>
	template<>
	void object::test<8>()
	{
		GeometryPtr boundary = empty_mp_->getBoundary();	
		ensure( boundary != 0 );
		ensure( boundary->isEmpty() );
		factory_.destroyGeometry(boundary);
	}

	// Test of convexHull() for empty MultiPoint
	template<>
	template<>
	void object::test<9>()
	{
		GeometryPtr hull = empty_mp_->convexHull();	
		ensure( hull != 0 );
		ensure( hull->isEmpty() );
		factory_.destroyGeometry(hull);
	}

	// Test of getGeometryTypeId() for empty MultiPoint
	template<>
	template<>
	void object::test<10>()
	{
		ensure_equals( empty_mp_->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT );
	}

	// Test of getGeometryType() for empty MultiPoint
	template<>
	template<>
	void object::test<11>()
	{
		const std::string type("MultiPoint");
		ensure_equals( empty_mp_->getGeometryType(), type );
	}

	// Test of getDimension() for empty MultiPoint
	template<>
	template<>
	void object::test<12>()
	{
		ensure_equals( empty_mp_->getDimension(), geos::geom::Dimension::P );
	}

	// Test of getBoundaryDimension() for empty MultiPoint
	template<>
	template<>
	void object::test<13>()
	{
		ensure_equals( empty_mp_->getBoundaryDimension(), geos::geom::Dimension::False );
	}	

	// Test of getNumPoints() for empty MultiPoint
	template<>
	template<>
	void object::test<14>()
	{
		ensure_equals( empty_mp_->getNumPoints(), 0u );
	}

	// Test of getLength() for empty MultiPoint
	template<>
	template<>
	void object::test<15>()
	{
		ensure_equals( empty_mp_->getLength(), 0.0 );
	}

	// Test of getArea() for empty MultiPoint
	template<>
	template<>
	void object::test<16>()
	{
		ensure_equals( empty_mp_->getArea(), 0.0 );
	}

	// Test of isEmpty() for non-empty LinearRing
	template<>
	template<>
	void object::test<17>()
	{
		ensure(mp_ != 0);
		ensure( !mp_->isEmpty() );
	}

	// Test of getEnvelope() for non-empty LinearRing
	template<>
	template<>
	void object::test<18>()
	{
		ensure(mp_ != 0);

		GeometryPtr envelope = mp_->getEnvelope();	
		ensure( envelope != 0 );
		ensure( !envelope->isEmpty() );
		ensure_equals( envelope->getDimension(), geos::geom::Dimension::A );

		// FREE MEMORY
		factory_.destroyGeometry(envelope);
	}

	// Test of getBoundary() for non-empty LinearRing
	template<>
	template<>
	void object::test<19>()
	{
		ensure(mp_ != 0);

		GeometryPtr boundary = mp_->getBoundary();	
		ensure( boundary != 0 );

		// OGC 05-126, Version: 1.1.0, Chapter 6.1.5 MultiPoint
		ensure( "[OGC] The boundary of a MultiPoint is the empty set.", boundary->isEmpty() );

		// FREE MEMORY
		factory_.destroyGeometry(boundary);
	}

	// Test of convexHull() for non-empty LinearRing
	template<>
	template<>
	void object::test<20>()
	{
		ensure(mp_ != 0);

		GeometryPtr hull = mp_->convexHull();	
		ensure( hull != 0 );
		ensure( !hull->isEmpty() );
		ensure_equals( hull->getGeometryTypeId(), geos::geom::GEOS_LINESTRING );
		ensure_equals( hull->getDimension(), geos::geom::Dimension::L );

		// FREE MEMORY
		factory_.destroyGeometry(hull);
	}

	// Test of getGeometryTypeId() for non-empty LinearRing
	template<>
	template<>
	void object::test<21>()
	{
		ensure(mp_ != 0);
		ensure_equals( mp_->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT );
	}

	// Test of getGeometryType() for non-empty Polygon
	template<>
	template<>
	void object::test<22>()
	{
		ensure( mp_ != 0 );

		const std::string type("MultiPoint");
		ensure_equals( mp_->getGeometryType(), type );
	}

	// Test of getDimension() for non-empty LinearRing
	template<>
	template<>
	void object::test<23>()
	{
		ensure(mp_ != 0);
		ensure_equals( mp_->getDimension(), geos::geom::Dimension::P );
	}

	// Test of getBoundaryDimension() for non-empty LinearRing
	template<>
	template<>
	void object::test<24>()
	{
		ensure(mp_ != 0);
		ensure_equals( mp_->getBoundaryDimension(), geos::geom::Dimension::False );
	}

	// Test of getNumPoints() for non-empty LinearRing
	template<>
	template<>
	void object::test<25>()
	{
		ensure(mp_ != 0);
		ensure_equals( mp_->getNumPoints(), mp_size_ );
	}

	// Test of getLength() for non-empty LinearRing
	template<>
	template<>
	void object::test<26>()
	{
		ensure(mp_ != 0);
		ensure_equals( mp_->getLength(), 0.0 );
	}

	// Test of getArea() for non-empty LinearRing
	template<>
	template<>
	void object::test<27>()
	{
		ensure(mp_ != 0);
		ensure_equals( mp_->getArea(), 0.0 );
	}

	// Test of ParseException thrown when constructing MultiPoint from invalind WKT
	template<>
	template<>
	void object::test<28>()
	{
		try
		{
			GeometryPtr geo = reader_.read("MULTIPOINT(0 0, 5)");
			ensure(geo != 0);

			// FREE TESTED LINEARRING
			factory_.destroyGeometry(geo);

			fail("ParseException expected.");
		}
		catch (geos::io::ParseException const& e)
		{
			const char* msg = e.what(); // ok 
			ensure( msg != 0 );
		}
	}

} // namespace tut

