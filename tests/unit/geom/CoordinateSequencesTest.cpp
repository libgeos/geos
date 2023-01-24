//
// Test Suite for geos::geom::CoordinateSequences class.

#include <tut/tut.hpp>
// geos
#include <geos/geom/CoordinateSequences.h>
// std
#include <string>
#include <vector>
#include <iostream>
#include <cmath>

using geos::DoubleNotANumber;
using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYM;
using geos::geom::CoordinateXYZM;
using geos::geom::CoordinateSequence;
using geos::geom::CoordinateSequences;

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_coordinatesequences_data {
    test_coordinatesequences_data() {}

};

typedef test_group<test_coordinatesequences_data> group;
typedef group::object object;

group test_coordinatesequences_group("geos::geom::CoordinateSequences");

//
// Test Cases
//

struct FunctorWithoutArgs {
    template<typename T1, typename T2>
    void operator()() {
        fail();
    }

};

template<>
void FunctorWithoutArgs::operator()<Coordinate, CoordinateXYZM>() {
    // do not fail
}

// Test dispatch to functor with no arguments
template<>
template<>
void object::test<1>
()
{
    CoordinateSequence xyz = CoordinateSequence::XYZ(0);
    CoordinateSequence xyzm = CoordinateSequence::XYZM(0);

    FunctorWithoutArgs obj;
    CoordinateSequences::binaryDispatch(xyz, xyzm, obj); // Calling the XYZ / XYZM specialization will not throw

    try {
        CoordinateSequences::binaryDispatch(xyzm, xyz, obj); // Calling the default operator() template will thro
        fail();
    } catch (const std::exception &) {
        // do not fail
    }
}

struct FunctorWithTwoArgs {
    template<typename T1, typename T2>
    void operator()(int a, double b) {
        ensure_equals(a, static_cast<int>(b));
    }

};

// Test arguments passed to functor
template<>
template<>
void object::test<2>
()
{
    CoordinateSequence xyz = CoordinateSequence::XYZ(0);

    FunctorWithTwoArgs obj;
    CoordinateSequences::binaryDispatch(xyz, xyz, obj, 7, 7.0);
}


} // namespace tut
