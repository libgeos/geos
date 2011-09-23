// 
// Test Suite for geos::noding::NodedSegmentString class.

#include <tut.hpp>
// geos
#include <geos/noding/NodedSegmentString.h>
#include <geos/noding/Octant.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
// std
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_nodedsegmentstring_data
    {

        typedef std::auto_ptr<geos::geom::CoordinateSequence> \
        CoordinateSequenceAutoPtr;

        typedef std::auto_ptr<geos::noding::NodedSegmentString> \
        SegmentStringAutoPtr;
    
    const geos::geom::CoordinateSequenceFactory* csFactory;

    SegmentStringAutoPtr
    makeSegmentString(geos::geom::CoordinateSequence* cs, void *d = 0)
    {
        return SegmentStringAutoPtr(
            new geos::noding::NodedSegmentString(cs, d)
            );
    }

    test_nodedsegmentstring_data()
        :
        csFactory(geos::geom::CoordinateArraySequenceFactory::instance())
    {
    }

    ~test_nodedsegmentstring_data()
    {
    }

    };

    typedef test_group<test_nodedsegmentstring_data> group;
    typedef group::object object;

    group test_nodedsegmentstring_group("geos::noding::NodedSegmentString");

    //
    // Test Cases
    //

    // test constructor with 2 equal points
    template<>
    template<>
    void object::test<1>()
    {
        CoordinateSequenceAutoPtr cs(csFactory->create((size_t)0, 2));

        ensure(0 != cs.get());

        geos::geom::Coordinate c0(0, 0);
        geos::geom::Coordinate c1(0, 0);

        cs->add(c0);
        cs->add(c1);

        ensure_equals(cs->size(), 2u);

        SegmentStringAutoPtr ss(makeSegmentString(cs.release()));
        ensure(0 != ss.get());

        ensure_equals(ss->size(), 2u);

        ensure_equals(ss->getData(), (void*)0);

        ensure_equals(ss->getCoordinate(0), c0);

        ensure_equals(ss->getCoordinate(1), c1);

        ensure_equals(ss->isClosed(), true);

        ensure_equals(ss->getNodeList().size(), 0u);

        ensure_equals(ss->getSegmentOctant(0), 0);
    }

    // test constructor with 2 different points
    template<>
    template<>
    void object::test<2>()
    {
        CoordinateSequenceAutoPtr cs(csFactory->create((size_t)0, 2));

    ensure(0 != cs.get());

    geos::geom::Coordinate c0(0, 0);
    geos::geom::Coordinate c1(1, 0);

    cs->add(c0);
    cs->add(c1);

    ensure_equals(cs->size(), 2u);

    SegmentStringAutoPtr ss(makeSegmentString(cs.release()));
    ensure(0 != ss.get());

    ensure_equals(ss->size(), 2u);

    ensure_equals(ss->getData(), (void*)0);

    ensure_equals(ss->getCoordinate(0), c0);

    ensure_equals(ss->getCoordinate(1), c1);

    ensure_equals(ss->isClosed(), false);

    ensure_equals(ss->getSegmentOctant(0), 0);

    ensure_equals(ss->getNodeList().size(), 0u);
    }

    // test constructor with 4 different points forming a ring
    template<>
    template<>
    void object::test<3>()
    {
        CoordinateSequenceAutoPtr cs(csFactory->create((size_t)0, 2));

    ensure(0 != cs.get());

    geos::geom::Coordinate c0(0, 0);
    geos::geom::Coordinate c1(1, 0);
    geos::geom::Coordinate c2(1, 1);

    cs->add(c0);
    cs->add(c1);
    cs->add(c2);
    cs->add(c0);

    ensure_equals(cs->size(), 4u);

    SegmentStringAutoPtr ss(makeSegmentString(cs.release()));
    ensure(0 != ss.get());

    ensure_equals(ss->size(), 4u);

    ensure_equals(ss->getData(), (void*)0);

    ensure_equals(ss->getCoordinate(0), c0);

    ensure_equals(ss->getCoordinate(1), c1);

    ensure_equals(ss->getCoordinate(2), c2);

    ensure_equals(ss->getCoordinate(3), c0);

    ensure_equals(ss->isClosed(), true);

    ensure_equals(ss->getSegmentOctant(2), 4);

    ensure_equals(ss->getSegmentOctant(1), 1);

    ensure_equals(ss->getSegmentOctant(0), 0);

    ensure_equals(ss->getNodeList().size(), 0u);
    }

    // test Octant class
    template<>
    template<>
    void object::test<4>()
    {
        geos::geom::Coordinate p0(0, 0);
        geos::geom::Coordinate p1(5,-5);
        int octant_rc1 = 0;
        int octant_rc2 = 0;
        int testPassed = true;
        try {
            octant_rc1 = geos::noding::Octant::octant(p0,p1);
            octant_rc2 = geos::noding::Octant::octant(&p0,&p1);
            testPassed = (octant_rc1 == octant_rc2);
        }
        catch (...) {
            testPassed = false;
        }
        ensure( 0 != testPassed);
    }

    // test adding intersections
    template<>
    template<>
    void object::test<5>()
    {
        geos::geom::Coordinate p0(0, 0);
        geos::geom::Coordinate p1(10, 0);


        CoordinateSequenceAutoPtr cs(csFactory->create((size_t)0, 2));
        cs->add(p0);
        cs->add(p1);

        SegmentStringAutoPtr ss(makeSegmentString(cs.release()));

        ensure_equals(ss->getNodeList().size(), 0u);

        // the intersection is invalid, but SegmentString trusts us
        ss->addIntersection(p0, 0);
        ensure_equals(ss->getNodeList().size(), 1u);

        // This node is already present, so shouldn't be
        // accepted as a new one
        ss->addIntersection(p0, 0);
        ensure_equals(ss->getNodeList().size(), 1u);

        ss->addIntersection(p1, 0);
        ensure_equals(ss->getNodeList().size(), 2u);

        ss->addIntersection(p1, 0);
        ensure_equals(ss->getNodeList().size(), 2u);

        ss->addIntersection(p0, 0);
        ensure_equals(ss->getNodeList().size(), 2u);

    }

    // TODO: test getting noded substrings
//  template<>
//  template<>
//  void object::test<6>()
//  {
//      geos::geom::Coordinate cs1p0(0, 0);
//      geos::geom::Coordinate cs1p1(10, 0);
//      CoordinateSequenceAutoPtr cs1(csFactory->create(0, 2));
//      cs1->add(cs1p0);
//      cs1->add(cs1p1);
//
//      geos::geom::Coordinate cs2p0(5, -5);
//      geos::geom::Coordinate cs2p1(5, 5);
//      CoordinateSequenceAutoPtr cs2(csFactory->create(0, 2));
//      cs2->add(cs2p0);
//      cs2->add(cs2p1);
//
//      using geos::noding::SegmentString;
//      using geos::noding::NodedSegmentString;
//
//      SegmentString::NonConstVect inputStrings;
//      inputStrings.push_back(makeSegmentString(cs2.get()).get());
//
//      std::auto_ptr<SegmentString::NonConstVect> nodedStrings(
//          NodedSegmentString::getNodedSubstrings(inputStrings)
//      );
//
//      ensure_equals(nodedStrings->size(), 0u);
//  }



} // namespace tut

