// 
// Test Suite for geos::geom::MultiPolygon class.

#include <tut.hpp>
// geos
#include <geos/geom/MultiPolygon.h>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_multipolygon_data
    {
        test_multipolygon_data() {}
    };

    typedef test_group<test_multipolygon_data> group;
    typedef group::object object;

    group test_multipolygon_group("geos::geom::MultiPolygon");

    //
    // Test Cases
    //

    // Test of default constructor
    template<>
    template<>
    void object::test<1>()
    {
	    //inform("Test not implemented!");
    }

} // namespace tut
