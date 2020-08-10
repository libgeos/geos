//
// Test Suite for geos::geom::Location class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/geom/Location.h>
#include <geos/util/IllegalArgumentException.h>
#include <sstream>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_location_data {
    geos::geom::Location undef;
    geos::geom::Location interior;
    geos::geom::Location boundary;
    geos::geom::Location exterior;
    test_location_data()
        : undef(geos::geom::Location::NONE),
          interior(geos::geom::Location::INTERIOR),
          boundary(geos::geom::Location::BOUNDARY),
          exterior(geos::geom::Location::EXTERIOR)
    {}
};

typedef test_group<test_location_data> group;
typedef group::object object;

group test_location_group("geos::geom::Location");

//
// Test Cases
//

// Test of default constructor
template<>
template<>
void object::test<1>
()
{
    ensure("NOTE: Location has no default constructor.", true);
}

// Test of << operator
template<>
template<>
void object::test<2>
()
{
    using geos::geom::Location;

    std::stringstream s;

    s << Location::EXTERIOR;
    ensure_equals(s.str(), "e");
    s.str(""); // reset
    s.clear();

    s << Location::BOUNDARY;
    ensure_equals(s.str(), "b");
    s.str(""); // reset
    s.clear();

    s << Location::INTERIOR;
    ensure_equals(s.str(), "i");
    s.str(""); // reset
    s.clear();

    s << Location::NONE;
    ensure_equals(s.str(), "-");
}

} // namespace tut

