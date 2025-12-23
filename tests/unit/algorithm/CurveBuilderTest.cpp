#include <tut/tut.hpp>

#include <geos/io/WKTReader.h>

#include "utility.h"

namespace tut {

struct test_curvebuilder_data {
    geos::io::WKTReader reader_;

    void checkRoundTrip(const std::string& wkt_in, const std::string& wkt_expected, double stepSizeDegrees) const {
        auto inCurve = reader_.read(wkt_in);

        auto linearized = inCurve->getLinearized(stepSizeDegrees);
        auto outCurve = linearized->getCurved(1e-6);
        auto expected = reader_.read(wkt_expected);

        double distanceTolerance = 1e-4;

        ensure_equals_exact_geometry_xyzm(outCurve.get(), expected.get(), distanceTolerance);
    }

    void checkRoundTripUnchanged(const std::string& wkt_in, double stepSizeDegrees) const {
        checkRoundTrip(wkt_in, wkt_in, stepSizeDegrees);
    }

    void checkLineToCurve(const std::string& wkt_in, const std::string& wkt_expected, double distanceTolerance) const {
        auto ls = reader_.read<LineString>(wkt_in);
        auto curve = ls->getCurved(distanceTolerance);
        auto expected = reader_.read(wkt_expected);

        ensure_equals_exact_geometry_xyzm(curve.get(), expected.get(), distanceTolerance);
    }

    void checkLineToCurveUnchanged(const std::string& wkt_in, double distanceTolerance) const {
        auto ls = reader_.read<LineString>(wkt_in);
        auto expected = ls->clone();
        auto curve = ls->getCurved(distanceTolerance);

        ensure_equals_exact_geometry_xyzm(curve.get(), expected.get(), 0.0);
    }
};

typedef test_group<test_curvebuilder_data> group;
typedef group::object object;

group test_curvebuilder_group("geos::algorithm::CurveBuilder");

template<>
template<>
void object::test<1>()
{
    set_test_name("two-point LineString");

    checkLineToCurveUnchanged("LINESTRING (3 4, 2 7)", 1);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("LineString of three collinear points");

    checkLineToCurveUnchanged("LINESTRING (0 0, 1 2, 2 4)", 1);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("coarsely linearized semicircle");

    checkLineToCurve("LINESTRING(0 0,29.2893 70.7107,100 100,170.7107 70.7107,200 0)",
                     "CIRCULARSTRING (0 0, 100 100, 200 0)",
                     2e-3);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("coarsely linearized closed semicircle");

    checkLineToCurve("LINESTRING(0 0,29.2893 70.7107,100 100,170.7107 70.7107,200 0, 0 0)",
                     "COMPOUNDCURVE (CIRCULARSTRING (0 0, 100 100, 200 0), LINESTRING (200 0, 0 0))",
                     2.5e-3);
}

template<>
template<>
void object::test<5>()
{
    set_test_name("round-trip with 3-quadrant CircularString");

    // cu_lwstroke.c: 392
    checkRoundTrip("CIRCULARSTRING (-1 0, 0 1, 0 -1)",
                   "CIRCULARSTRING (-1 0, 0.70710678 0.707010678, 0 -1)",
                   90.0 / 8);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("Two-part CompoundCurve round-trip");

    // cu_lwstroke.c: 404
    checkRoundTrip("COMPOUNDCURVE (CIRCULARSTRING (-1 0, 0 1, 0 -1), (0 -1, -1 -1))",
                   "COMPOUNDCURVE( CIRCULARSTRING (-1 0, 0.70710678 0.707010678, 0 -1), (0 -1, -1 -1))",
                   90.0 / 8);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("Three-part CompoundCurve round-trip");

    // cu_lwstroke.c: 416
    checkRoundTrip("COMPOUNDCURVE((-3 -3,-1 0),CIRCULARSTRING(-1 0,0 1,0 -1),(0 -1,0 -1.5,0 -2),CIRCULARSTRING(0 -2,-1 -3,1 -3),(1 -3,5 5))",
                   "COMPOUNDCURVE((-3 -3,-1 0),CIRCULARSTRING(-1 0,0.70710678 0.70710678,0 -1),(0 -1,0 -1.5,0 -2),CIRCULARSTRING(0 -2,-0.70710678 -3.70710678,1 -3),(1 -3,5 5))",
                   90.0 / 8);
}

template<>
template<>
void object::test<8>()
{
    set_test_name("CompoundCurve with two CircularStrings, round-trip");

    // cu_lwstroke.c: 432
    // NOTE: The expected result here is modified from the original. It is not clear why liblwgeom returns a
    // CompoundCurve with two CircularStrings instead of a single CircularString (like GDAL). The expected result
    // used here is from GDAL.
    checkRoundTrip("COMPOUNDCURVE(CIRCULARSTRING(-1 0,0 1,0 -1),CIRCULARSTRING(0 -1,-1 -2,1 -2))",
                   "CIRCULARSTRING(-1 0,0.70710678 0.70710678,0 -1,-0.70710678 -2.70710678,1 -2)",
                   90.0 / 8);
}

template<>
template<>
void object::test<9>()
{
    set_test_name("CompoundCurve with a CircularString between two LineStrings, round-trip");

    // cu_lwstroke.c: 447
    checkRoundTripUnchanged("COMPOUNDCURVE((0 0, 1 1), CIRCULARSTRING(1 1, 2 2, 3 1), (3 1, 4 4))",
                       90.0 / 8);
}

template<>
template<>
void object::test<10>()
{
    set_test_name("LineString forming a square, round-trip");

    // cu_lwstroke.c: 461
    checkRoundTripUnchanged("LINESTRING(0 0,10 0,10 10,0 10,0 0)",
                       90.0 / 8);

    // cu_lwstroke.c: 469
    checkRoundTripUnchanged("LINESTRING(10 10,0 10,0 0,10 0)",
                            90.0 / 8);

    // cu_lwstroke.c: 478
    checkRoundTripUnchanged("LINESTRING(0 0,10 0,10 10,0 10)",
                       90.0 / 8);
}

template<>
template<>
void object::test<11>()
{
    set_test_name("collection of two-point LineStrings, round-trip");

    // cu_lwstroke.c: 497
    checkRoundTripUnchanged("GEOMETRYCOLLECTION(LINESTRING(10 10,10 11),LINESTRING(10 11,11 11),LINESTRING(11 11,10 10))",
                       90.0 / 8);
}

template<>
template<>
void object::test<13>()
{
    set_test_name("collection of two-point LineStrings and a CircularString, round-trip");

    // cu_lwstroke.c: 508
    checkRoundTripUnchanged("GEOMETRYCOLLECTION(LINESTRING(4 4,4 8),CIRCULARSTRING(4 8,6 10,8 8),LINESTRING(8 8,8 4))",
                       90.0 / 8);
}

template<>
template<>
void object::test<14>()
{
    set_test_name("round-trip with 5-point CircularString");

    checkRoundTripUnchanged("CIRCULARSTRING (-5 0, 0 5, 5 0, 4 1, 3 0)",
                   90.0 / 4);
}

template<>
template<>
void object::test<15>()
{
    set_test_name("constructed curve does not depend on direction of input");

    auto cs = reader_.read("CIRCULARSTRING (-5 0, 0 5, 5 0)");

    auto lin = cs->getLinearized(90.0 / 4);
    auto linRev = lin->reverse();

    auto curveRev = linRev->getCurved(1e-4);
    auto curve1 = curveRev->reverse();

    auto curve2 = lin->getCurved(1e-4);

    ensure_equals_exact_geometry_xyzm(curve1.get(), curve2.get(), 0);
}


}
