// $Id$
// 
// Test Suite for geos::geom::CoordinateArraySequence class.

// TUT
#include <tut.h>
// GEOS
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
// STL
#include <string>
#include <vector>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_coordinatearraysequence_data
    {

        test_coordinatearraysequence_data() {}
    };

    typedef test_group<test_coordinatearraysequence_data> group;
    typedef group::object object;

    group test_coordinatearraysequence_group("geos::geom::CoordinateArraySequence");

    //
    // Test Cases
    //

    // Test of default constructor
    template<>
    template<>
    void object::test<1>()
    {
        geos::geom::CoordinateArraySequence sequence;
    
        ensure( sequence.isEmpty() );
        ensure_equals( sequence.getSize(), 0 );
		ensure_equals( sequence.size(), 0 );

		ensure_equals( sequence.toString(), std::string("()") );

		ensure_equals( sequence.getDimension(), 3);
    }

    // Test of overriden constructor
    template<>
    template<>
    void object::test<2>()
    {
		const int size = 3;
        geos::geom::CoordinateArraySequence sequence(size);
    
        ensure( !sequence.isEmpty() );
        ensure_equals( sequence.getSize(), size );
		ensure_equals( sequence.size(), size );

		ensure_not_equals( sequence.toString(), std::string("()") );

		ensure("Every coodinate in the default sequence should be same.", sequence.hasRepeatedPoints() );

		ensure_equals( sequence.getDimension(), 3);
    }

    // Test of overriden constructor taking vector of coordiantes
    template<>
    template<>
    void object::test<3>()
    {
		using geos::geom::Coordinate;
		
		const int size = 3;
		const double a = 0;
		const double b = 5.0;
		const double c = 10.0;

		std::vector<Coordinate>* col = new std::vector<Coordinate>();
		col->push_back(Coordinate(a, a, a));
		col->push_back(Coordinate(b, b, b));
		col->push_back(Coordinate(c, c, c));

		// sequence takes ownership of the col
		geos::geom::CoordinateArraySequence sequence(col);

		ensure( !sequence.isEmpty() );
        ensure_equals( sequence.getSize(), size );
		ensure_equals( sequence.size(), size );

		ensure_not_equals( sequence.toString(), std::string("()") );

		ensure("Coordinate sequence should contain unique coordinates.", !sequence.hasRepeatedPoints() );
	}

    // Test of copy constructor
    template<>
    template<>
    void object::test<4>()
    {
		using geos::geom::Coordinate;

		// Create empty sequence
        geos::geom::CoordinateArraySequence empty_original;
      
		ensure( empty_original.isEmpty() );
		ensure_equals( empty_original.size(), 0 );
		ensure_equals( empty_original.toString(), std::string("()") );

		// Create copy of empty sequence
		geos::geom::CoordinateArraySequence empty_copy(empty_original);

		ensure( empty_copy.isEmpty() );
		ensure_equals( empty_copy.size(), 0 );
		ensure_equals( empty_copy.toString(), std::string("()") );

		// Create non-empty sequence
		std::vector<Coordinate>* col = new std::vector<Coordinate>();
		col->push_back(Coordinate(1, 2, 3));
		col->push_back(Coordinate(5, 10, 15));
		geos::geom::CoordinateArraySequence non_empty_original(col);
		
		ensure( !non_empty_original.isEmpty() );
		ensure_equals( non_empty_original.size(), 2 );

		// Create copy of non-empty sequence
		geos::geom::CoordinateArraySequence non_empty_copy(non_empty_original);

		ensure( !non_empty_copy.isEmpty() );
		ensure_equals( non_empty_copy.size(), 2 );

		// Compare non-empty original and copy using equality operators
		ensure_equals( non_empty_original.getAt(0), non_empty_copy.getAt(0) );
		ensure_equals( non_empty_original.getAt(1), non_empty_copy.getAt(1) );
		ensure_not_equals( non_empty_original.getAt(0), non_empty_copy.getAt(1) );	
	}

    // Test of getX() and getY()
    template<>
    template<>
    void object::test<5>()
    {
		using geos::geom::Coordinate;

		// Create non-empty sequence
		std::vector<Coordinate>* col = new std::vector<Coordinate>();
		col->push_back(Coordinate(1, 2));
		col->push_back(Coordinate(5, 10));
		geos::geom::CoordinateArraySequence sequence(col);
		
		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), 2 );

		ensure_equals( sequence.getX(0), 1 );
		ensure_equals( sequence.getY(0), 2 );
		ensure_equals( sequence.getX(1), 5 );
		ensure_equals( sequence.getY(1), 10 );
	}

    // Test of getAt()
    template<>
    template<>
    void object::test<6>()
    {
		using geos::geom::Coordinate;

		// Create non-empty sequence
		std::vector<Coordinate>* col = new std::vector<Coordinate>();
		col->push_back(Coordinate(1, 2, 3));
		col->push_back(Coordinate(5, 10, 15));
		geos::geom::CoordinateArraySequence sequence(col);
		
		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), 2 );

		ensure_not_equals( sequence.getAt(0), sequence.getAt(1) );

		// First version of getAt()
		ensure_equals( sequence.getAt(0).x, 1 );
		ensure_equals( sequence.getAt(0).y, 2 );
		ensure_equals( sequence.getAt(0).z, 3 );
		ensure_equals( sequence.getAt(1).x, 5 );
		ensure_equals( sequence.getAt(1).y, 10 );
		ensure_equals( sequence.getAt(1).z, 15 );

		// Second version of getAt()
		Coordinate buf;

		sequence.getAt(0, buf);
		ensure_equals( buf.x, 1 );
		ensure_equals( buf.y, 2 );
		ensure_equals( buf.z, 3 );
		
		sequence.getAt(1, buf);
		ensure_equals( buf.x, 5 );
		ensure_equals( buf.y, 10 );
		ensure_equals( buf.z, 15 );
	}

    // Test of add()
    template<>
    template<>
    void object::test<7>()
    {
		using geos::geom::Coordinate;

		// Create empty sequence to fill with coordinates
		geos::geom::CoordinateArraySequence sequence;
		
		ensure( sequence.isEmpty() );
		ensure_equals( sequence.size(), 0 );

		// Add coordinates
		Coordinate tmp(1, 2, 3);		
		sequence.add(tmp); // insert copy of tmp

		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), 1 );

		tmp.x = 5;
		tmp.y = 10;
		tmp.z = 15;
		sequence.add(tmp); // insert copy of tmp
		ensure_equals( sequence.size(), 2 );

		ensure( !sequence.hasRepeatedPoints() );

		// Check elements of sequence
		ensure_not_equals( sequence.getAt(0), sequence.getAt(1) );
		
		ensure_equals( sequence.getAt(0).x, 1 );
		ensure_equals( sequence.getAt(0).y, 2 );
		ensure_equals( sequence.getAt(0).z, 3 );
		ensure_equals( sequence.getAt(1).x, 5 );
		ensure_equals( sequence.getAt(1).y, 10 );
		ensure_equals( sequence.getAt(1).z, 15 );
	}

    // Test of setAt()
    template<>
    template<>
    void object::test<8>()
    {
		using geos::geom::Coordinate;

		const int size = 2;
		// Create sequence with 2 default coordinates
		geos::geom::CoordinateArraySequence sequence(size);
		
		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), size );
		ensure( sequence.hasRepeatedPoints() );

		// Set new values to first coordinate
		Coordinate first(1, 2, 3);
		sequence.setAt(first, 0);

		ensure_equals( sequence.size(), size );
		ensure_equals( sequence.getAt(0).x, 1 );
		ensure_equals( sequence.getAt(0).y, 2 );
		ensure_equals( sequence.getAt(0).z, 3 );


		// Set new values to second coordinate 
		Coordinate second(5, 10, 15);
		sequence.setAt(second, 1);

		ensure_equals( sequence.size(), size );

		ensure_equals( sequence.getAt(1).x, 5 );
		ensure_equals( sequence.getAt(1).y, 10 );
		ensure_equals( sequence.getAt(1).z, 15 );

		ensure( !sequence.hasRepeatedPoints() );
	}

    // Test of deleteAt()
    template<>
    template<>
    void object::test<9>()
    {
		using geos::geom::Coordinate;

		// Create sequence with only 1 default coordinate
		geos::geom::CoordinateArraySequence sequence(1);
		
		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), 1 );

		// Delete the only coordinate in the sequence
		sequence.deleteAt(0);

		ensure( sequence.isEmpty() );
		ensure_equals( sequence.size(), 0 );

		// Add new 3 coordinates
		Coordinate first(1, 2, 3);
		sequence.add(first);
		Coordinate second(5, 10, 15);
		sequence.add(second);
		Coordinate third(9, 18, 27);
		sequence.add(third);
		
		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), 3 );

		// Delete coordinate in the middle of sequence - the second one.
		sequence.deleteAt(1); // (5, 10, 15)

		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), 2 );
		
		ensure_not_equals( sequence.getAt(0), sequence.getAt(1) );
		ensure_equals( sequence.getAt(0), first );
		ensure_not_equals( "deleteAt() did not remove coordinate.", sequence.getAt(1), second );
		ensure_equals( sequence.getAt(1), third );
	}

	// Test of setPoints()
    template<>
    template<>
    void object::test<10>()
    {
		using geos::geom::Coordinate;

		// Create empty sequence
		geos::geom::CoordinateArraySequence sequence;
		
		ensure( sequence.isEmpty() );
		ensure_equals( sequence.size(), 0 );

		// Create collection of points
		std::vector<Coordinate> col;
		col.push_back(Coordinate(1, 2, 3));
		col.push_back(Coordinate(5, 10, 15));
		col.push_back(Coordinate(9, 18, 27));

		ensure( "std::vector bug assumed!", !col.empty() );
		ensure_equals( "std::vector bug assumed!", col.size(), 3 );

		// Use setPoints()
		sequence.setPoints(col);

		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), col.size() );
		ensure( !sequence.hasRepeatedPoints() );

		// Check inserted points
		ensure_equals( sequence.getAt(0).x, 1 );
		ensure_equals( sequence.getAt(0).y, 2 );
		ensure_equals( sequence.getAt(0).z, 3 );

		ensure_equals( sequence.getAt(1).x, 5 );
		ensure_equals( sequence.getAt(1).y, 10 );
		ensure_equals( sequence.getAt(1).z, 15 );

		ensure_equals( sequence.getAt(2).x, 9 );
		ensure_equals( sequence.getAt(2).y, 18 );
		ensure_equals( sequence.getAt(2).z, 27 );
	}

} // namespace tut
