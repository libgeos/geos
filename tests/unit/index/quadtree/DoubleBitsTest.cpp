//
// Test Suite for geos::index::quadtree::DoubleBits class.

#include <tut/tut.hpp>
// geos
#include <geos/index/quadtree/DoubleBits.h>

using namespace geos::index::quadtree;

namespace tut {
// dummy data, not used
struct test_doublebits_data {};

typedef test_group<test_doublebits_data> group;
typedef group::object object;

group test_doublebits_group("geos::index::quadtree::DoubleBits");

//
// Test Cases
//

// 1 - Test exponent()
template<>
template<>
void object::test<1>
()
{
    ensure_equals(DoubleBits::exponent(-1), 0);
    ensure_equals(DoubleBits::exponent(8.0), 3);
    ensure_equals(DoubleBits::exponent(128.0), 7);
}

// 2 - Test toString()
template<>
template<>
void object::test<2>
()
{
    ensure_equals(DoubleBits(-1).toString(),
        "1  01111111111(0) 0000000000000000000000000000000000000000000000000000 [ -1.000000 ]");
    ensure_equals(DoubleBits(8.0).toString(),
        "0  10000000010(3) 0000000000000000000000000000000000000000000000000000 [ 8.000000 ]");
    ensure_equals(DoubleBits(128.0).toString(),
        "0  10000000110(7) 0000000000000000000000000000000000000000000000000000 [ 128.000000 ]");
}

} // namespace tut

