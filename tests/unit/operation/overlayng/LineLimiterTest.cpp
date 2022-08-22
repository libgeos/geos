//
// Test Suite for geos::operation::overlayng::LineLimiter class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/MultiLineString.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/operation/overlayng/LineLimiter.h>

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
struct test_linelimiter_data {

    WKTReader r;
    WKTWriter w;

    void
    checkLimit(std::string& wkt, const Envelope& clipEnv, std::string& wktExpected)
    {
        std::unique_ptr<Geometry> line = r.read(wkt);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);

        LineLimiter limiter(&clipEnv);
        std::vector<std::unique_ptr<CoordinateArraySequence>>& sections = limiter.limit((line->getCoordinates()).get());

        std::unique_ptr<Geometry> result = toLines(sections, line->getFactory());

        // std::cout << std::endl;
        // std::cout << "--result--" << std::endl;
        // std::cout << w.write(result.get()) << std::endl;
        // std::cout << "--expect--" << std::endl;
        // std::cout << w.write(expected.get()) << std::endl;

        ensure_equals_geometry(expected.get(), result.get());
    }

    std::unique_ptr<Geometry>
    toLines(std::vector<std::unique_ptr<CoordinateArraySequence>>& sections, const GeometryFactory* factory)
    {
        std::vector<std::unique_ptr<LineString>> lines;
        for (auto& cas: sections) {
            std::unique_ptr<LineString> line = factory->createLineString(std::move(cas));
            lines.push_back(std::move(line));
        }
        std::unique_ptr<Geometry> geomRslt;
        if (lines.size() == 1) {
            geomRslt = std::move(lines[0]);
        }
        else {
            geomRslt = factory->createMultiLineString(std::move(lines));
        }
        return geomRslt;
    }


};

typedef test_group<test_linelimiter_data> group;
typedef group::object object;

group test_linelimiter_group("geos::operation::overlayng::LineLimiter");

//
// Test Cases
//

//  testEmptyEnv
template<>
template<>
void object::test<1> ()
{
    std::string wkt= "LINESTRING (5 15, 5 25, 25 25, 25 5, 5 5)";
    Envelope env;
    std::string expected = "MULTILINESTRING EMPTY";
    checkLimit(wkt, env, expected);
}

//  testPointEnv
template<>
template<>
void object::test<2> ()
{
    std::string wkt= "LINESTRING (5 15, 5 25, 25 25, 25 5, 5 5)";
    Envelope env(10,10,10,10);
    std::string expected = "MULTILINESTRING EMPTY";
    checkLimit(wkt, env, expected);
}

//  testNonIntersecting
template<>
template<>
void object::test<3> ()
{
    std::string wkt= "LINESTRING (5 15, 5 25, 25 25, 25 5, 5 5)";
    Envelope env(10,20,10,20);
    std::string expected = "MULTILINESTRING EMPTY";
    checkLimit(wkt, env, expected);
}

//  testPartiallyInside
template<>
template<>
void object::test<4> ()
{
    std::string wkt= "LINESTRING (4 17, 8 14, 12 18, 15 15)";
    Envelope env(10,20,10,20);
    std::string expected = "LINESTRING (8 14, 12 18, 15 15)";
    checkLimit(wkt, env, expected);
}

//  testCrossing
template<>
template<>
void object::test<5> ()
{
    std::string wkt= "LINESTRING (5 17, 8 14, 12 18, 15 15, 18 18, 22 14, 25 18)";
    Envelope env(10,20,10,20);
    std::string expected = "LINESTRING (8 14, 12 18, 15 15, 18 18, 22 14)";
    checkLimit(wkt, env, expected);
}

//  testCrossesTwice
template<>
template<>
void object::test<6> ()
{
    std::string wkt= "LINESTRING (7 17, 23 17, 23 13, 7 13)";
    Envelope env(10,20,10,20);
    std::string expected = "MULTILINESTRING ((7 17, 23 17), (23 13, 7 13))";
    checkLimit(wkt, env, expected);
}

//  testDiamond
template<>
template<>
void object::test<7> ()
{
    std::string wkt= "LINESTRING (8 15, 15 22, 22 15, 15 8, 8 15)";
    Envelope env(10,20,10,20);
    std::string expected = "LINESTRING (8 15, 15 8, 22 15, 15 22, 8 15)";
    checkLimit(wkt, env, expected);
}

//  testOctagon
template<>
template<>
void object::test<8> ()
{
    std::string wkt= "LINESTRING (9 12, 12 9, 18 9, 21 12, 21 18, 18 21, 12 21, 9 18, 9 13)";
    Envelope env(10,20,10,20);
    std::string expected = "MULTILINESTRING ((9 12, 12 9), (18 9, 21 12), (21 18, 18 21), (12 21, 9 18))";
    checkLimit(wkt, env, expected);
}




} // namespace tut
