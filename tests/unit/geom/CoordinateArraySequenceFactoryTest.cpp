// $Id$
// 
// Test Suite for geos::geom::CoordinateArraySequence class.

// TUT
#include <tut.h>
// GEOS
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
// STL
#include <exception>
#include <vector>

namespace tut
{
	//
	// Test Group
	//

	// Common data used by tests
	struct test_coordinatearraysequencefactory_data
	{
		typedef geos::geom::CoordinateSequence* CoordianteSequencePtr;
		typedef geos::geom::CoordinateSequenceFactory const* CoordianteFactoryCPtr;
		typedef geos::geom::CoordinateArraySequenceFactory const* CoordianteArrayFactoryCPtr;

		test_coordinatearraysequencefactory_data() {}
	};

	typedef test_group<test_coordinatearraysequencefactory_data> group;
	typedef group::object object;

	group test_coordinatearraysequencefactory_group("geos::geom::CoordinateArraySequenceFactory");

	//
	// Test Cases
	//

	// Test of instance()
	template<>
	template<>
	void object::test<1>()
	{
		try
		{
			CoordianteFactoryCPtr base = geos::geom::CoordinateArraySequenceFactory::instance();
			ensure( 0 != base );

			ensure(typeid(base).name(), typeid(base) == typeid(CoordianteFactoryCPtr) );
			ensure(typeid(base).name(), typeid(base) != typeid(CoordianteArrayFactoryCPtr) );

			CoordianteArrayFactoryCPtr derived;
			derived = dynamic_cast<CoordianteArrayFactoryCPtr>(base);
			ensure( 0 != derived );
			ensure(typeid(derived).name(), typeid(derived) == typeid(CoordianteArrayFactoryCPtr) );
		}
		catch (std::exception& e)
		{
			fail( e.what() );
		}
	}


	// Test of first version of create() with empty sequence
	template<>
	template<>
	void object::test<2>()
	{
		using geos::geom::Coordinate;

		try
		{
			CoordianteFactoryCPtr factory = geos::geom::CoordinateArraySequenceFactory::instance();

			ensure( 0 != factory );

			std::vector<Coordinate>* col = new std::vector<Coordinate>();

			ensure( 0 != col);

			const size_t size0 = 0;
			CoordianteSequencePtr sequence = factory->create(col);

			ensure( 0 != sequence);
			ensure( sequence->isEmpty() );
			ensure_equals( sequence->size(), size0 );

			// FREE MEMORY
			delete sequence;
		}
		catch (std::exception& e)
		{
			fail( e.what() );
		}
	}

	// Test of first version of create() with non-empty sequence
	template<>
	template<>
	void object::test<3>()
	{
		using geos::geom::Coordinate;

		try
		{
			CoordianteFactoryCPtr factory = geos::geom::CoordinateArraySequenceFactory::instance();
			ensure( 0 != factory );

			std::vector<Coordinate>* col = new std::vector<Coordinate>();

			ensure( 0 != col);

			col->push_back(Coordinate(1, 2, 3));
			col->push_back(Coordinate(5, 10, 15));

			const size_t size2 = 2;
			CoordianteSequencePtr sequence = factory->create(col);

			ensure( 0 != sequence);
			ensure( !sequence->isEmpty() );
			ensure_equals( sequence->size(), size2 );
			ensure_not_equals( sequence->getAt(0), sequence->getAt(1) );

			// FREE MEMORY
			delete sequence;
		}
		catch (std::exception& e)
		{
			fail( e.what() );
		}
	}

	// Test of second version of create()
	template<>
	template<>
	void object::test<4>()
	{
		using geos::geom::Coordinate;

		try
		{
			CoordianteFactoryCPtr factory = geos::geom::CoordinateArraySequenceFactory::instance();

			ensure( 0 != factory );

			const size_t size1000 = 1000;
			CoordianteSequencePtr sequence = factory->create(size1000, 3);

			ensure( 0 != sequence);
			ensure( !sequence->isEmpty() );
			ensure_equals( sequence->size(), size1000 );
			ensure( sequence->hasRepeatedPoints() );
			ensure_equals( sequence->getAt(0), sequence->getAt(size1000 - 1) );
			ensure_equals( sequence->getAt(0), sequence->getAt(size1000 / 2) );
			
			// FREE MEMORY
			delete sequence;
		}
		catch (std::exception& e)
		{
			fail( e.what() );
		}
	}


} // namespace tut
