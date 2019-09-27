//
// Test Suite for Geometry's normalize()

// tut
#include <tut/tut.hpp>
// geos
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
// std
#include <memory>
#include <string>
#include <iostream>

namespace tut {

//
// Test Group
//

struct test_geometry_normalize_data {
    typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;
    geos::io::WKTReader reader;
    geos::io::WKTWriter writer;

    test_geometry_normalize_data()
        : reader(), writer()
    {
        writer.setTrim(true);
    }

    void
    runTest(const char* from, const char* exp)
    {
        GeomPtr g1(reader.read(from));
        ensure(g1.get() != 0);
        GeomPtr g2(g1->clone());
        ensure(g2.get() != 0);
        g2->normalize();

        GeomPtr ge(reader.read(exp));
        bool eq = g2->equalsExact(ge.get());
        if(! eq) {
            using namespace std;
            cout << endl;
            cout << "Expected: " << exp << endl;
            cout << "Obtained: " << writer.write(g2.get()) << endl;
        }
        ensure(eq);

        // Clone and re-normalize, check it doesn't change anymore
        ge = g2->clone();
        g2->normalize();
        ensure(g2->equalsExact(ge.get()));
    }
};

typedef test_group<test_geometry_normalize_data> group;
typedef group::object object;

group test_geometry_normalize_data("geos::geom::Geometry::normalize");

//
// Test Cases
//

template<>
template<>
void object::test<1>
()
{
    const char* inp = "POINT (0 100)";
    const char* exp = inp; // point does not normalize to anything else
    runTest(inp, exp);
}

template<>
template<>
void object::test<2>
()
{
    const char* inp =
        "LINESTRING(100 0,100 100,0 100,0 0)";
    const char* exp =
        "LINESTRING (0 0, 0 100, 100 100, 100 0)";
    runTest(inp, exp);
}

template<>
template<>
void object::test<3>
()
{
    const char* inp =
        "POLYGON ("
        "(0 0,100 0,100 100,0 100,0 0)," // CCW
        "(10 10,20 10,20 20,10 20,10 10)," // CCW, leftmost
        "(40 10,40 20,60 20,60 10,40 10)" // CW, rightmost
        ")" ;
    const char* exp =
        "POLYGON("
        "(0 0,0 100,100 100,100 0,0 0)," // CW
        "(40 10,60 10,60 20,40 20,40 10)," // CCW, rightmost
        "(10 10,20 10,20 20,10 20,10 10)" // CCW, leftmost
        ")";

    runTest(inp, exp);
}

template<>
template<>
void object::test<4>
()
{
    const char* inp =
        "MULTIPOINT ("
        "0 100," // leftmost
        "5 6"    // rightmost
        ")";
    const char* exp =
        "MULTIPOINT ("
        "5 6,"   // rightmost
        "0 100"  // leftmost
        ")";
    runTest(inp, exp);
}

template<>
template<>
void object::test<5>
()
{
    const char* inp =
        "MULTILINESTRING("
        "(15 25, 25 52),"               // rightmost vertex @ 25
        "(0 0, 0 100, 100 100, 100 0)"  // rightmost vertex @ 100
        ")";
    const char* exp =
        "MULTILINESTRING("
        "(0 0, 0 100, 100 100, 100 0)," // rightmost vertex @ 100
        "(15 25, 25 52)"                // rightmost vertex @ 25
        ")";
    runTest(inp, exp);
}

template<>
template<>
void object::test<6>
()
{
    const char* inp =
        "MULTIPOLYGON("
        "((0 0, 0 1, 1 1, 1 0, 0 0))," // leftmost
        "((2 0, 2 1, 3 1, 3 0, 2 0))"  // rightmost
        ")";
    const char* exp =
        "MULTIPOLYGON("
        "((2 0, 2 1, 3 1, 3 0, 2 0))," // rightmost
        "((0 0, 0 1, 1 1, 1 0, 0 0))"  // leftmost
        ")";
    runTest(inp, exp);
}

template<>
template<>
void object::test<7>
()
{
    const char* inp =
        "GEOMETRYCOLLECTION("
        "MULTIPOINT ("
        "0 100," // leftmost
        "5 6"    // rightmost
        "),"
        "POINT(10 4)," // more on the right than the multipoint
        "MULTILINESTRING("
        "(15 25, 25 52),"               // rightmost vertex @ 25
        "(0 0, 0 100, 100 100, 100 0)"  // rightmost vertex @ 100
        "),"
        "LINESTRING(100 0,100 100,0 100,0 0),"
        "MULTIPOLYGON("
        "((0 0, 0 1, 1 1, 1 0, 0 0))," // leftmost
        "((2 0, 2 1, 3 1, 3 0, 2 0))"  // rightmost
        "),"
        "POLYGON ("
        "(0 0,100 0,100 100,0 100,0 0)," // CCW
        "(10 10,20 10,20 20,10 20,10 10)," // CCW, leftmost
        "(40 10,40 20,60 20,60 10,40 10)" // CW, rightmost
        ")"
        ")";
    const char* exp =
        "GEOMETRYCOLLECTION("
        "MULTIPOLYGON("
        "((2 0, 2 1, 3 1, 3 0, 2 0))," // rightmost
        "((0 0, 0 1, 1 1, 1 0, 0 0))"  // leftmost
        "),"
        "POLYGON("
        "(0 0,0 100,100 100,100 0,0 0)," // CW
        "(40 10,60 10,60 20,40 20,40 10)," // CCW, rightmost
        "(10 10,20 10,20 20,10 20,10 10)" // CCW, leftmost
        "),"
        "MULTILINESTRING("
        "(0 0, 0 100, 100 100, 100 0)," // rightmost vertex @ 100
        "(15 25, 25 52)"                // rightmost vertex @ 25
        "),"
        "LINESTRING (0 0, 0 100, 100 100, 100 0),"
        "MULTIPOINT ("
        "5 6,"   // rightmost
        "0 100"  // leftmost
        "),"
        "POINT(10 4)" // more on the right than the multipoint
        ")";
    runTest(inp, exp);
}


} // namespace tut

