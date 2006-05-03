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
		const size_t size = 0;
        geos::geom::CoordinateArraySequence sequence;
    
        ensure( sequence.isEmpty() );
        ensure_equals( sequence.getSize(), size );
		ensure_equals( sequence.size(), size );

		ensure_equals( sequence.toString(), std::string("()") );
		
		const size_t dim = 3;
		ensure_equals( sequence.getDimension() , dim);
    }

    // Test of overriden constructor
    template<>
    template<>
    void object::test<2>()
    {
		const size_t size = 3;
        geos::geom::CoordinateArraySequence sequence(size);
    
        ensure( !sequence.isEmpty() );
        ensure_equals( sequence.getSize(), size );
		ensure_equals( sequence.size(), size );

		ensure_not_equals( sequence.toString(), std::string("()") );

		ensure("Every coodinate in the default sequence should be same.", sequence.hasRepeatedPoints() );

		const size_t dim = 3;
		ensure_equals( sequence.getDimension(), dim );
    }

    // Test of overriden constructor taking vector of coordiantes
    template<>
    template<>
    void object::test<3>()
    {
		using geos::geom::Coordinate;
		
		const size_t size = 3;
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
		const size_t sizeEmpty = 0;
        geos::geom::CoordinateArraySequence empty_original;
      
		ensure( empty_original.isEmpty() );
		ensure_equals( empty_original.size(), sizeEmpty );
		ensure_equals( empty_original.toString(), std::string("()") );

		// Create copy of empty sequence
		geos::geom::CoordinateArraySequence empty_copy(empty_original);

		ensure( empty_copy.isEmpty() );
		ensure_equals( empty_copy.size(), sizeEmpty );
		ensure_equals( empty_copy.toString(), std::string("()") );

		// Create non-empty sequence
		const size_t sizeNonEmpty = 2;
		std::vector<Coordinate>* col = new std::vector<Coordinate>();
		col->push_back(Coordinate(1, 2, 3));
		col->push_back(Coordinate(5, 10, 15));
		geos::geom::CoordinateArraySequence non_empty_original(col);
		
		ensure( !non_empty_original.isEmpty() );
		ensure_equals( non_empty_original.size(), sizeNonEmpty );

		// Create copy of non-empty sequence
		geos::geom::CoordinateArraySequence non_empty_copy(non_empty_original);

		ensure( !non_empty_copy.isEmpty() );
		ensure_equals( non_empty_copy.size(), sizeNonEmpty );

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
		
		const size_t size = 2;
		geos::geom::CoordinateArraySequence sequence(col);
		
		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), size );

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
		
		const size_t size = 2;
		geos::geom::CoordinateArraySequence sequence(col);
		
		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), size );
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
		const size_t size = 0;
		geos::geom::CoordinateArraySequence sequence;
		
		ensure( sequence.isEmpty() );
		ensure_equals( sequence.size(), size );

		// Add coordinates
		Coordinate tmp(1, 2, 3);		
		sequence.add(tmp); // insert copy of tmp
		const size_t sizeOne = 1;

		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), sizeOne );

		tmp.x = 5;
		tmp.y = 10;
		tmp.z = 15;
		sequence.add(tmp); // insert copy of tmp
		const size_t sizeTwo = 2;
		
		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), sizeTwo );

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

		// Create sequence with 2 default coordinates
		const size_t size = 2;
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
		const size_t sizeOne = 1;
		geos::geom::CoordinateArraySequence sequence(1);
		
		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), sizeOne );

		// Delete the only coordinate in the sequence
		sequence.deleteAt(0);
		const size_t sizeZero = 0;
		
		ensure( sequence.isEmpty() );
		ensure_equals( sequence.size(), sizeZero );

		// Add new 3 coordinates
		Coordinate first(1, 2, 3);
		sequence.add(first);
		Coordinate second(5, 10, 15);
		sequence.add(second);
		Coordinate third(9, 18, 27);
		sequence.add(third);
		const size_t sizeThree = 3;
				
		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), sizeThree );

		// Delete coordinate in the middle of sequence - the second one.
		sequence.deleteAt(1); // (5, 10, 15)
		const size_t sizeTwo = 2;

		ensure( !sequence.isEmpty() );
		ensure_equals( sequence.size(), sizeTwo );
		
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
		const size_t size = 0;
		geos::geom::CoordinateArraySequence sequence;
		
		ensure( sequence.isEmpty() );
		ensure_equals( sequence.size(), size );

		// Create collection of points
		const std::vector<Coordinate>::size_type sizeCol = 3;
		std::vector<Coordinate> col;
		col.push_back(Coordinate(1, 2, 3));
		col.push_back(Coordinate(5, 10, 15));
		col.push_back(Coordinate(9, 18, 27));

		ensure( "std::vector bug assumed!", !col.empty() );
		ensure_equals( "std::vector bug assumed!", col.size(), sizeCol );

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

	// Test of removeRepeatedPoints
	template<>
	template<>
	void object::test<11>()
	{
		using geos::geom::Coordinate;

		geos::geom::CoordinateArraySequence sequence;

		// Add new 3 equal coordinates
		Coordinate c(1, 2, 3);
		sequence.add(c);
		sequence.add(c);
		sequence.add(c);

		Coordinate c2(5, 10, 15);
		sequence.add(c2);
		sequence.add(c2);

		sequence.add(c);

		ensure_equals( sequence.size(), 6 );
		sequence.removeRepeatedPoints();

		ensure_equals( sequence.size(), 3 );
	}

} // namespace tut
