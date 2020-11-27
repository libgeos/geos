// Test Suite for HilbertCode class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/constants.h>
#include <geos/shape/fractal/HilbertCode.h>
#include <geos/geom/Coordinate.h>
// std

using geos::shape::fractal::HilbertCode;
using geos::geom::Coordinate;

namespace tut {

// Common data used by tests
struct test_hilbertcode_data {

    void checkDecode(uint32_t order, uint32_t index, int x, int y)
    {
        Coordinate p = HilbertCode::decode(order, index);
        //System.out.println(p);
        ensure_equals(x, (int) p.x);
        ensure_equals(y, (int) p.y);
    }

    void checkDecodeEncodeForLevel(uint32_t level)
    {
        uint32_t n = HilbertCode::levelSize(level);
        for (uint32_t i = 0; i < n; i++) {
            checkDecodeEncode(level, i);
        }
    }


    void
    checkDecodeEncode(uint32_t level, uint32_t index)
    {
        Coordinate p = HilbertCode::decode(level, index);
        uint32_t encode = HilbertCode::encode(level, (uint32_t)(p.x), (uint32_t)(p.y));
        ensure_equals(index, encode);
    }

};

typedef test_group<test_hilbertcode_data> group;
typedef group::object object;

group test_hilbertcode_group("geos::shape::fractal::HilbertCode");

template<>
template<>
void object::test<1>
()
{
    ensure_equals( HilbertCode::levelSize( 0 ), 1u);
    ensure_equals( HilbertCode::levelSize( 1 ), 4u);
    ensure_equals( HilbertCode::levelSize( 2 ), 16u);
    ensure_equals( HilbertCode::levelSize( 3 ), 64u);
    ensure_equals( HilbertCode::levelSize( 4 ), 256u);
    ensure_equals( HilbertCode::levelSize( 5 ), 1024u);
    ensure_equals( HilbertCode::levelSize( 6 ), 4096u);
}


template<>
template<>
void object::test<2>
()
{
    ensure_equals( HilbertCode::level( 1 ), 0u);

    ensure_equals( HilbertCode::level( 2 ), 1u);
    ensure_equals( HilbertCode::level( 3 ), 1u);
    ensure_equals( HilbertCode::level( 4 ), 1u);

    ensure_equals( HilbertCode::level( 5 ), 2u);
    ensure_equals( HilbertCode::level( 13 ), 2u);
    ensure_equals( HilbertCode::level( 15 ), 2u);
    ensure_equals( HilbertCode::level( 16 ), 2u);

    ensure_equals( HilbertCode::level( 17 ), 3u);
    ensure_equals( HilbertCode::level( 63 ), 3u);
    ensure_equals( HilbertCode::level( 64 ), 3u);

    ensure_equals( HilbertCode::level( 65 ), 4u);
    ensure_equals( HilbertCode::level( 255 ), 4u);
    ensure_equals( HilbertCode::level( 255 ), 4u);
    ensure_equals( HilbertCode::level( 256 ), 4u);
}


template<>
template<>
void object::test<3>
()
{
    checkDecode(1, 0, 0, 0);

    checkDecode(1, 0, 0, 0);
    checkDecode(1, 1, 0, 1);

    checkDecode(3, 0, 0, 0);
    checkDecode(3, 1, 0, 1);

    checkDecode(4,0, 0, 0);
    checkDecode(4, 1, 1, 0);
    checkDecode(4, 24, 6, 2);
    checkDecode(4, 255, 15, 0);

    checkDecode(5, 124, 8, 6);
}


template<>
template<>
void object::test<4>
()
{
    checkDecodeEncodeForLevel(4);
    checkDecodeEncodeForLevel(5);
}


} // namespace tut
