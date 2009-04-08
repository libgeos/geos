// $Id$
// 
// Test Suite for geos::noding::SegmentNode class.

// TUT
#include <tut.h>
// GEOS
#include <geos/noding/SegmentNode.h>
#include <geos/noding/NodedSegmentString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by all tests
    struct test_segmentnode_data
    {

    	typedef std::auto_ptr<geos::geom::CoordinateSequence>
            CoordSeqPtr;

    	typedef std::auto_ptr<geos::noding::SegmentString>
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
    void object::test<1>()
    {
        using geos::geom::Coordinate;
        using geos::noding::NodedSegmentString;
        using geos::noding::SegmentNode;

        // Create coordinates sequence
        const size_t coords_size = 2;
        CoordSeqPtr cs( factory_->create(0, coords_size) );

        ensure( 0 != cs.get() );

        Coordinate c0(0, 0);
        Coordinate c1(3, 3);
        cs->add(c0);
        cs->add(c1);

        ensure_equals( cs->size(), coords_size );

        // Create SegmentString instance
        
        NodedSegmentString segment(cs.get(), 0);

        ensure_equals( segment.size(), coords_size );

        // Construct a node on the given NodedSegmentString
        {
            const size_t segment_index = 0;
            Coordinate cint(3, 3);
            SegmentNode node( segment, cint, segment_index,
                              segment.getSegmentOctant(segment_index) );

            ensure_equals( node.segmentIndex, segment_index );
            ensure( node.isInterior() ); 

            // 
            // TODO - mloskot 
            //  1. What's the purpose of isEndPoint() and how to test it?
            //  2. Add new test cases
            //

        }
    }

} // namespace tut
