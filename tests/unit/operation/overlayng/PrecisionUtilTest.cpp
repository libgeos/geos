//
// Test Suite for geos::operation::overlayng::PrecisionUtil class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/PrecisionUtil.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::overlayng;
using geos::io::WKTReader;
using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_precisionutil_data {

    WKTReader r;
    WKTWriter w;

    void
    checkRobustScale(const std::string& wkt_a, const std::string& wkt_b,
        double scaleExpected, double safeScaleExpected, double inherentScaleExpected)
    {
        std::unique_ptr<Geometry> a = r.read(wkt_a);
        std::unique_ptr<Geometry> b(nullptr);
        if (wkt_b.length() > 0) {
            b = r.read(wkt_b);
        }
        double robustScale = PrecisionUtil::robustScale(a.get(), b.get());
        ensure_equals("Inherent scale: ", PrecisionUtil::inherentScale(a.get(), b.get()), inherentScaleExpected );
        ensure_equals("Safe scale: ", PrecisionUtil::safeScale(a.get(), b.get()), safeScaleExpected );
        ensure_equals("Auto scale: ", robustScale, scaleExpected );
    }

};

typedef test_group<test_precisionutil_data> group;
typedef group::object object;

group test_precisionutil_group("geos::operation::overlayng::PrecisionUtil");

//
// Test Cases
//


// testInts
template<>
template<>
void object::test<1> ()
{
    checkRobustScale(
        "POINT(1 1)",
        "POINT(10 10)",
        1, 1e12, 1 );
}

// testBNull
template<>
template<>
void object::test<2> ()
{
    checkRobustScale("POINT(1 1)", "",
        1, 1e13, 1 );
}

// testPower10
template<>
template<>
void object::test<3> ()
{
    checkRobustScale(
        "POINT(100 100)",
        "POINT(1000 1000)",
        1, 1e11, 1 );
}

// testDecimalsDifferent
template<>
template<>
void object::test<4> ()
{
    checkRobustScale(
        "POINT( 1.123 1.12 )",
        "POINT( 10.123 10.12345 )",
        1e5, 1e12, 1e5 );
}

// testDecimalsShort
template<>
template<>
void object::test<5> ()
{
    checkRobustScale(
        "POINT(1 1.12345)",
        "POINT(10 10)",
        1e5, 1e12, 1e5 );
}

// testDecimalsMany
template<>
template<>
void object::test<6> ()
{
    checkRobustScale(
        "POINT(1 1.123451234512345)",
        "POINT(10 10)",
        1e12, 1e12, 1e16 );
}

// testDecimalsAllLong
template<>
template<>
void object::test<7> ()
{
    checkRobustScale(
        "POINT( 1.123451234512345 1.123451234512345 )",
        "POINT( 10.123451234512345 10.123451234512345 )",
        1e12, 1e12, 1e16 );
}

// testSafeScaleChosen
template<>
template<>
void object::test<8> ()
{
    checkRobustScale(
        "POINT( 123123.123451234512345 1 )",
        "POINT( 10 10 )",
        1e8, 1e8, 1e11 );
}

// testSafeScaleChosenLargeMagnitude
template<>
template<>
void object::test<9> ()
{
    checkRobustScale(
        "POINT( 123123123.123451234512345 1 )",
        "POINT( 10 10 )",
        1e5, 1e5, 1e8 );
}

// testInherentWithLargeMagnitude
template<>
template<>
void object::test<10> ()
{
    checkRobustScale(
        "POINT( 123123123.12 1 )",
        "POINT( 10 10 )",
        1e2, 1e5, 1e2 );
}

// testMixedMagnitude
template<>
template<>
void object::test<11> ()
{
    checkRobustScale(
        "POINT( 1.123451234512345 1 )",
        "POINT( 100000.12345 10 )",
        1e8, 1e8, 1e16 );
}

// testInherentBelowSafe
template<>
template<>
void object::test<12> ()
{
    checkRobustScale(
        "POINT( 100.1234512 1 )",
        "POINT( 100.12345 10 )",
        1e7, 1e11, 1e7 );
}


// numberOfDecimals
template<>
template<>
void object::test<13> ()
{
    ensure_equals("14.12346", PrecisionUtil::numberOfDecimals(14.12346), 5);
    ensure_equals("1", PrecisionUtil::numberOfDecimals(1), 0);
    ensure_equals("10", PrecisionUtil::numberOfDecimals(10), 0);
    ensure_equals("10.4", PrecisionUtil::numberOfDecimals(10.4), 1);
    ensure_equals("14.0", PrecisionUtil::numberOfDecimals(14.0), 0);
    ensure_equals("14.041", PrecisionUtil::numberOfDecimals(14.041), 3);
    ensure_equals("14.1234512", PrecisionUtil::numberOfDecimals(14.1234512), 7);
    ensure_equals("14.1234512346", PrecisionUtil::numberOfDecimals(14.1234512346), 10);
    ensure_equals("14.123451234512345", PrecisionUtil::numberOfDecimals(14.123451234512345), 15);
}







} // namespace tut
