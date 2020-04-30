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

using geos::geom::FixedSizeCoordinateSequence;

group test_fixedsizecoordinatesequence_group("geos::geom::FixedSizeCoordinateSequence");

//
// Test Cases
//

// Empty sequence is empty
template<>
template<>
void object::test<1>() {
    FixedSizeCoordinateSequence<0> seq;

    ensure(seq.isEmpty());

}

// test getSize
template<>
template<>
void object::test<2>() {
    FixedSizeCoordinateSequence<3> seq;

    ensure_equals(seq.getSize(), 3ul);
}

//  test setAt, getAt
template<>
template<>
void object::test<3>() {
    FixedSizeCoordinateSequence<3> seq;

    seq.setAt({1, 2}, 0);
    seq.setAt({3, 4}, 1);
    seq.setAt({5, 6}, 2);

    ensure(seq.getAt(0) == geos::geom::Coordinate{1, 2});

    geos::geom::Coordinate c;
    seq.getAt(2, c);
    ensure(c == geos::geom::Coordinate{5, 6});
}

// test setOrdinate
template<>
template<>
void object::test<4>() {
    FixedSizeCoordinateSequence<2> seq;

    seq.setOrdinate(0, geos::geom::CoordinateSequence::X, 2.2);
    seq.setOrdinate(0, geos::geom::CoordinateSequence::Y, 3.3);
    seq.setOrdinate(0, geos::geom::CoordinateSequence::Z, 4.4);

    try {
        seq.setOrdinate(0, 17, 5.5);
        fail();
    } catch (geos::util::IllegalArgumentException &) {}

    ensure(seq[0].equals3D(geos::geom::Coordinate{2.2, 3.3, 4.4}));
}

// test getDimension
template<>
template<>
void object::test<5>() {
    // empty sequence is always XYZ
    FixedSizeCoordinateSequence<0> seq0;
    ensure_equals(seq0.getDimension(), 3ul);

    // sequence dimension is set by first coordinate
    FixedSizeCoordinateSequence<1> seq1_2d;
    seq1_2d.setAt({1, 2}, 0);
    ensure_equals(seq1_2d.getDimension(), 2ul);

    FixedSizeCoordinateSequence<1> seq1_3d;
    seq1_3d.setAt({1, 2, 3}, 0);
    ensure_equals(seq1_3d.getDimension(), 3ul);

    // sequence dimension doesn't change even if coordinate dimension does
    seq1_2d.setAt({1, 2, 3}, 0);
    ensure_equals(seq1_2d.getDimension(), 2ul);

    // sequence dimension doesn't change even if coordinate dimension does
    seq1_3d.setAt({1, 2}, 0);
    ensure_equals(seq1_3d.getDimension(), 3ul);

    // unless it's changed by apply_rw. weird!

    struct ZSetter : public geos::geom::CoordinateFilter {
        ZSetter(double val) : m_val(val) {}

        void filter_rw(geos::geom::Coordinate* c) const override {
            c->z = m_val;
        }

        double m_val;
    };

    ZSetter setNaN(geos::DoubleNotANumber);
    seq1_3d.apply_rw(&setNaN);
    ensure_equals(seq1_3d.getDimension(), 2ul);

    ZSetter setZero(0);
    seq1_3d.apply_rw(&setZero);
    ensure_equals(seq1_3d.getDimension(), 3ul);
}

// test clone
template<>
template<>
void object::test<6>
()
{
    FixedSizeCoordinateSequence<1> a(2);
    a.setAt({ 1, 2, 3 }, 0);

    ensure_equals(a.getDimension(), 2u);

    auto b = a.clone();
    ensure_equals(b->getDimension(), 2u);
    ensure(a.getAt(0).equals3D(b->getAt(0)));
}

}
