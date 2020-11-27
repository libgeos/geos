// Test Suite for MortonCode class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/constants.h>
#include <geos/shape/fractal/MortonCode.h>
#include <geos/geom/Coordinate.h>
// std

using geos::shape::fractal::MortonCode;
using geos::geom::Coordinate;

namespace tut {

// Common data used by tests
struct test_mortoncode_data {

    void checkDecode(uint32_t index, int x, int y)
    {
        Coordinate p = MortonCode::decode(index);
        //System.out.println(p);
        ensure_equals(x, (int) p.x);
        ensure_equals(y, (int) p.y);
    }

    void checkDecodeEncodeForLevel(uint32_t level)
    {
        uint32_t n = MortonCode::levelSize(level);
        for (uint32_t i = 0; i < n; i++) {
            checkDecodeEncode(i);
        }
    }


    void
    checkDecodeEncode(uint32_t index)
    {
        Coordinate p = MortonCode::decode(index);
        uint32_t encode = MortonCode::encode((uint32_t)(p.x), (uint32_t)(p.y));
        ensure_equals(index, encode);
    }

};

typedef test_group<test_mortoncode_data> group;
typedef group::object object;

group test_mortoncode_group("geos::shape::fractal::MortonCode");

template<>
template<>
void object::test<1>
()
{
    ensure_equals( MortonCode::levelSize( 0 ), 1u);
    ensure_equals( MortonCode::levelSize( 1 ), 4u);
    ensure_equals( MortonCode::levelSize( 2 ), 16u);
    ensure_equals( MortonCode::levelSize( 3 ), 64u);
    ensure_equals( MortonCode::levelSize( 4 ), 256u);
    ensure_equals( MortonCode::levelSize( 5 ), 1024u);
    ensure_equals( MortonCode::levelSize( 6 ), 4096u);
}


template<>
template<>
void object::test<2>
()
{
    ensure_equals( MortonCode::level( 1 ), 0u);

    ensure_equals( MortonCode::level( 2 ), 1u);
    ensure_equals( MortonCode::level( 3 ), 1u);
    ensure_equals( MortonCode::level( 4 ), 1u);

    ensure_equals( MortonCode::level( 5 ), 2u);
    ensure_equals( MortonCode::level( 13 ), 2u);
    ensure_equals( MortonCode::level( 15 ), 2u);
    ensure_equals( MortonCode::level( 16 ), 2u);

    ensure_equals( MortonCode::level( 17 ), 3u);
    ensure_equals( MortonCode::level( 63 ), 3u);
    ensure_equals( MortonCode::level( 64 ), 3u);

    ensure_equals( MortonCode::level( 65 ), 4u);
    ensure_equals( MortonCode::level( 255 ), 4u);
    ensure_equals( MortonCode::level( 255 ), 4u);
    ensure_equals( MortonCode::level( 256 ), 4u);
}


template<>
template<>
void object::test<3>
()
{
    checkDecode(0, 0, 0);
    checkDecode(1, 1, 0);
    checkDecode(2, 0, 1);
    checkDecode(3, 1, 1);
    checkDecode(4, 2, 0);

    checkDecode(24, 4, 2);
    checkDecode(124, 14, 6);
    checkDecode(255, 15, 15);
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
