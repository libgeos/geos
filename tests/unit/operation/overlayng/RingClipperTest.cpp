//
// Test Suite for geos::operation::overlayng::RingClipper class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/operation/overlayng/RingClipper.h>

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
struct test_ringclipper_data {

    WKTReader r;
    WKTWriter w;

    void
    checkClip(std::string& wkt, const Envelope& clipEnv, std::string& wktExpected)
    {
        std::unique_ptr<Geometry> line = r.read(wkt);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);

        RingClipper clipper(&clipEnv);
        std::unique_ptr<CoordinateArraySequence> pts = clipper.clip((line->getCoordinates()).get());
        std::unique_ptr<Geometry> result = line->getFactory()->createLineString(std::move(pts));

        // std::cout << std::endl;
        // std::cout << "--result--" << std::endl;
        // std::cout << w.write(result.get()) << std::endl;
        // std::cout << "--expect--" << std::endl;
        // std::cout << w.write(expected.get()) << std::endl;

        ensure_equals_geometry(expected.get(), result.get());
    }


};

typedef test_group<test_ringclipper_data> group;
typedef group::object object;

group test_ringclipper_group("geos::operation::overlayng::RingClipper");

//
// Test Cases
//


//  testEmptyEnv
template<>
template<>
void object::test<1> ()
{
    std::string wkt = "POLYGON ((2 9, 7 27, 26 34, 45 10, 26 9, 17 -7, 14 4, 2 9))";
    Envelope env;
    std::string expected = "LINESTRING EMPTY";
    checkClip(wkt, env, expected);
}

// testPointEnv
template<>
template<>
void object::test<2> ()
{
    std::string wkt = "POLYGON ((2 9, 7 27, 26 34, 45 10, 26 9, 17 -7, 14 4, 2 9))";
    Envelope env(10,10,10,10);
    std::string expected = "LINESTRING EMPTY";
    checkClip(wkt, env, expected);
}

// testClipCompletely
template<>
template<>
void object::test<3> ()
{
    std::string wkt = "POLYGON ((2 9, 7 27, 26 34, 45 10, 26 9, 17 -7, 14 4, 2 9))";
    Envelope env(10,20,10,20);
    std::string expected = "LINESTRING (10 20, 20 20, 20 10, 10 10, 10 20)";
    checkClip(wkt, env, expected);
}

// testInside
template<>
template<>
void object::test<4> ()
{
    std::string wkt = "POLYGON ((12 13, 13 17, 18 17, 15 16, 17 12, 14 14, 12 13))";
    Envelope env(10,20,10,20);
    std::string expected = "LINESTRING (12 13, 13 17, 18 17, 15 16, 17 12, 14 14, 12 13)";
    checkClip(wkt, env, expected);
}

// testStarClipped
template<>
template<>
void object::test<5> ()
{
    std::string wkt = "POLYGON ((7 15, 12 18, 15 23, 18 18, 24 15, 18 12, 15 7, 12 12, 7 15))";
    Envelope env(10,20,10,20);
    std::string expected = "LINESTRING (10 16.8, 12 18, 13.2 20, 16.8 20, 18 18, 20 17, 20 13, 18 12, 16.8 10, 13.2 10, 12 12, 10 13.2, 10 16.8)";
    checkClip(wkt, env, expected);
}

// testStarClipped
template<>
template<>
void object::test<6> ()
{
    std::string wkt = "POLYGON ((30 60, 60 60, 40 80, 40 110, 110 110, 110 80, 90 60, 120 60, 120 120, 30 120, 30 60))";
    Envelope env(50,100, 50,100);
    std::string expected = "LINESTRING (50 60, 60 60, 50 70, 50 100, 100 100, 100 70, 90 60, 100 60, 100 100, 50 100, 50 60)";
    checkClip(wkt, env, expected);
}

// testWrapAllSides
template<>
template<>
void object::test<7> ()
{
    std::string wkt = "POLYGON ((30 80, 60 80, 60 90, 40 90, 40 110, 110 110, 110 40, 40 40, 40 59, 60 59, 60 70, 30 70, 30 30, 120 30, 120 120, 30 120, 30 80))";
    Envelope env(50,100, 50,100);
    std::string expected = "LINESTRING (50 80, 60 80, 60 90, 50 90, 50 100, 100 100, 100 50, 50 50, 50 59, 60 59, 60 70, 50 70, 50 50, 100 50, 100 100, 50 100, 50 80)";
    checkClip(wkt, env, expected);
}

// testWrapOverlap
template<>
template<>
void object::test<8> ()
{
    std::string wkt = "POLYGON ((30 80, 60 80, 60 90, 40 90, 40 110, 110 110, 110 40, 40 40, 40 59, 30 70, 20 100, 10 100, 10 30, 120 30, 120 120, 30 120, 30 80))";
    Envelope env(50,100, 50,100);
    std::string expected = "LINESTRING (50 80, 60 80, 60 90, 50 90, 50 100, 100 100, 100 50, 50 50, 100 50, 100 100, 50 100, 50 80)";
    checkClip(wkt, env, expected);
}




} // namespace tut
