// $Id$
// 
// Test Suite for geos::noding::SegmentPointComparator class.

// TUT
#include <tut.h>
// GEOS
#include <geos/noding/SegmentNode.h>
#include <geos/noding/SegmentString.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequenceFactory.h>
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_segmentpointcomparator_data
    {

    	typedef std::auto_ptr<geos::geom::CoordinateSequence>
            CoordinateSequenceAutoPtr;

    	typedef std::auto_ptr<geos::noding::SegmentString>
            SegmentStringAutoPtr;
	
        const geos::geom::CoordinateSequenceFactory* csFactory;

        test_segmentpointcomparator_data()
            : csFactory(geos::geom::CoordinateArraySequenceFactory::instance())
	    {}
    };

    typedef test_group<test_segmentpointcomparator_data> group;
    typedef group::object object;

    group test_segmentpointcomparator_group("geos::noding::SegmentPointComparator");

    //
    // Test Cases
    //

    // TODO
    template<>
    template<>
    void object::test<1>()
    {

        // TODO - mloskot add test cases

    }

} // namespace tut
