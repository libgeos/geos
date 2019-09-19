// Test Suite for geos::precision::CommonBits class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/constants.h>
#include <geos/precision/CommonBits.h>
// std

using geos::precision::CommonBits;

namespace tut {
// Common data used by tests
struct test_commonbits_data {};

typedef test_group<test_commonbits_data> group;
typedef group::object object;

group test_commonbits_group("geos::precision::CommonBits");

// getBit from zero.
template<>
template<>
void object::test<1>
()
{
    constexpr int64 val = 0ull;
    for(int i = 0; i < 64; i++) {
        ensure_equals(CommonBits::getBit(val, i), 0);
    }
}

// getBit from all ones.
template<>
template<>
void object::test<2>
()
{
    constexpr int64 val = 0xffffffffffffffffull;
    for(int i = 0; i < 64; i++) {
        ensure_equals(CommonBits::getBit(val, i), 1);
    }
}

// getBit check high versus low bits.
template<>
template<>
void object::test<3>
()
{
    constexpr int64 val = 0xffffffff00000000ull;
    ensure_equals(CommonBits::getBit(val, 0), 0);
    ensure_equals(CommonBits::getBit(val, 63), 1);
}

// zeroLowerBits.
template<>
template<>
void object::test<4>()
{
    constexpr int64 val = static_cast<int64>(0xffffffffffffffffull);
    ensure_equals(sizeof(val), 8u);

    ensure_equals(CommonBits::zeroLowerBits(val, -1), 0);
    ensure_equals(CommonBits::zeroLowerBits(val, 0), -1);
    ensure_equals(CommonBits::zeroLowerBits(val, 1), -2);
    ensure_equals(CommonBits::zeroLowerBits(val, 2), -4);
    ensure_equals(CommonBits::zeroLowerBits(val, 16), -65536);
    ensure_equals(CommonBits::zeroLowerBits(val, 31), -2147483648ll);
    ensure_equals(CommonBits::zeroLowerBits(val, 32), -4294967296ll);
    ensure_equals(CommonBits::zeroLowerBits(val, 62), -4611686018427387904ll);
    ensure_equals(static_cast<uint64_t>(CommonBits::zeroLowerBits(val, 62)), 0xc000000000000000ull);
    ensure_equals(static_cast<uint64_t>(CommonBits::zeroLowerBits(val, 63)), 9223372036854775808ull);
    ensure_equals(static_cast<uint64_t>(CommonBits::zeroLowerBits(val, 63)), 0x8000000000000000ull);
    ensure_equals(CommonBits::zeroLowerBits(val, 64), 0);
    ensure_equals(CommonBits::zeroLowerBits(val, 10000), 0);
}


} // namespace tut
