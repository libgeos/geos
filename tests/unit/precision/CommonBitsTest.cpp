// Test Suite for geos::precision::CommonBits class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/constants.h>
#include <geos/precision/CommonBits.h>
// std

using geos::precision::CommonBits;

namespace tut
{
    // Common data used by tests
    struct test_commonbits_data {};

    typedef test_group<test_commonbits_data> group;
    typedef group::object object;

    group test_commonbits_group("geos::precision::CommonBits");

    // getBit from zero.
    template<>
    template<>
    void object::test<1>()
    {
        constexpr int64 val = 0ull;
        for (int i=0; i < 64; i++)
        {
            ensure_equals(CommonBits::getBit(val, i), 0);
        }
    }

    // getBit from all ones.
    template<>
    template<>
    void object::test<2>()
    {
        constexpr int64 val = 0xffffffffffffffffull;
        for (int i=0; i < 64; i++)
        {
            ensure_equals(CommonBits::getBit(val, i), 1);
        }
    }

    // getBit check high versus low bits.
    template<>
    template<>
    void object::test<3>()
    {
        constexpr int64 val = 0xffffffff00000000ull;
        ensure_equals(CommonBits::getBit(val, 0), 0);
        ensure_equals(CommonBits::getBit(val, 63), 1);
    }

} // namespace tut
