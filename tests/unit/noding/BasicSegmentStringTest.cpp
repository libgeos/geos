//
// Test Suite for geos::noding::BasicSegmentString class.

#include <tut/tut.hpp>
// geos
#include <geos/noding/BasicSegmentString.h>
#include <geos/noding/Octant.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <geos/util.h>
// std
#include <memory>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_basicsegmentstring_data {

    typedef std::unique_ptr<geos::geom::CoordinateSequence> \
    CoordinateSequenceAutoPtr;

    typedef std::unique_ptr<geos::noding::BasicSegmentString> \
    SegmentStringAutoPtr;

    const geos::geom::CoordinateSequenceFactory* csFactory;

    SegmentStringAutoPtr
    makeSegmentString(geos::geom::CoordinateSequence* cs, void* d = nullptr)
    {
        return SegmentStringAutoPtr(
                   new geos::noding::BasicSegmentString(cs, d)
               );
    }

    test_basicsegmentstring_data()
        :
        csFactory(geos::geom::CoordinateArraySequenceFactory::instance())
    {
    }

    ~test_basicsegmentstring_data()
    {
    }

};

typedef test_group<test_basicsegmentstring_data> group;
typedef group::object object;

group test_basicsegmentstring_group("geos::noding::BasicSegmentString");

//
// Test Cases
//

// test constructor with 2 equal points
template<>
template<>
void object::test<1>
()
{
    auto cs = geos::detail::make_unique<geos::geom::CoordinateArraySequence>(0, 2);

    ensure(nullptr != cs.get());

    geos::geom::Coordinate c0(0, 0);
    geos::geom::Coordinate c1(0, 0);

    cs->add(c0);
    cs->add(c1);

    ensure_equals(cs->size(), 2u);

    SegmentStringAutoPtr ss(makeSegmentString(cs.get()));
    ensure(nullptr != ss.get());

    ensure_equals(ss->size(), 2u);

    ensure_equals(ss->getData(), (void*)nullptr);

    ensure_equals(ss->getCoordinates(), cs.get());

    ensure_equals(ss->getCoordinate(0), c0);

    ensure_equals(ss->getCoordinate(1), c1);

    ensure_equals(ss->isClosed(), true);

    // this would throw an exception
    bool octant_failed = false;
    try {
        ss->getSegmentOctant(0);
    }
    catch(...) {
        octant_failed = true;
    }
    ensure(octant_failed);

}

// test constructor with 2 different points
template<>
template<>
void object::test<2>
()
{
    auto cs = geos::detail::make_unique<geos::geom::CoordinateArraySequence>(0, 2);

    ensure(nullptr != cs.get());

    geos::geom::Coordinate c0(0, 0);
    geos::geom::Coordinate c1(1, 0);

    cs->add(c0);
    cs->add(c1);

    ensure_equals(cs->size(), 2u);

    SegmentStringAutoPtr ss(makeSegmentString(cs.get()));
    ensure(nullptr != ss.get());

    ensure_equals(ss->size(), 2u);

    ensure_equals(ss->getData(), (void*)nullptr);

    ensure_equals(ss->getCoordinates(), cs.get());

    ensure_equals(ss->getCoordinate(0), c0);

    ensure_equals(ss->getCoordinate(1), c1);

    ensure_equals(ss->isClosed(), false);

    ensure_equals(ss->getSegmentOctant(0), 0);

}

// test constructor with 4 different points forming a ring
template<>
template<>
void object::test<3>
()
{
    auto cs = geos::detail::make_unique<geos::geom::CoordinateArraySequence>(0, 2);

    ensure(nullptr != cs.get());

    geos::geom::Coordinate c0(0, 0);
    geos::geom::Coordinate c1(1, 0);
    geos::geom::Coordinate c2(1, 1);

    cs->add(c0);
    cs->add(c1);
    cs->add(c2);
    cs->add(c0);

    ensure_equals(cs->size(), 4u);

    SegmentStringAutoPtr ss(makeSegmentString(cs.get()));
    ensure(nullptr != ss.get());

    ensure_equals(ss->size(), 4u);

    ensure_equals(ss->getData(), (void*)nullptr);

    ensure_equals(ss->getCoordinates(), cs.get());

    ensure_equals(ss->getCoordinate(0), c0);

    ensure_equals(ss->getCoordinate(1), c1);

    ensure_equals(ss->getCoordinate(2), c2);

    ensure_equals(ss->getCoordinate(3), c0);

    ensure_equals(ss->isClosed(), true);

    ensure_equals(ss->getSegmentOctant(2), 4);

    ensure_equals(ss->getSegmentOctant(1), 1);

    ensure_equals(ss->getSegmentOctant(0), 0);

}

// test Octant class
template<>
template<>
void object::test<4>
()
{
    geos::geom::Coordinate p0(0, 0);
    geos::geom::Coordinate p1(5, -5);
    int octant_rc1 = 0;
    int octant_rc2 = 0;
    int testPassed = true;
    try {
        octant_rc1 = geos::noding::Octant::octant(p0, p1);
        octant_rc2 = geos::noding::Octant::octant(&p0, &p1);
        testPassed = (octant_rc1 == octant_rc2);
    }
    catch(...) {
        testPassed = false;
    }
    ensure(0 != testPassed);
}


} // namespace tut

