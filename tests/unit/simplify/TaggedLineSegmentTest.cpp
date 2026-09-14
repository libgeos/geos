//
// Test Suite for geos::simplify::TaggedLineSegment class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/simplify/TaggedLineSegment.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Geometry.h>

namespace tut {

struct test_taggedlinesegment_data {
};

typedef test_group<test_taggedlinesegment_data> group;
typedef group::object object;

group test_taggedlinesegment_group("geos::simplify::TaggedLineSegment");

// operator= (GH-1528 follow-up): TaggedLineSegment's copy constructor is
// exactly memberwise (base LineSegment plus a raw non-owning parent
// pointer and index), so operator= is defaulted; verify it actually
// behaves that way.
template<>
template<>
void object::test<1>()
{
    using geos::geom::Coordinate;
    using geos::simplify::TaggedLineSegment;

    Coordinate a0(0, 0);
    Coordinate a1(1, 1);
    Coordinate b0(5, 5);
    Coordinate b1(6, 7);

    const geos::geom::Geometry* parentB = reinterpret_cast<const geos::geom::Geometry*>(0x1234);

    TaggedLineSegment a(a0, a1);
    TaggedLineSegment b(b0, b1, parentB, 42);

    a = b;

    ensure_equals(a.p0, b0);
    ensure_equals(a.p1, b1);
    ensure(a.getParent() == parentB);
    ensure_equals(a.getIndex(), std::size_t(42));

    // Self-assignment must not corrupt the object.
    TaggedLineSegment& aref = a;
    a = aref;

    ensure_equals(a.p0, b0);
    ensure_equals(a.p1, b1);
    ensure(a.getParent() == parentB);
    ensure_equals(a.getIndex(), std::size_t(42));
}

} // namespace tut
