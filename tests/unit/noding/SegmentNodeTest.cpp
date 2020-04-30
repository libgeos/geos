//
// Test Suite for geos::noding::SegmentNode class.

#include <tut/tut.hpp>
// geos
#include <geos/noding/SegmentNode.h>
#include <geos/noding/NodedSegmentString.h>
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

// Common data used by all tests
struct test_segmentnode_data {

    typedef std::unique_ptr<geos::geom::CoordinateSequence>
    CoordSeqPtr;

    typedef std::unique_ptr<geos::noding::SegmentString>
    SegmentStringPtr;

    const geos::geom::CoordinateSequenceFactory* factory_;

    test_segmentnode_data()
        : factory_(geos::geom::CoordinateArraySequenceFactory::instance())
    {}
};

typedef test_group<test_segmentnode_data> group;
typedef group::object object;

group test_segmentnode_group("geos::noding::SegmentNode");

//
// Test Cases
//

// Test of overriden constructor
template<>
template<>
void object::test<1>
()
{
    using geos::geom::Coordinate;
    using geos::noding::NodedSegmentString;
    using geos::noding::SegmentNode;

    // Create coordinates sequence
    const size_t coords_size = 2;
    auto cs = geos::detail::make_unique<geos::geom::CoordinateArraySequence>(0u, coords_size);

    ensure(nullptr != cs.get());

    Coordinate c0(0, 0);
    Coordinate c1(3, 3);
    cs->add(c0);
    cs->add(c1);

    ensure_equals(cs->size(), coords_size);

    // Create SegmentString instance

    NodedSegmentString segment(cs.release(), nullptr);

    ensure_equals(segment.size(), coords_size);

    // Construct a node on the given NodedSegmentString
    {
        const size_t segment_index = 0;
        SegmentNode node(segment, Coordinate(3, 3), segment_index,
                         segment.getSegmentOctant(segment_index));

        ensure_equals(node.segmentIndex, segment_index);

        // only first endpoint is considered interior
        ensure(node.isInterior());

        //
        // TODO - mloskot
        //  1. What's the purpose of isEndPoint() and how to test it?
        //  2. Add new test cases
        //

    }

}

template<>
template<>
void object::test<2>
()
{
    using geos::geom::Coordinate;
    using geos::noding::NodedSegmentString;
    using geos::noding::SegmentNode;

    // Create coordinates sequence
    const size_t coords_size = 2;
    auto cs = geos::detail::make_unique<geos::geom::CoordinateArraySequence>(0, coords_size);

    ensure(nullptr != cs.get());

    Coordinate c0(0, 0);
    Coordinate c1(3, 3);
    cs->add(c0);
    cs->add(c1);

    ensure_equals(cs->size(), coords_size);

    // Create SegmentString instance

    NodedSegmentString segment(cs.release(), nullptr);

    ensure_equals(segment.size(), coords_size);

    // Construct an interior node on the given NodedSegmentString
    {
        const size_t segment_index = 0;
        SegmentNode node(segment, Coordinate(0, 0), segment_index,
                         segment.getSegmentOctant(segment_index));

        ensure_equals(node.segmentIndex, segment_index);

        // on first endpoint ...
        ensure(! node.isInterior());

    }

}

template<>
template<>
void object::test<3>
()
{
    using geos::geom::Coordinate;
    using geos::noding::NodedSegmentString;
    using geos::noding::SegmentNode;

    // Create coordinates sequence
    const size_t coords_size = 2;
    auto cs = geos::detail::make_unique<geos::geom::CoordinateArraySequence>(0, coords_size);

    ensure(nullptr != cs.get());

    Coordinate c0(0, 0);
    Coordinate c1(3, 3);
    cs->add(c0);
    cs->add(c1);

    ensure_equals(cs->size(), coords_size);

    // Create SegmentString instance

    NodedSegmentString segment(cs.release(), nullptr);

    ensure_equals(segment.size(), coords_size);

    // Construct an interior node on the given NodedSegmentString
    {
        const size_t segment_index = 0;
        SegmentNode node(segment, Coordinate(2, 2), segment_index,
                         segment.getSegmentOctant(segment_index));

        ensure_equals(node.segmentIndex, segment_index);

        // on first endpoint ...
        ensure(node.isInterior());

    }

}

template<>
template<>
void object::test<4>
()
{
    using geos::geom::Coordinate;
    using geos::noding::NodedSegmentString;
    using geos::noding::SegmentNode;

    // Create coordinates sequence
    const size_t coords_size = 2;
    auto cs = geos::detail::make_unique<geos::geom::CoordinateArraySequence>(0, coords_size);

    ensure(nullptr != cs.get());

    Coordinate c0(0, 0);
    Coordinate c1(3, 3);
    cs->add(c0);
    cs->add(c1);

    ensure_equals(cs->size(), coords_size);

    // Create SegmentString instance

    NodedSegmentString segment(cs.release(), nullptr);

    ensure_equals(segment.size(), coords_size);

    // Construct a node that doesn't even intersect !!
    {
        const size_t segment_index = 0;
        SegmentNode node(segment, Coordinate(1, 2), segment_index,
                         segment.getSegmentOctant(segment_index));

        ensure_equals(node.segmentIndex, segment_index);

        // on first endpoint ...
        ensure(node.isInterior());

    }

}

} // namespace tut
