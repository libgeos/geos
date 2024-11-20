#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geos_printdouble_data : public capitest::utility {};

typedef test_group<test_geos_printdouble_data> group;
typedef group::object object;

group test_geos_printdouble("capi::GEOS_printDouble");

template<>
template<>
void object::test<1>()
{
    struct TESTCASE {
        TESTCASE(unsigned int p_p, double p_d, std::string p_expected)
            : p(p_p), d(p_d), expected(p_expected) {}
        unsigned int p;
        double d;
        std::string expected;
    };
    std::vector<TESTCASE> testcase_l{
        TESTCASE(1, 0.0, "0"),
        TESTCASE(1, std::nan("0"), "NaN"),
        TESTCASE(1, std::numeric_limits<double>::infinity(), "Infinity"),
        TESTCASE(1, -std::numeric_limits<double>::infinity(), "-Infinity"),
        TESTCASE(16, 1.0, "1"),
        TESTCASE(16, 1.2e+234, "1.2e+234"),
        TESTCASE(2, -1.2e+234, "-1.2e+234"),
        TESTCASE(16, 1.2e-234, "1.2e-234"),
        TESTCASE(2, -1.2e-234, "-1.2e-234"),
        TESTCASE(2, 1.1e-5, "1.1e-5"),
        TESTCASE(0, 1e-4, "0.0001"),
        TESTCASE(1, 1e-4, "0.0001"),
        TESTCASE(2, 1e-4, "0.0001"),
        TESTCASE(3, 1e-4, "0.0001"),
        TESTCASE(4, 1e-4, "0.0001"),
        TESTCASE(5, 1e-4, "0.0001"),
        TESTCASE(0, 5.6e-4, "0.0006"),
        TESTCASE(1, 5.6e-4, "0.0006"),
        TESTCASE(2, 5.6e-4, "0.0006"),
        TESTCASE(3, 5.6e-4, "0.0006"),
        TESTCASE(4, 5.6e-4, "0.0006"),
        TESTCASE(5, 5.6e-4, "0.00056"),
        TESTCASE(0, 1.2345678901234e+15, "1234567890123400"),
        TESTCASE(1, 1.2345678901234e+15, "1234567890123400"),
        TESTCASE(0, 1.2345678901234e+16, "12345678901234000"),
        TESTCASE(1, 1.2345678901234e+16, "12345678901234000"),
        TESTCASE(0, 1.2345678901234e+17, "1e+17"),
        TESTCASE(1, 1.2345678901234e+17, "1.2e+17"),
        TESTCASE(2, 1.2345678901234e+17, "1.23e+17"),
        TESTCASE(3, 1.2345678901234e+17, "1.235e+17"),
        TESTCASE(4, 1.2345678901234e+17, "1.2346e+17"),
        TESTCASE(5, 1.2345678901234e+17, "1.23457e+17"),
        TESTCASE(6, 1.2345678901234e+17, "1.234568e+17"),
        TESTCASE(7, 1.2345678901234e+17, "1.2345679e+17"),
        TESTCASE(8, 1.2345678901234e+17, "1.23456789e+17"),
        TESTCASE(9, 1.2345678901234e+17, "1.23456789e+17"),
        TESTCASE(10, 1.2345678901234e+17, "1.2345678901e+17"),
        TESTCASE(11, 1.2345678901234e+17, "1.23456789012e+17"),
        TESTCASE(12, 1.2345678901234e+17, "1.234567890123e+17"),
        TESTCASE(13, 1.2345678901234e+17, "1.2345678901234e+17"),
        TESTCASE(14, 1.2345678901234e+17, "1.2345678901234e+17"),
        TESTCASE(0, 0.0123456789, "0.01"),
        TESTCASE(1, 0.0123456789, "0.01"),
        TESTCASE(2, 0.0123456789, "0.01"),
        TESTCASE(3, 0.0123456789, "0.012"),
        TESTCASE(4, 0.0123456789, "0.0123"),
        TESTCASE(0, 0.123456789, "0.1"),
        TESTCASE(1, 0.123456789, "0.1"),
        TESTCASE(2, 0.123456789, "0.12"),
        TESTCASE(3, 0.123456789, "0.123"),
        TESTCASE(4, 0.123456789, "0.1235"),
        TESTCASE(0, 1.23456789, "1"),
        TESTCASE(1, 1.23456789, "1.2"),
        TESTCASE(2, 1.23456789, "1.23"),
        TESTCASE(3, 1.23456789, "1.235"),
        TESTCASE(4, 1.23456789, "1.2346"),
    };
    for (const auto& testcase : testcase_l) {
        char buf[28];
        const auto len = GEOS_printDouble(testcase.d, testcase.p, buf);
        buf[len] = '\0';
        const auto res_str = std::string(buf);
        ensure_equals(res_str, testcase.expected);
        ensure_equals(len, static_cast<int>(testcase.expected.size()));
    }

}

} // namespace tut
