// $Id$
// 
// Test Suite for geos::geom::prep::PreparedGeometryFactory class.

// TUT
#include <tut.h>
// GEOS
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/io/WKTReader.h>
#include <geos/util/IllegalArgumentException.h>
// STL
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
	}
	
    // Test passing null-pointer to prepare static method
	template<>
	template<>
	void object::test<2>()
	{
        // FIXME: null pointer throws segfault (Ticket #197)
        // ensure(0 == prep::PreparedGeometryFactory::prepare(0));
	}
	
    // Test passing null-pointer to create method
	template<>
	template<>
	void object::test<3>()
	{
		prep::PreparedGeometryFactory pgf;

        // FIXME: null pointer throws segfault (Ticket #197)
        // ensure(0 == pgf.create(0));
	}
	
    // Test prepare with empty POINT
	template<>
	template<>
	void object::test<4>()
	{
		GeometryFactory gf;
		Geometry* g = gf.createEmptyGeometry();
		ensure( 0 != g );
		
        prep::PreparedGeometry const* pg = prep::PreparedGeometryFactory::prepare(g);
		ensure( 0 != pg );

        // FIXME: pg leaks here, but we don't know how to destroy
	}
	
    // Test create with empty POINT
	template<>
	template<>
	void object::test<5>()
	{
		GeometryFactory gf;
		Geometry* g = gf.createEmptyGeometry();
		ensure( 0 != g );
		
		prep::PreparedGeometryFactory pgf;
		prep::PreparedGeometry const* pg = pgf.create(g);
		ensure( 0 != pg );
        // FIXME: pg leaks here, but we don't know how to destroy
	}
	

} // namespace tut

