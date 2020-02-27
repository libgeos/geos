//
// Test Suite for geos::geom::CoordinateArraySequence class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
// std
#include <exception>
#include <vector>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_coordinatearraysequencefactory_data {
    typedef geos::geom::CoordinateSequence* CoordinateSequencePtr;
    typedef geos::geom::CoordinateSequenceFactory const* CoordinateFactoryCPtr;
    typedef geos::geom::CoordinateArraySequenceFactory const* CoordinateArrayFactoryCPtr;

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
void object::test<1>
()
{
    try {
        CoordinateFactoryCPtr base = geos::geom::CoordinateArraySequenceFactory::instance();
        ensure(nullptr != base);

        ensure(typeid(base).name(), typeid(base) == typeid(CoordinateFactoryCPtr));
        ensure(typeid(base).name(), typeid(base) != typeid(CoordinateArrayFactoryCPtr));

        CoordinateArrayFactoryCPtr derived;
        derived = dynamic_cast<CoordinateArrayFactoryCPtr>(base);
        ensure(nullptr != derived);
        ensure(typeid(derived).name(), typeid(derived) == typeid(CoordinateArrayFactoryCPtr));
    }
    catch(std::exception& e) {
        fail( e.what() );
    }
}


// Test of first version of create() with empty sequence
template<>
template<>
void object::test<2>
()
{
    using geos::geom::Coordinate;

    try {
        CoordinateFactoryCPtr factory = geos::geom::CoordinateArraySequenceFactory::instance();

        ensure(nullptr != factory);

        std::vector<Coordinate>* col = new std::vector<Coordinate>();

        ensure(nullptr != col);

        const size_t size0 = 0;
        auto sequence = factory->create(col);

        ensure(nullptr != sequence);
        ensure(sequence->isEmpty());
        ensure_equals(sequence->size(), size0);
    }
    catch(std::exception& e) {
        fail(e.what());
    }
}

// Test of first version of create() with non-empty sequence
template<>
template<>
void object::test<3>
()
{
    using geos::geom::Coordinate;

    try {
        CoordinateFactoryCPtr factory = geos::geom::CoordinateArraySequenceFactory::instance();
        ensure(nullptr != factory);

        std::vector<Coordinate>* col = new std::vector<Coordinate>();

        ensure(nullptr != col);

        col->push_back(Coordinate(1, 2, 3));
        col->push_back(Coordinate(5, 10, 15));

        const size_t size2 = 2;
        auto sequence = factory->create(col);

        ensure(nullptr != sequence);
        ensure(!sequence->isEmpty());
        ensure_equals(sequence->size(), size2);
        ensure(sequence->getAt(0) != sequence->getAt(1));
    }
    catch(std::exception& e) {
        fail(e.what());
    }
}

// Test of second version of create()
template<>
template<>
void object::test<4>
()
{
    using geos::geom::Coordinate;

    try {
        CoordinateFactoryCPtr factory = geos::geom::CoordinateArraySequenceFactory::instance();

        ensure(nullptr != factory);

        const size_t size1000 = 1000;
        auto sequence = factory->create(size1000, 3);

        ensure(nullptr != sequence);
        ensure(!sequence->isEmpty());
        ensure_equals(sequence->size(), size1000);
        ensure(sequence->hasRepeatedPoints());
        ensure_equals(sequence->getAt(0), sequence->getAt(size1000 - 1));
        ensure_equals(sequence->getAt(0), sequence->getAt(size1000 / 2));
    }
    catch(std::exception& e) {
        fail(e.what());
    }
}

// Test of create() without arguments
template<>
template<>
void object::test<5>
()
{
    using geos::geom::Coordinate;

    try {
        CoordinateFactoryCPtr factory = geos::geom::CoordinateArraySequenceFactory::instance();

        ensure(nullptr != factory);

        const size_t size0 = 0;
        auto sequence = factory->create();

        ensure(nullptr != sequence);
        ensure(sequence->isEmpty());
        ensure_equals(sequence->size(), size0);
    }
    catch(std::exception& e) {
        fail(e.what());
    }
}


} // namespace tut
