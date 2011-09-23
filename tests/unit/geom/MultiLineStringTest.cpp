// $Id: MultiLineStringTest.cpp 2344 2009-04-09 21:46:30Z mloskot $
// 
// Test Suite for geos::geom::MultiLineString class.

#include <tut.hpp>
// geos
#include <geos/geom/MultiLineString.h>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_multilinestring_data
    {
        test_multilinestring_data() {}
    };

    typedef test_group<test_multilinestring_data> group;
    typedef group::object object;

    group test_multilinestring_group("geos::geom::MultiLineString");

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

