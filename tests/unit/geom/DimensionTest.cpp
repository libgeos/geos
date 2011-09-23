// 
// Test Suite for geos::geom::Location class.

// tut
#include <tut.hpp>
// geos
#include <geos/geom/Dimension.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util.h>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_dimension_data
    {
		int nomatter;
		int yes;
		int no;
		int point;
		int curve;
		int surface;

		test_dimension_data()
			:
			nomatter(geos::geom::Dimension::DONTCARE),
			yes(geos::geom::Dimension::True),
			no(geos::geom::Dimension::False),
			point(geos::geom::Dimension::P),
			curve(geos::geom::Dimension::L),
			surface(geos::geom::Dimension::A)
		{}
	};

    typedef test_group<test_dimension_data> group;
    typedef group::object object;

    group test_dimension_group("geos::geom::Dimension");

    //
    // Test Cases
    //

    // Test of default constructor
    template<>
    template<>
    void object::test<1>()
    {
		// Just compile-time test
		geos::geom::Dimension dim;
        ::geos::ignore_unused_variable_warning(dim);
    }

    // Test of toDimensionSymbol()
    template<>
    template<>
    void object::test<2>()
    {
		using geos::geom::Dimension;

		try
		{
			ensure_equals( Dimension::toDimensionSymbol(nomatter), '*' );
			ensure_equals( Dimension::toDimensionSymbol(yes), 'T' );
			ensure_equals( Dimension::toDimensionSymbol(no), 'F' );
			ensure_equals( Dimension::toDimensionSymbol(point), '0' );
			ensure_equals( Dimension::toDimensionSymbol(curve), '1' );
			ensure_equals( Dimension::toDimensionSymbol(surface), '2' );
		}
		catch ( geos::util::IllegalArgumentException const& e ) 
		{
			fail( e.what() ); 
		}
    }

    // Test of toDimensionSymbol() throwing IllegalArgumentException
    template<>
    template<>
    void object::test<3>()
    {
		using geos::geom::Dimension;

		try 
		{
			Dimension::toDimensionSymbol(101);
			Dimension::toDimensionSymbol(-101);
		
			fail("IllegalArgumentException expected"); 
		}
		catch ( geos::util::IllegalArgumentException const& e ) 
		{ 
			const char* msg = e.what(); // ok 
			ensure( msg != 0 );
		} 
    }

    // Test of toDimensionValue()
    template<>
    template<>
    void object::test<4>()
    {
		using geos::geom::Dimension;

		try
		{
			ensure_equals( Dimension::toDimensionValue('*'), nomatter );
			ensure_equals( Dimension::toDimensionValue('F'), no );
			ensure_equals( Dimension::toDimensionValue('f'), no );
			ensure_equals( Dimension::toDimensionValue('T') , yes );
			ensure_equals( Dimension::toDimensionValue('t') , yes );
			ensure_equals( Dimension::toDimensionValue('0'), point );
			ensure_equals( Dimension::toDimensionValue('1'), curve );
			ensure_equals( Dimension::toDimensionValue('2'), surface );
		}
		catch ( geos::util::IllegalArgumentException const& e ) 
		{
			fail( e.what() ); 
		}
    }

    // Test of toDimensionValue() throwing IllegalArgumentException
    template<>
    template<>
    void object::test<5>()
    {
		using geos::geom::Dimension;

		try 
		{
			Dimension::toDimensionValue('X');
			Dimension::toDimensionValue('9');
		
			fail("IllegalArgumentException expected"); 
		}
		catch ( geos::util::IllegalArgumentException const& e ) 
		{ 
			const char* msg = e.what(); // ok 
			ensure( msg != 0 );
		} 
    }

} // namespace tut

