#include <tut/tut.hpp>
// geos
#include <geos/geom/Coordinate.h>
#include <geos/geom/FixedSizeCoordinateSequence.h>
// std

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_fixedsizecoordinatesequence_data {
};

typedef test_group<test_fixedsizecoordinatesequence_data> group;
typedef group::object object;

group test_fixedsizecoordinatesequence_group("geos::geom::FixedSizeCoordinateSequence");

//
// Test Cases
//

template<>
template<>
void object::test<1>() {
    geos::geom::FixedSizeCoordinateSequence<0> seq;

    ensure(seq.isEmpty());
}

template<>
template<>
void object::test<2>() {
    geos::geom::FixedSizeCoordinateSequence<3> seq;

    ensure_equals(seq.getSize(), 3ul);
}

template<>
template<>
void object::test<3>() {
    geos::geom::FixedSizeCoordinateSequence<3> seq;

    seq.setAt({1, 2}, 0);
    seq.setAt({3, 4}, 1);
    seq.setAt({5, 6}, 2);

    ensure(seq.getAt(0) == geos::geom::Coordinate{1, 2});
}

}
