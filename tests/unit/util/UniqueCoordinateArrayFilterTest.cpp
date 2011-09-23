// 
// Test Suite for geos::util::UniqueCoordinateArrayFilter class.

// tut
#include <tut.hpp>
// geos
#include <geos/util/UniqueCoordinateArrayFilter.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>
#include <string>

namespace tut
{
    //
    // Test Group
    //

    // Common data used in test cases.
    struct test_uniquecoordinatearrayfilter_data
	{
		typedef std::auto_ptr<geos::geom::Geometry> GeometryPtr;
		
		geos::geom::PrecisionModel pm_;
		geos::geom::GeometryFactory factory_;
		geos::io::WKTReader reader_;

		test_uniquecoordinatearrayfilter_data()
			: pm_(1), factory_(&pm_, 0), reader_(&factory_)
		{}		
	};

	typedef test_group<test_uniquecoordinatearrayfilter_data> group;
    typedef group::object object;

	group test_uniquecoordinatearrayfilter_group("geos::util::UniqueCoordinateArrayFilter");

    //
    // Test Cases
    //

    // Test of apply_ro() on MULTIPOINT
    template<>
    template<>
    void object::test<1>()
    {
		using geos::geom::Coordinate;
		
		// Create geometry from WKT
		const Coordinate::ConstVect::size_type size5 = 5;
		const std::string wkt("MULTIPOINT(10 10, 20 20, 30 30, 20 20, 10 10)");
		GeometryPtr geo(reader_.read(wkt));
		
		ensure_equals( geo->getGeometryTypeId(), geos::geom::GEOS_MULTIPOINT );
		std::auto_ptr<geos::geom::CoordinateSequence> cs;
		cs.reset(geo->getCoordinates());
		ensure_equals(cs->getSize(), size5 );
		
		// Create collection buffer for filtered coordinates
		const Coordinate::ConstVect::size_type size0 = 0;
		Coordinate::ConstVect coords;
		
		// Create filtering object		
		geos::util::UniqueCoordinateArrayFilter filter(coords);		
		
		ensure_equals( coords.size(), size0 );
		
		// Apply filter
		const Coordinate::ConstVect::size_type size3 = 3;
		geo->apply_ro(&filter);
		
		cs.reset(geo->getCoordinates());
		ensure_equals( cs->getSize(), size5 );
		ensure_equals( coords.size(), size3 );
		ensure_equals( coords.at(0)->x, 10 );
		ensure_equals( coords.at(0)->y, 10 );
		ensure_equals( coords.at(1)->x, 20 );
		ensure_equals( coords.at(1)->y, 20 );
		ensure_equals( coords.at(2)->x, 30 );
		ensure_equals( coords.at(2)->y, 30 );
    }	

} // namespace tut

