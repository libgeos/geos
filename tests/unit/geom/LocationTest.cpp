// $Id: LocationTest.cpp 2344 2009-04-09 21:46:30Z mloskot $
// 
// Test Suite for geos::geom::Location class.

// tut
#include <tut.hpp>
// geos
#include <geos/geom/Location.h>
#include <geos/util/IllegalArgumentException.h>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_location_data
    {
		int undef;
		int interior;
		int boundary;
		int exterior;
		test_location_data()
			: undef(geos::geom::Location::UNDEF),
			interior(geos::geom::Location::INTERIOR),
			boundary(geos::geom::Location::BOUNDARY),
			exterior(geos::geom::Location::EXTERIOR)
		{}
	};

    typedef test_group<test_location_data> group;
    typedef group::object object;

    group test_location_group("geos::geom::Location");

    //
    // Test Cases
    //

    // Test of default constructor
    template<>
    template<>
    void object::test<1>()
    {
		ensure("NOTE: Location has no default constructor.", true);
    }

    // Test of toLocationSymbol()
    template<>
    template<>
    void object::test<2>()
    {
		using geos::geom::Location;

		ensure_equals( Location::toLocationSymbol(exterior), 'e' );
		ensure_equals( Location::toLocationSymbol(boundary), 'b' );
		ensure_equals( Location::toLocationSymbol(interior), 'i' );
		ensure_equals( Location::toLocationSymbol(undef), '-' );
    }

    // Test of toLocationSymbol() throwing IllegalArgumentException
    template<>
    template<>
    void object::test<3>()
    {
		using geos::geom::Location;

		try 
		{
			Location::toLocationSymbol(101);
			Location::toLocationSymbol(-101);
		
			fail("IllegalArgumentException expected"); 
		}
		catch ( geos::util::IllegalArgumentException const& e ) 
		{ 
			const char* msg = e.what(); // ok 
			ensure( msg != 0 );
		} 
    }

} // namespace tut

