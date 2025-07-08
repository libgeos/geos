//
// Test Suite for geos::algorithm::distance::DiscreteFrechetDistance
//

#include <tut/tut.hpp>
// geos
#include <geos/constants.h>
#include <geos/io/WKTReader.h>
#include <geos/algorithm/distance/DiscreteFrechetDistance.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h> // required for use in unique_ptr
#include <geos/geom/Coordinate.h>
#include <geos/util.h>
// std
#include <cmath>
#include <string>
#include <memory>

namespace geos {
namespace geom {
class Geometry;
}
}

using namespace geos::geom;
using namespace geos::algorithm::distance; // for Location

namespace tut {
//
// Test Group
//

struct test_frechetdistance_data {

    geos::io::WKTReader reader;
    static constexpr double TOLERANCE = 0.00001;

    void
    checkDiscreteFrechet(const std::string& wkt1, const std::string& wkt2,
            double expectedDistance)
    {
        std::unique_ptr<Geometry> g1(reader.read(wkt1));
        std::unique_ptr<Geometry> g2(reader.read(wkt2));

        double distance = DiscreteFrechetDistance::distance(*g1, *g2);
        ensure_equals("checkDiscreteFrechet", distance, expectedDistance, TOLERANCE);
    }

    void
    checkDensifiedFrechet(const std::string& wkt1, const std::string& wkt2,
            double densifyFactor, double expectedDistance)
    {
        std::unique_ptr<Geometry> g1(reader.read(wkt1));
        std::unique_ptr<Geometry> g2(reader.read(wkt2));

        double distance = DiscreteFrechetDistance::distance(*g1, *g2, densifyFactor);
        ensure_equals("checkDensifiedFrechet", distance, expectedDistance, TOLERANCE);
    }

};

typedef test_group<test_frechetdistance_data> group;
typedef group::object object;

group test_frechetdistance_group("geos::algorithm::distance::DiscreteFrechetDistance");

// 1 - testLineSegments
template<>
template<>
void object::test<1> ()
{
    checkDiscreteFrechet("LINESTRING (0 0, 2 1)", "LINESTRING (0 0, 2 0)", 1.0);

    // zero densify factor
    try {
        checkDensifiedFrechet("LINESTRING (0 0, 2 1)", "LINESTRING EMPTY", 0.0, 0);
    }
    catch(const geos::util::IllegalArgumentException& e) {
        // We do expect an exception
        ::geos::ignore_unused_variable_warning(e);
    }

    // too big densify factor
    try {
        checkDensifiedFrechet("LINESTRING (0 0, 2 1)", "LINESTRING EMPTY", 1 + 1e-10, 0);
    }
    catch(const geos::util::IllegalArgumentException& e) {
        // We do expect an exception
        ::geos::ignore_unused_variable_warning(e);
    }

    // too small positive densify factor
    try {
        checkDensifiedFrechet("LINESTRING (0 0, 2 1)", "LINESTRING EMPTY", 1e-30, 0);
    }
    catch(const geos::util::IllegalArgumentException& e) {
        // We do expect an exception
        ::geos::ignore_unused_variable_warning(e);
    }
}

// 2 - testLineSegments2
template<>
template<>
void object::test<2> ()
{
    checkDiscreteFrechet("LINESTRING (0 0, 2 0)", "LINESTRING (0 1, 1 2, 2 1)", 2.23606797749979);
}

// 3 - testLinePoints
template<>
template<>
void object::test<3> ()
{
    checkDiscreteFrechet("LINESTRING (0 0, 2 0)", "MULTIPOINT ((0 1), (1 0), (2 1))", 1.0);
}

// 4 - testLinesShowingDiscretenessEffect
//
// Shows effects of limiting FD to vertices
// Answer is not true Frechet distance.
//
template<>
template<>
void object::test<4> ()
{
    checkDiscreteFrechet("LINESTRING (0 0, 100 0)", "LINESTRING (0 0, 50 50, 100 0)", 70.7106781186548);
    // densifying provides accurate HD
    checkDensifiedFrechet("LINESTRING (0 0, 100 0)", "LINESTRING (0 0, 50 50, 100 0)", 0.5, 50.0);
}

// 5 - test Line Segments revealing distance initialization bug
template<>
template<>
void object::test<5> ()
{
    checkDiscreteFrechet("LINESTRING (1 1, 2 2)", "LINESTRING (1 4, 2 3)", 3);
}

// Empty arguments should throw error
template<>
template<>
void object::test<6> ()
{
    auto g1 = reader.read("LINESTRING EMPTY");
    auto g2 = reader.read("POLYGON EMPTY");

    try {
        DiscreteFrechetDistance::distance(*g1, *g2);
    } catch (const geos::util::GEOSException& e) {
        ::geos::ignore_unused_variable_warning(e);
    }
}

// Large test data set caused stack overflow in old
// recursive version of the algorithm
// https://github.com/libgeos/geos/issues/516

#include "DiscreteFrechetDistanceData.h"

template<>
template<>
void object::test<7> ()
{
    checkDiscreteFrechet(LS1, LS2, 2.49903e-04);
}

template<>
template<>
void object::test<8> ()
{
    checkDensifiedFrechet("LINESTRING(1 0, 2 0)", "LINESTRING(-1 0, 0 0, 7 8)", 0.002, 9.43398);
}

template<>
template<>
void object::test<9> ()
{
    checkDensifiedFrechet("POINT(-11.1111111 40)", "POINT(-11.1111111 40)", 0.8, 0);
}

} // namespace tut
