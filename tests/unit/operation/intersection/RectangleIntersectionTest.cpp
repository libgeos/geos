//
// Test Suite for geos::operation::intersection::RectangleIntersection class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/operation/intersection/Rectangle.h>
#include <geos/operation/intersection/RectangleIntersection.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Point.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
// std
#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace tut
{
    //
    // Test Group
    //

    // Common data used by tests
    struct test_rectangleintersectiontest_data
    {
        geos::io::WKTReader wktreader;
        geos::io::WKTWriter wktwriter;

        typedef geos::geom::Geometry::Ptr GeomPtr;
        typedef geos::geom::Geometry Geom;
        typedef geos::operation::intersection::Rectangle Rectangle;
        typedef geos::operation::intersection::RectangleIntersection RectangleIntersection;

        test_rectangleintersectiontest_data()
          : wktreader()
        {
          wktwriter.setTrim(true);
        }

        GeomPtr readWKT(const std::string& inputWKT)
        {
            return GeomPtr(wktreader.read(inputWKT));
        }

        static GeomPtr normalize(const Geom& g)
        {
          GeomPtr g2 ( g.clone() );
          g2->normalize();
          return g2;
        }

        bool isEqual(const Geom& a, const Geom& b, double tolerance=0)
        {
          using std::cout;
          using std::endl;
          bool eq;
          // TODO: use HausdorffDistance ?
          GeomPtr a2 = normalize(a);
          GeomPtr b2 = normalize(b);
          eq = a2->equalsExact(b2.get(), tolerance);
          if  ( ! eq ) {
            cout << "OBTAINED: " << wktwriter.write(b2.get()) << endl;
          }
          return eq;
        }

        void doLineClipTest(const char* inputWKT, const std::string& expectedWKT, const Rectangle& rect, double tolerance=0)
        {
          GeomPtr g = readWKT(inputWKT);
          ensure(g.get());
          GeomPtr obt = RectangleIntersection::clipBoundary(*g,rect);
          ensure(obt.get());
          bool ok = isEqual(*readWKT(expectedWKT), *obt, tolerance);
          ensure(ok);
        }

        void doClipTest(const char* inputWKT, const std::string& expectedWKT, const Rectangle& rect, double tolerance=0)
        {
          GeomPtr g = readWKT(inputWKT);
          ensure(g.get());
          GeomPtr obt = RectangleIntersection::clip(*g,rect);
          ensure(obt.get());
          bool ok = isEqual(*readWKT(expectedWKT), *obt, tolerance);
          ensure(ok);
// Compare with GEOSIntersection output
#if 0
          GeomPtr g2 ( rect.toPolygon(*g->getFactory()) );
          obt.reset(g->intersection(g2.get()));
          ensure(obt.get());
          ok = isEqual(*readWKT(expectedWKT), *obt, tolerance);
          ensure(ok);
#endif
        }

    };

    typedef test_group<test_rectangleintersectiontest_data, 255> group;
    typedef group::object object;

    group test_rectangleintersectiontest_group("geos::operation::intersection::RectangleIntersection");

    // inside
    template<> template<> void object::test<1>()
    {
      doLineClipTest(
        "LINESTRING (1 1,1 9,9 9,9 1)",
        "LINESTRING (1 1,1 9,9 9,9 1)",
        Rectangle(0,0,10,10)
      );
    }

    // outside
    template<> template<> void object::test<2>()
    {
      doLineClipTest(
        "LINESTRING (-1 -9,-1 11,9 11)",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // go in from left
    template<> template<> void object::test<3>()
    {
      doLineClipTest(
        "LINESTRING (-1 5,5 5,9 9)",
        "LINESTRING (0 5,5 5,9 9)",
        Rectangle(0,0,10,10)
      );
    }

    // go out from right
    template<> template<> void object::test<4>()
    {
      doLineClipTest(
        "LINESTRING (5 5,8 5,12 5)",
        "LINESTRING (5 5,8 5,10 5)",
        Rectangle(0,0,10,10)
      );
    }

    // go in and out
    template<> template<> void object::test<5>()
    {
      doLineClipTest(
        "LINESTRING (5 -1,5 5,1 2,-3 2,1 6)",
        "MULTILINESTRING ((5 0,5 5,1 2,0 2),(0 5,1 6))",
        Rectangle(0,0,10,10)
      );
    }

    // go along left edge
    template<> template<> void object::test<6>()
    {
      doLineClipTest(
        "LINESTRING (0 3,0 5,0 7)",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // go out from left edge
    template<> template<> void object::test<7>()
    {
      doLineClipTest(
        "LINESTRING (0 3,0 5,-1 7)",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // go in from left edge
    template<> template<> void object::test<8>()
    {
      doLineClipTest(
        "LINESTRING (0 3,0 5,2 7)",
        "LINESTRING (0 5,2 7)",
        Rectangle(0,0,10,10)
      );
    }

    // triangle corner at bottom left corner
    template<> template<> void object::test<9>()
    {
      doLineClipTest(
        "LINESTRING (2 1,0 0,1 2)",
        "LINESTRING (2 1,0 0,1 2)",
        Rectangle(0,0,10,10)
      );
    }

    // go from in to edge and back in
    template<> template<> void object::test<10>()
    {
      doLineClipTest(
        "LINESTRING (3 3,0 3,0 5,2 7)",
        "MULTILINESTRING ((3 3,0 3),(0 5,2 7))",
        Rectangle(0,0,10,10)
      );
    }

    // go from in to edge and then straight out
    template<> template<> void object::test<11>()
    {
      doLineClipTest(
        "LINESTRING (5 5,10 5,20 5)",
        "LINESTRING (5 5,10 5)",
        Rectangle(0,0,10,10)
      );
    }

    // triangle corner at left edge
    template<> template<> void object::test<12>()
    {
      doLineClipTest(
        "LINESTRING (3 3,0 6,3 9)",
        "LINESTRING (3 3,0 6,3 9)",
        Rectangle(0,0,10,10)
      );
    }

    // polygon completely inside
    template<> template<> void object::test<13>()
    {
      doLineClipTest(
        "POLYGON ((5 5,5 6,6 6,6 5,5 5))",
        "POLYGON ((5 5,5 6,6 6,6 5,5 5))",
        Rectangle(0,0,10,10)
      );
    }

    // polygon completely outside
    template<> template<> void object::test<14>()
    {
      doLineClipTest(
        "POLYGON ((15 15,15 16,16 16,16 15,15 15))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // polygon surrounds the rectangle
    template<> template<> void object::test<15>()
    {
      doLineClipTest(
        "POLYGON ((-1 -1,-1 11,11 11,11 -1,-1 -1))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // polygon cuts the rectangle
    template<> template<> void object::test<16>()
    {
      doLineClipTest(
        "POLYGON ((-1 -1,-1 5,5 5,5 -1,-1 -1))",
        "LINESTRING (0 5,5 5,5 0)",
        Rectangle(0,0,10,10)
      );
    }

    // polygon with hole cuts the rectangle
    template<> template<> void object::test<17>()
    {
      doLineClipTest(
        "POLYGON ((-2 -2,-2 5,5 5,5 -2,-2 -2), (3 3,4 4,4 2,3 3))",
        "GEOMETRYCOLLECTION (POLYGON ((3 3,4 4,4 2,3 3)),LINESTRING (0 5,5 5,5 0))",
        Rectangle(0,0,10,10)
      );
    }

    // rectangle cuts both the polygon and the hole
    template<> template<> void object::test<18>()
    {
      doLineClipTest(
        "POLYGON ((-2 -2,-2 5,5 5,5 -2,-2 -2), (-1 -1,3 1,3 3,-1 -1))",
        "MULTILINESTRING ((0 5,5 5,5 0),(1 0,3 1,3 3,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    // Triangle at two corners and one edge
    template<> template<> void object::test<19>()
    {
      doLineClipTest(
        "POLYGON ((0 0,10 0,5 10,0 0))",
        "LINESTRING (10 0,5 10,0 0)",
        Rectangle(0,0,10,10)
      );
    }

    // Same triangle with another starting point
    template<> template<> void object::test<20>()
    {
      doLineClipTest(
        "POLYGON ((5 10,0 0,10 0,5 10))",
        "LINESTRING (10 0,5 10,0 0)",
        Rectangle(0,0,10,10)
      );
    }

    // Triangle intersection at corner and edge
    template<> template<> void object::test<21>()
    {
      doLineClipTest(
        "POLYGON ((-5 -5,5 5,5 -5,-5 -5))",
        "LINESTRING (0 0,5 5,5 0)",
        Rectangle(0,0,10,10)
      );
    }

    // Triangle intersection at adjacent edges
    template<> template<> void object::test<22>()
    {
      doLineClipTest(
        "POLYGON ((-1 5,5 11,5 5,-1 5))",
        "MULTILINESTRING ((0 6,4 10),(5 10,5 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    // One triangle intersection and one inside edge
    template<> template<> void object::test<23>()
    {
      doLineClipTest(
        "POLYGON ((-5 5,5 10,5 5,-5 5))",
        "LINESTRING (0.0 7.5,5 10,5 5,0 5)",
        Rectangle(0,0,10,10)
      );
    }

    // Triangle intersection at center and end of the same edge
    template<> template<> void object::test<24>()
    {
      doLineClipTest(
        "POLYGON ((-10 5,10 10,10 5,-10 5))",
        "MULTILINESTRING ((0.0 7.5,10 10),(10 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    // Two different edges clips
    template<> template<> void object::test<25>()
    {
      doLineClipTest(
        "POLYGON ((-5 5,15 15,15 5,-5 5))",
        "MULTILINESTRING ((0.0 7.5,5 10),(10 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    // Inside triangle with all corners at edges
    template<> template<> void object::test<26>()
    {
      doLineClipTest(
        "POLYGON ((0 5,5 10,10 5,0 5))",
        "POLYGON ((0 5,5 10,10 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    // Inside triangle whose base is one of the edges
    template<> template<> void object::test<27>()
    {
      doLineClipTest(
        "POLYGON ((0 0,5 5,10 0,0 0))",
        "LINESTRING (0 0,5 5,10 0)",
        Rectangle(0,0,10,10)
      );
    }

    // Triangle touching two corners on the outside
    template<> template<> void object::test<28>()
    {
      doLineClipTest(
        "POLYGON ((-5 5,5 15,15 5,-5 5))",
        "LINESTRING (10 5,0 5)",
        Rectangle(0,0,10,10)
      );
    }

    // Triangle with a diagonal and sharing two edges
    template<> template<> void object::test<29>()
    {
      doLineClipTest(
        "POLYGON ((0 0,10 10,10 0,0 0))",
        "LINESTRING (0 0,10 10)",
        Rectangle(0,0,10,10)
      );
    }

    // Triangle exits edge at a corner
    template<> template<> void object::test<30>()
    {
      doLineClipTest(
        "POLYGON ((-5 0,5 10,5 0,-5 0))",
        "LINESTRING (0 5,5 10,5 0)",
        Rectangle(0,0,10,10)
      );
    }

    // Triangle enters edge at a corner
    template<> template<> void object::test<31>()
    {
      doLineClipTest(
        "POLYGON ((-5 10,5 10,0 0,-5 10))",
        "LINESTRING (5 10,0 0)",
        Rectangle(0,0,10,10)
      );
    }

    // Triangle enters and exits the same edge
    template<> template<> void object::test<32>()
    {
      doLineClipTest(
        "POLYGON ((-5 0,5 10,15 0,-5 0))",
        "LINESTRING (0 5,5 10,10 5)",
        Rectangle(0,0,10,10)
      );
    }

    // Triangle enters at a corner and exits at another
    template<> template<> void object::test<33>()
    {
      doLineClipTest(
        "POLYGON ((-5 -5,15 15,15 -5,-5 -5))",
        "LINESTRING (0 0,10 10)",
        Rectangle(0,0,10,10)
      );
    }

    // From outside to nearest edge etc
    template<> template<> void object::test<34>()
    {
      doLineClipTest(
        "POLYGON ((-5 -5,0 5,5 0,-5 -5))",
        "LINESTRING (0 5,5 0)",
        Rectangle(0,0,10,10)
      );
    }

    // From outside to opposite edge etc
    template<> template<> void object::test<35>()
    {
      doLineClipTest(
        "POLYGON ((-10 5,10 5,0 -5,-10 5))",
        "LINESTRING (0 5,10 5,5 0)",
        Rectangle(0,0,10,10)
      );
    }

    // Drew all combinations I could imagine on paper, and added the following.
    // All triangles fully inside
    template<> template<> void object::test<36>()
    {
      doLineClipTest(
        "POLYGON ((0 0,0 10,10 10,0 0))",
        "LINESTRING (10 10,0 0)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<37>()
    {
      doLineClipTest(
        "POLYGON ((0 5,0 10,10 10,0 5))",
        "LINESTRING (10 10,0 5)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<38>()
    {
      doLineClipTest(
        "POLYGON ((0 10,10 10,5 0,0 10))",
        "LINESTRING (10 10,5 0,0 10)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<39>()
    {
      doLineClipTest(
        "POLYGON ((0 10,10 10,5 5,0 10))",
        "LINESTRING (10 10,5 5,0 10)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<40>()
    {
      doLineClipTest(
        "POLYGON ((0 10,5 10,0 5,0 10))",
        "LINESTRING (5 10,0 5)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<41>()
    {
      doLineClipTest(
        "POLYGON ((0 10,10 5,0 5,0 10))",
        "LINESTRING (0 10,10 5,0 5)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<42>()
    {
      doLineClipTest(
        "POLYGON ((0 10,10 0,0 5,0 10))",
        "LINESTRING (0 10,10 0,0 5)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<43>()
    {
      doLineClipTest(
        "POLYGON ((0 10,5 0,0 5,0 10))",
        "LINESTRING (0 10,5 0,0 5)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<44>()
    {
      doLineClipTest(
        "POLYGON ((0 10,5 5,0 5,0 10))",
        "LINESTRING (0 10,5 5,0 5)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<45>()
    {
      doLineClipTest(
        "POLYGON ((0 10,7 7,3 3,0 10))",
        "POLYGON ((0 10,7 7,3 3,0 10))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<46>()
    {
      doLineClipTest(
        "POLYGON ((0 10,5 5,5 0,0 10))",
        "POLYGON ((0 10,5 5,5 0,0 10))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<47>()
    {
      doLineClipTest(
        "POLYGON ((0 10,10 5,5 0,0 10))",
        "POLYGON ((0 10,10 5,5 0,0 10))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<48>()
    {
      doLineClipTest(
        "POLYGON ((2 5,5 7,7 5,2 5))",
        "POLYGON ((2 5,5 7,7 5,2 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<49>()
    {
      doLineClipTest(
        "POLYGON ((2 5,5 10,7 5,2 5))",
        "POLYGON ((2 5,5 10,7 5,2 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<50>()
    {
      doLineClipTest(
        "POLYGON ((0 5,5 10,5 5,0 5))",
        "POLYGON ((0 5,5 10,5 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<51>()
    {
      doLineClipTest(
        "POLYGON ((0 5,5 10,10 5,0 5))",
        "POLYGON ((0 5,5 10,10 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<52>()
    {
      doLineClipTest(
        "POLYGON ((0 5,5 7,10 5,0 5))",
        "POLYGON ((0 5,5 7,10 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    // No points inside, one intersection
    template<> template<> void object::test<53>()
    {
      doLineClipTest(
        "POLYGON ((-5 10,0 15,0 10,-5 10))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<54>()
    {
      doLineClipTest(
        "POLYGON ((-5 10,0 5,-5 0,-5 10))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // No points inside, two intersections
    template<> template<> void object::test<55>()
    {
      doLineClipTest(
        "POLYGON ((-5 5,0 10,0 0,-5 5))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<56>()
    {
      doLineClipTest(
        "POLYGON ((-5 5,0 10,0 5,-5 5))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<57>()
    {
      doLineClipTest(
        "POLYGON ((-5 5,0 7,0 3,-5 5))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // One point inside
    template<> template<> void object::test<58>()
    {
      doLineClipTest(
        "POLYGON ((5 5,-5 0,-5 10,5 5))",
        "LINESTRING (0.0 7.5,5 5,0.0 2.5)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<59>()
    {
      doLineClipTest(
        "POLYGON ((5 0,-5 0,-5 10,5 0))",
        "LINESTRING (0 5,5 0)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<60>()
    {
      doLineClipTest(
        "POLYGON((10 0,-10 0,-10 10,10 0))",
        "LINESTRING (0 5,10 0)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<61>()
    {
      doLineClipTest(
        "POLYGON ((5 0,-5 5,-5 10,5 0))",
        "LINESTRING (0 5,5 0,0.0 2.5)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<62>()
    {
      doLineClipTest(
        "POLYGON ((10 5,-10 0,-10 10,10 5))",
        "LINESTRING (0.0 7.5,10 5,0.0 2.5)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<63>()
    {
      doLineClipTest(
        "POLYGON ((10 10,-10 0,-10 5,10 10))",
        "LINESTRING (0.0 7.5,10 10,0 5)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<64>()
    {
      doLineClipTest(
        "POLYGON ((5 5,-5 -5,-5 15,5 5))",
        "LINESTRING (0 10,5 5,0 0)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<65>()
    {
      doLineClipTest(
        "POLYGON ((10 5,-10 -5,-10 15,10 5))",
        "LINESTRING (0 10,10 5,0 0)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<66>()
    {
      doLineClipTest(
        "POLYGON ((5 0,-5 0,-5 20,5 0))",
        "LINESTRING (0 10,5 0)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<67>()
    {
      doLineClipTest(
        "POLYGON ((10 0,-10 0,-10 20,10 0))",
        "LINESTRING (0 10,10 0)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<68>()
    {
      doLineClipTest(
        "POLYGON ((5 5,-10 5,0 15,5 5))",
        "LINESTRING (2.5 10.0,5 5,0 5)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<69>()
    {
      doLineClipTest(
        "POLYGON ((5 5,-5 -5,0 15,5 5))",
        "LINESTRING (2.5 10.0,5 5,0 0)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<70>()
    {
      doLineClipTest(
        "POLYGON ((5 5,-15 -20,-15 30,5 5))",
        "LINESTRING (1 10,5 5,1 0)",
        Rectangle(0,0,10,10)
      );
    }

    // Two points inside
    template<> template<> void object::test<71>()
    {
      doLineClipTest(
        "POLYGON ((5 7,5 3,-5 5,5 7))",
        "LINESTRING (0 6,5 7,5 3,0 4)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<72>()
    {
      doLineClipTest(
        "POLYGON ((5 7,5 3,-5 13,5 7))",
        "LINESTRING (0 10,5 7,5 3,0 8)",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<73>()
    {
      doLineClipTest(
        "POLYGON ((6 6,4 4,-4 14,6 6))",
        "LINESTRING (1.0 10.0,6 6,4 4,0 9)",
        Rectangle(0,0,10,10), 1e-12
      );
    }

    // Polygon with hole which surrounds the rectangle
    template<> template<> void object::test<74>()
    {
      doLineClipTest(
        "POLYGON ((-2 -2,-2 12,12 12,12 -2,-2 -2),(-1 -1,11 -1,11 11,-1 11,-1 -1))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // Polygon surrounding the rect, but with a hole inside the rect
    template<> template<> void object::test<75>()
    {
      doLineClipTest(
        "POLYGON ((-2 -2,-2 12,12 12,12 -2,-2 -2),(1 1,9 1,9 9,1 9,1 1))",
        "POLYGON ((1 1,9 1,9 9,1 9,1 1))",
        Rectangle(0,0,10,10)
      );
    }


    // inside
    template<> template<> void object::test<76>()
    {
      doClipTest(
        "LINESTRING (1 1,1 9,9 9,9 1)",
        "LINESTRING (1 1,1 9,9 9,9 1)",
        Rectangle(0,0,10,10)
      );
    }

    // outside
    template<> template<> void object::test<77>()
    {
      doClipTest(
        "LINESTRING (-1 -9,-1 11,9 11)",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // go in from left
    template<> template<> void object::test<78>()
    {
      doClipTest(
        "LINESTRING (-1 5,5 5,9 9)",
        "LINESTRING (0 5,5 5,9 9)",
        Rectangle(0,0,10,10)
      );
    }

    // go out from right
    template<> template<> void object::test<79>()
    {
      doClipTest(
        "LINESTRING (5 5,8 5,12 5)",
        "LINESTRING (5 5,8 5,10 5)",
        Rectangle(0,0,10,10)
      );
    }

    // go in and out
    template<> template<> void object::test<80>()
    {
      doClipTest(
        "LINESTRING (5 -1,5 5,1 2,-3 2,1 6)",
        "MULTILINESTRING ((5 0,5 5,1 2,0 2),(0 5,1 6))",
        Rectangle(0,0,10,10)
      );
    }

    // go along left edge
    template<> template<> void object::test<81>()
    {
      doClipTest(
        "LINESTRING (0 3,0 5,0 7)",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // go out from left edge
    template<> template<> void object::test<82>()
    {
      doClipTest(
        "LINESTRING (0 3,0 5,-1 7)",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // go in from left edge
    template<> template<> void object::test<83>()
    {
      doClipTest(
        "LINESTRING (0 3,0 5,2 7)",
        "LINESTRING (0 5,2 7)",
        Rectangle(0,0,10,10)
      );
    }

    // triangle corner at bottom left corner
    template<> template<> void object::test<84>()
    {
      doClipTest(
        "LINESTRING (2 1,0 0,1 2)",
        "LINESTRING (2 1,0 0,1 2)",
        Rectangle(0,0,10,10)
      );
    }

    // go from in to edge and back in
    template<> template<> void object::test<85>()
    {
      doClipTest(
        "LINESTRING (3 3,0 3,0 5,2 7)",
        "MULTILINESTRING ((3 3,0 3),(0 5,2 7))",
        Rectangle(0,0,10,10)
      );
    }

    // go from in to edge and then straight out
    template<> template<> void object::test<86>()
    {
      doClipTest(
        "LINESTRING (5 5,10 5,20 5)",
        "LINESTRING (5 5,10 5)",
        Rectangle(0,0,10,10)
      );
    }

    // triangle corner at left edge
    template<> template<> void object::test<87>()
    {
      doClipTest(
        "LINESTRING (3 3,0 6,3 9)",
        "LINESTRING (3 3,0 6,3 9)",
        Rectangle(0,0,10,10)
      );
    }

    // polygon completely inside
    template<> template<> void object::test<88>()
    {
      doClipTest(
        "POLYGON ((5 5,5 6,6 6,6 5,5 5))",
        "POLYGON ((5 5,5 6,6 6,6 5,5 5))",
        Rectangle(0,0,10,10)
      );
    }

    // polygon completely outside
    template<> template<> void object::test<89>()
    {
      doClipTest(
        "POLYGON ((15 15,15 16,16 16,16 15,15 15))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // polygon surrounds the rectangle
    template<> template<> void object::test<90>()
    {
      doClipTest(
        "POLYGON ((-1 -1,-1 11,11 11,11 -1,-1 -1))",
        "POLYGON ((0 0,0 10,10 10,10 0,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    // polygon cuts the rectangle
    template<> template<> void object::test<91>()
    {
      doClipTest(
        "POLYGON ((-1 -1,-1 5,5 5,5 -1,-1 -1))",
        "POLYGON ((0 0,0 5,5 5,5 0,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    // polygon with hole cuts the rectangle
    template<> template<> void object::test<92>()
    {
      doClipTest(
        "POLYGON ((-2 -2,-2 5,5 5,5 -2,-2 -2), (3 3,4 4,4 2,3 3))",
        "POLYGON ((0 0,0 5,5 5,5 0,0 0),(3 3,4 4,4 2,3 3))",
        Rectangle(0,0,10,10)
      );
    }

    // rectangle cuts both the polygon and the hole
    template<> template<> void object::test<93>()
    {
      doClipTest(
        "POLYGON ((-2 -2,-2 5,5 5,5 -2,-2 -2), (-1 -1,3 1,3 3,-1 -1))",
        "POLYGON ((0 0,0 5,5 5,5 0,1 0,3 1,3 3,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    // Triangle at two corners and one edge
    template<> template<> void object::test<94>()
    {
      doClipTest(
        "POLYGON ((0 0,10 0,5 10,0 0))",
        "POLYGON ((0 0,10 0,5 10,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    // Same triangle with another starting point
    template<> template<> void object::test<95>()
    {
      doClipTest(
        "POLYGON ((5 10,0 0,10 0,5 10))",
        "POLYGON ((0 0,10 0,5 10,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    // Triangle intersection at corner and edge
    template<> template<> void object::test<96>()
    {
      doClipTest(
        "POLYGON ((-5 -5,5 5,5 -5,-5 -5))",
        "POLYGON ((0 0,5 5,5 0,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    // All triangles fully inside
    template<> template<> void object::test<97>()
    {
      doClipTest(
        "POLYGON ((0 0,0 10,10 10,0 0))",
        "POLYGON ((0 0,0 10,10 10,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<98>()
    {
      doClipTest(
        "POLYGON ((0 5,0 10,10 10,0 5))",
        "POLYGON ((0 5,0 10,10 10,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<99>()
    {
      doClipTest(
        "POLYGON ((0 10,10 10,5 0,0 10))",
        "POLYGON ((0 10,10 10,5 0,0 10))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<100>()
    {
      doClipTest(
        "POLYGON ((0 10,10 10,5 5,0 10))",
        "POLYGON ((0 10,10 10,5 5,0 10))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<101>()
    {
      doClipTest(
        "POLYGON ((0 10,5 10,0 5,0 10))",
        "POLYGON ((0 5,0 10,5 10,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<102>()
    {
      doClipTest(
        "POLYGON ((0 10,10 5,0 5,0 10))",
        "POLYGON ((0 5,0 10,10 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<103>()
    {
      doClipTest(
        "POLYGON ((0 10,10 0,0 5,0 10))",
        "POLYGON ((0 5,0 10,10 0,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<104>()
    {
      doClipTest(
        "POLYGON ((0 10,5 0,0 5,0 10))",
        "POLYGON ((0 5,0 10,5 0,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<105>()
    {
      doClipTest(
        "POLYGON ((0 10,5 5,0 5,0 10))",
        "POLYGON ((0 5,0 10,5 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<106>()
    {
      doClipTest(
        "POLYGON ((0 10,7 7,3 3,0 10))",
        "POLYGON ((0 10,7 7,3 3,0 10))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<107>()
    {
      doClipTest(
        "POLYGON ((0 10,5 5,5 0,0 10))",
        "POLYGON ((0 10,5 5,5 0,0 10))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<108>()
    {
      doClipTest(
        "POLYGON ((0 10,10 5,5 0,0 10))",
        "POLYGON ((0 10,10 5,5 0,0 10))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<109>()
    {
      doClipTest(
        "POLYGON ((2 5,5 7,7 5,2 5))",
        "POLYGON ((2 5,5 7,7 5,2 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<110>()
    {
      doClipTest(
        "POLYGON ((2 5,5 10,7 5,2 5))",
        "POLYGON ((2 5,5 10,7 5,2 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<111>()
    {
      doClipTest(
        "POLYGON ((0 5,5 10,5 5,0 5))",
        "POLYGON ((0 5,5 10,5 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<112>()
    {
      doClipTest(
        "POLYGON ((0 5,5 10,10 5,0 5))",
        "POLYGON ((0 5,5 10,10 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<113>()
    {
      doClipTest(
        "POLYGON ((0 5,5 7,10 5,0 5))",
        "POLYGON ((0 5,5 7,10 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    // No points inside, one intersection
    template<> template<> void object::test<114>()
    {
      doClipTest(
        "POLYGON ((-5 10,0 15,0 10,-5 10))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<115>()
    {
      doClipTest(
        "POLYGON ((-5 10,0 5,-5 0,-5 10))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // No points inside, two intersections
    template<> template<> void object::test<116>()
    {
      doClipTest(
        "POLYGON ((-5 5,0 10,0 0,-5 5))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<117>()
    {
      doClipTest(
        "POLYGON ((-5 5,0 10,0 5,-5 5))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<118>()
    {
      doClipTest(
        "POLYGON ((-5 5,0 7,0 3,-5 5))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // One point inside
    template<> template<> void object::test<119>()
    {
      doClipTest(
        "POLYGON ((5 5,-5 0,-5 10,5 5))",
        "POLYGON ((0.0 2.5,0.0 7.5,5 5,0.0 2.5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<120>()
    {
      doClipTest(
        "POLYGON ((5 0,-5 0,-5 10,5 0))",
        "POLYGON ((0 0,0 5,5 0,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<121>()
    {
      doClipTest(
        "POLYGON ((10 0,-10 0,-10 10,10 0))",
        "POLYGON ((0 0,0 5,10 0,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<122>()
    {
      doClipTest(
        "POLYGON ((5 0,-5 5,-5 10,5 0))",
        "POLYGON ((0.0 2.5,0 5,5 0,0.0 2.5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<123>()
    {
      doClipTest(
        "POLYGON ((10 5,-10 0,-10 10,10 5))",
        "POLYGON ((0.0 2.5,0.0 7.5,10 5,0.0 2.5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<124>()
    {
      doClipTest(
        "POLYGON ((10 10,-10 0,-10 5,10 10))",
        "POLYGON ((0 5,0.0 7.5,10 10,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<125>()
    {
      doClipTest(
        "POLYGON ((5 5,-5 -5,-5 15,5 5))",
        "POLYGON ((0 0,0 10,5 5,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<126>()
    {
      doClipTest(
        "POLYGON ((10 5,-10 -5,-10 15,10 5))",
        "POLYGON ((0 0,0 10,10 5,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<127>()
    {
      doClipTest(
        "POLYGON ((5 0,-5 0,-5 20,5 0))",
        "POLYGON ((0 0,0 10,5 0,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<128>()
    {
      doClipTest(
        "POLYGON ((10 0,-10 0,-10 20,10 0))",
        "POLYGON ((0 0,0 10,10 0,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<129>()
    {
      doClipTest(
        "POLYGON ((5 5,-10 5,0 15,5 5))",
        "POLYGON ((0 5,0 10,2.5 10.0,5 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<130>()
    {
      doClipTest(
        "POLYGON ((5 5,-5 -5,0 15,5 5))",
        "POLYGON ((0 0,0 10,2.5 10.0,5 5,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<131>()
    {
      doClipTest(
        "POLYGON ((5 5,-15 -20,-15 30,5 5))",
        "POLYGON ((0 0,0 10,1 10,5 5,1 0,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    // Two points inside
    template<> template<> void object::test<132>()
    {
      doClipTest(
        "POLYGON ((5 7,5 3,-5 5,5 7))",
        "POLYGON ((0 4,0 6,5 7,5 3,0 4))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<133>()
    {
      doClipTest(
        "POLYGON ((5 7,5 3,-5 13,5 7))",
        "POLYGON ((0 8,0 10,5 7,5 3,0 8))",
        Rectangle(0,0,10,10)
      );
    }

    template<> template<> void object::test<134>()
    {
      doClipTest(
        "POLYGON ((6 6,4 4,-4 14,6 6))",
        "POLYGON ((0 9,0 10,1.0 10.0,6 6,4 4,0 9))",
        Rectangle(0,0,10,10), 1e-12
      );
    }

    // Polygon with hole which surrounds the rectangle
    template<> template<> void object::test<135>()
    {
      doClipTest(
        "POLYGON ((-2 -2,-2 12,12 12,12 -2,-2 -2),(-1 -1,11 -1,11 11,-1 11,-1 -1))",
        "GEOMETRYCOLLECTION EMPTY",
        Rectangle(0,0,10,10)
      );
    }

    // Polygon surrounding the rect, but with a hole inside the rect
    template<> template<> void object::test<136>()
    {
      doClipTest(
        "POLYGON ((-2 -2,-2 12,12 12,12 -2,-2 -2),(1 1,9 1,9 9,1 9,1 1))",
        "POLYGON ((0 0,0 10,10 10,10 0,0 0),(1 1,9 1,9 9,1 9,1 1))",
        Rectangle(0,0,10,10)
      );
    }

    // Polygon with hole cut at the right corner
    template<> template<> void object::test<137>()
    {
      doClipTest(
        "POLYGON ((5 5,15 5,15 -5,5 -5,5 5),(8 1,8 -1,9 -1,9 1,8 1))",
        "POLYGON ((5 0,5 5,10 5,10 0,9 0,9 1,8 1,8 0,5 0))",
        Rectangle(0,0,10,10)
      );
    }

    // Polygon going around a corner
    template<> template<> void object::test<138>()
    {
      doClipTest(
        "POLYGON ((-6 5,5 5,5 -6,-6 5))",
        "POLYGON ((0 0,0 5,5 5,5 0,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    // Hole in a corner
    template<> template<> void object::test<139>()
    {
      doClipTest(
        "POLYGON ((-15 -15,-15 15,15 15,15 -15,-15 -15),(-5 5,-5 -5,5 -5,5 5,-5 5))",
        "POLYGON ((0 5,0 10,10 10,10 0,5 0,5 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    // Hole going around a corner
    template<> template<> void object::test<140>()
    {
      doClipTest(
        "POLYGON ((-15 -15,-15 15,15 15,15 -15,-15 -15),(-6 5,5 -6,5 5,-6 5))",
        "POLYGON ((0 5,0 10,10 10,10 0,5 0,5 5,0 5))",
        Rectangle(0,0,10,10)
      );
    }

    // Surround the rectangle, hole outside rectangle
    template<> template<> void object::test<141>()
    {
      doClipTest(
        "POLYGON ((-15 -15,-15 15,15 15,15 -15,-15 -15),(-5 5,-6 5,-6 6,-5 6,-5 5))",
        "POLYGON ((0 0,0 10,10 10,10 0,0 0))",
        Rectangle(0,0,10,10)
      );
    }

    // Surround the rectangle, hole outside rectangle but shares edge
    template<> template<> void object::test<142>()
    {
      doClipTest(
        "POLYGON ((-15 -15,-15 15,15 15,15 -15,-15 -15),(0 5,-1 5,-1 6,0 6,0 5))",
        "POLYGON ((0 0,0 10,10 10,10 0,0 0))",
        Rectangle(0,0,10,10)
      );
    }

//-- NEW TESTS HERE

    // Simple overlap, counter-clockwise shell
    // Found in TestFunctionAA.xml case1 test1
    template<> template<> void object::test<204>()
    {
      Rectangle r(10,10,100,100);
      const char *inp =
        "POLYGON ((50 50,200 50,200 200,50 200,50 50))"; // CCW
        //"POLYGON((50 50,50 200,200 200,200 50,50 50))"; // CW
      const char *exp =
        "POLYGON ((50 50, 50 100, 100 100, 100 50, 50 50))";
      doClipTest(inp, exp, r);
    }

    // Clockwise shell, clockwise hole (both clipped)
    template<> template<> void object::test<205>()
    {
      Rectangle r(0,0,10,10);
      const char *inp =
        "POLYGON ("
          "(-10 2,-10 8,8 8,8 2,-10 2)," // CW
          "(-5 6,5 6,5 4,-5 4,-5 6)"     // CW
        ")";
      const char *exp =
        "POLYGON((0 8,8 8, 8 2, 0 2, 0 4, 5 4, 5 6, 0 6, 0 8))";
      doClipTest(inp, exp, r);
    }

    // Counterclockwise shell, clockwise hole (both clipped)
    template<> template<> void object::test<206>()
    {
      Rectangle r(0,0,10,10);
      const char *inp =
        "POLYGON ("
          "(-10 2,8 2,8 8,-10 8,-10 2)," // CCW
          "(-5 6,5 6,5 4,-5 4,-5 6)"     // CW
        ")";
      const char *exp =
        "POLYGON((0 8,8 8, 8 2, 0 2, 0 4, 5 4, 5 6, 0 6, 0 8))";
      doClipTest(inp, exp, r);
    }

    // Hole outside the rect shares the lower-left corner
    template<> template<> void object::test<207>()
    {
      Rectangle r(0,0,10,10);
      const char *inp =
        "POLYGON ("
          "(-10 -10,-10 20,20 20,20 -10,-10 -10)," // CW
          "(-5 -5,0 -5,0 0,-5 0,-5 -5)"     // CCW
        ")";
      const char *exp =
        "POLYGON((0 0,0 10,10 10,10 0,0 0))";
      doClipTest(inp, exp, r);
    }

	
    // PostGIS hanging unit test
    template<> template<> void object::test<207>()
    {
      Rectangle r(0.000000, -20000000.000000, 20000000.000000, -1.000000);
      const char *inp =
        "POLYGON((0 -20000000,0 -1,0.001570795681 -0.9999987663,0.003141587486 -0.999995065202,0.004712371539 -0.999988896716,0.006283143966 -0.999980260856,0.007853900889 -0.999969157645,0.009424638433 -0.999955587109,0.010995352723 -0.999939549282,0.012566039883 -0.999921044204,0.014136696038 -0.99990007192,0.015707317312 -0.999876632482,0.017277899829 -0.999850725947,0.018848439715 -0.999822352381,0.020418933095 -0.999791511852,0.021989376093 -0.999758204437,0.023559764834 -0.999722430218,0.025130095443 -0.999684189283,0.026700364047 -0.999643481727,0.02827056677 -0.99960030765,0.029840699739 -0.999554667159,0.031410759078 -0.999506560366,0.032980740915 -0.999455987389,0.034550641375 -0.999402948355,0.036120456584 -0.999347443393,0.03769018267 -0.999289472641,0.039259815759 -0.999229036241,0.040829351979 -0.999166134343,0.042398787456 -0.999100767101,0.043968118318 -0.999032934678,0.045537340693 -0.998962637241,0.04710645071 -0.998889874962,0.048675444496 -0.998814648022,0.05024431818 -0.998736956606,0.051813067891 -0.998656800906,0.053381689759 -0.99857418112,0.054950179912 -0.998489097451,0.056518534482 -0.998401550109,0.058086749598 -0.998311539311,0.05965482139 -0.998219065278,0.06122274599 -0.998124128239,0.062790519529 -0.998026728428,0.064358138139 -0.997926866086,0.065925597951 -0.997824541457,0.067492895099 -0.997719754796,0.069060025714 -0.997612506361,0.070626985931 -0.997502796416,0.072193771883 -0.997390625232,0.073760379704 -0.997275993086,0.075326805528 -0.997158900261,0.076893045491 -0.997039347045,0.078459095728 -0.996917333733,0.080024952375 -0.996792860627,0.081590611568 -0.996665928034,0.083156069445 -0.996536536267,0.084721322142 -0.996404685645,0.086286365798 -0.996270376493,0.087851196551 -0.996133609143,0.089415810539 -0.995994383933,0.090980203904 -0.995852701205,0.092544372783 -0.99570856131,0.094108313319 -0.995561964603,0.095672021651 -0.995412911446,0.097235493922 -0.995261402206,0.098798726275 -0.995107437258,0.100361714851 -0.994951016981,0.101924455795 -0.994792141762,0.10348694525 -0.994630811991,0.105049179362 -0.994467028068,0.106611154275 -0.994300790397,0.108172866136 -0.994132099387,0.109734311091 -0.993960955455,0.111295485288 -0.993787359023,0.112856384874 -0.99361131052,0.114417005998 -0.99343281038,0.115977344809 -0.993251859042,0.117537397458 -0.993068456955,0.119097160095 -0.99288260457,0.120656628872 -0.992694302346,0.12221579994 -0.992503550747,0.123774669453 -0.992310350244,0.125333233564 -0.992114701314,0.126891488428 -0.99191660444,0.1284494302 -0.991716060111,0.130007055036 -0.99151306882,0.131564359092 -0.99130763107,0.133121338527 -0.991099747366,0.134677989497 -0.990889418222,0.136234308163 -0.990676644158,0.137790290685 -0.990461425697,0.139345933222 -0.990243763371,0.140901231938 -0.990023657717,0.142456182993 -0.989801109278,0.144010782552 -0.989576118603,0.145565026779 -0.989348686247,0.147118911839 -0.989118812772,0.148672433897 -0.988886498745,0.150225589121 -0.988651744738,0.151778373678 -0.988414551331,0.153330783737 -0.98817491911,0.154882815468 -0.987932848666,0.15643446504 -0.987688340595,0.157985728626 -0.987441395502,0.159536602399 -0.987192013995,0.16108708253 -0.98694019669,0.162637165195 -0.986685944208,0.164186846569 -0.986429257176,0.165736122828 -0.986170136229,0.16728499015 -0.985908582005,0.168833444713 -0.985644595149,0.170381482696 -0.985378176313,0.171929100279 -0.985109326155,0.173476293645 -0.984838045337,0.175023058975 -0.984564334529,0.176569392453 -0.984288194407,0.178115290264 -0.984009625651,0.179660748593 -0.98372862895,0.181205763627 -0.983445204995,0.182750331554 -0.983159354488,0.184294448562 -0.982871078132,0.185838110843 -0.98258037664,0.187381314586 -0.982287250729,0.188924055984 -0.981991701121,0.190466331231 -0.981693728546,0.192008136522 -0.98139333374,0.193549468051 -0.981090517443,0.195090322016 -0.980785280403,0.196630694615 -0.980477623373,0.198170582048 -0.980167547112,0.199709980514 -0.979855052384,0.201248886216 -0.979540139962,0.202787295357 -0.979222810622,0.204325204139 -0.978903065147,0.20586260877 -0.978580904325,0.207399505455 -0.978256328953,0.208935890402 -0.977929339831,0.210471759821 -0.977599937765,0.212007109922 -0.977268123568,0.213541936916 -0.97693389806,0.215076237017 -0.976597262064,0.216610006439 -0.976258216411,0.218143241397 -0.975916761939,0.219675938108 -0.975572899489,0.22120809279 -0.975226629909,0.222739701664 -0.974877954055,0.224270760949 -0.974526872787,0.225801266869 -0.97417338697,0.227331215647 -0.973817497477,0.228860603507 -0.973459205187,0.230389426677 -0.973098510982,0.231917681383 -0.972735415754,0.233445363856 -0.972369920398,0.234972470325 -0.972002025815,0.236498997024 -0.971631732915,0.238024940184 -0.971259042609,0.239550296042 -0.970883955819,0.241075060833 -0.970506473469,0.242599230795 -0.97012659649,0.244122802168 -0.969744325821,0.245645771192 -0.969359662404,0.24716813411 -0.968972607188,0.248689887165 -0.968583161129,0.250211026602 -0.968191325187,0.251731548669 -0.967797100329,0.253251449612 -0.967400487528,0.254770725683 -0.967001487762,0.256289373133 -0.966600102017,0.257807388214 -0.966196331282,0.259324767181 -0.965790176553,0.26084150629 -0.965381638833,0.262357601798 -0.96497071913,0.263873049965 -0.964557418458,0.265387847052 -0.964141737836,0.26690198932 -0.96372367829,0.268415473035 -0.963303240852,0.269928294461 -0.962880426559,0.271440449865 -0.962455236454,0.272951935517 -0.962027671586,0.274462747688 -0.961597733011,0.275972882649 -0.961165421789,0.277482336674 -0.960730738987,0.278991106039 -0.960293685677,0.280499187022 -0.959854262938,0.2820065759 -0.959412471854,0.283513268955 -0.958968313515,0.28501926247 -0.958521789017,0.286524552728 -0.958072899462,0.288029136015 -0.957621645958,0.289533008618 -0.957168029617,0.291036166828 -0.956712051559,0.292538606935 -0.956253712909,0.294040325232 -0.955793014798,0.295541318014 -0.955329958363,0.297041581577 -0.954864544747,0.298541112219 -0.954396775097,0.300039906241 -0.953926650567,0.301537959945 -0.953454172319,0.303035269633 -0.952979341517,0.304531831612 -0.952502159334,0.306027642189 -0.952022626946,0.307522697673 -0.951540745537,0.309016994375 -0.951056516295,0.310510528609 -0.950569940416,0.312003296688 -0.950081019101,0.313495294931 -0.949589753555,0.314986519655 -0.94909614499,0.316476967182 -0.948600194626,0.317966633832 -0.948101903684,0.319455515932 -0.947601273395,0.320943609807 -0.947098304995,0.322430911786 -0.946592999723,0.323917418198 -0.946085358828,0.325403125376 -0.94557538356,0.326888029655 -0.94506307518,0.32837212737 -0.94454843495,0.329855414859 -0.944031464141,0.331337888463 -0.943512164028,0.332819544523 -0.942990535893,0.334300379384 -0.942466581022,0.335780389393 -0.941940300709,0.337259570896 -0.941411696251,0.338737920245 -0.940880768954,0.340215433792 -0.940347520127,0.341692107891 -0.939811951086,0.343167938899 -0.939274063153,0.344642923175 -0.938733857654,0.346117057078 -0.938191335922,0.347590336971 -0.937646499297,0.34906275922 -0.937099349122,0.350534320191 -0.936549886748,0.352005016254 -0.93599811353,0.353474843779 -0.93544403083,0.354943799141 -0.934887640014,0.356411878713 -0.934328942457,0.357879078875 -0.933767939535,0.359345396006 -0.933204632634,0.360810826488 -0.932639023143,0.362275366705 -0.932071112458,0.363739013043 -0.931500901981,0.365201761892 -0.930928393117,0.366663609641 -0.93035358728,0.368124552685 -0.929776485888,0.369584587417 -0.929197090365,0.371043710237 -0.928615402141,0.372501917543 -0.92803142265,0.373959205738 -0.927445153335,0.375415571225 -0.92685659564,0.376871010412 -0.926265751019,0.378325519707 -0.925672620929,0.379779095522 -0.925077206834,0.381231734269 -0.924479510204,0.382683432365 -0.923879532511,0.384134186228 -0.923277275238,0.385583992277 -0.92267273987,0.387032846937 -0.922065927899,0.388480746631 -0.921456840821,0.389927687788 -0.920845480141,0.391373666837 -0.920231847366,0.392818680211 -0.91961594401,0.394262724343 -0.918997771593,0.395705795671 -0.918377331641,0.397147890635 -0.917754625684,0.398589005675 -0.917129655259,0.400029137237 -0.916502421907,0.401468281767 -0.915872927177,0.402906435714 -0.915241172621,0.404343595529 -0.914607159799,0.405779757666 -0.913970890274,0.407214918582 -0.913332365617,0.408649074736 -0.912691587404,0.410082222589 -0.912048557214,0.411514358605 -0.911403276635,0.41294547925 -0.91075574726,0.414375580993 -0.910105970685,0.415804660306 -0.909453948514,0.417232713662 -0.908799682356,0.418659737537 -0.908143173825,0.420085728412 -0.907484424541,0.421510682766 -0.90682343613,0.422934597085 -0.906160210221,0.424357467855 -0.905494748453,0.425779291565 -0.904827052466,0.427200064707 -0.904157123908,0.428619783775 -0.903484964433,0.430038445267 -0.902810575698,0.431456045681 -0.902133959368,0.43287258152 -0.901455117112,0.43428804929 -0.900774050605,0.435702445497 -0.900090761528,0.437115766651 -0.899405251566,0.438528009266 -0.898717522412,0.439939169856 -0.898027575761,0.44134924494 -0.897335413316,0.442758231039 -0.896641036785,0.444166124676 -0.895944447882,0.445572922377 -0.895245648325,0.446978620671 -0.894544639838,0.44838321609 -0.893841424151,0.449786705168 -0.893136003,0.451189084442 -0.892428378124,0.452590350452 -0.891718551269,0.45399049974 -0.891006524188,0.455389528851 -0.890292298637,0.456787434334 -0.889575876378,0.45818421274 -0.888857259179,0.459579860621 -0.888136448814,0.460974374535 -0.887413447059,0.462367751041 -0.886688255701,0.4637599867 -0.885960876527,0.465151078077 -0.885231311332,0.466541021741 -0.884499561918,0.467929814261 -0.883765630089,0.46931745221 -0.883029517656,0.470703932165 -0.882291226435,0.472089250705 -0.881550758248,0.473473404412 -0.880808114923,0.474856389871 -0.880063298291,0.476238203668 -0.879316310191,0.477618842395 -0.878567152464,0.478998302645 -0.877815826961,0.480376581014 -0.877062335535,0.481753674102 -0.876306680044,0.48312957851 -0.875548862354,0.484504290844 -0.874788884333,0.485877807712 -0.874026747859,0.487250125725 -0.87326245481,0.488621241497 -0.872496007073,0.489991151644 -0.871727406539,0.491359852787 -0.870956655104,0.492727341548 -0.87018375467,0.494093614554 -0.869408707144,0.495458668432 -0.868631514438,0.496822499816 -0.867852178471,0.498185105339 -0.867070701164,0.499546481641 -0.866287084447,0.500906625361 -0.865501330253,0.502265533143 -0.86471344052,0.503623201636 -0.863923417193,0.504979627488 -0.86313126222,0.506334807353 -0.862336977557,0.507688737887 -0.861540565163,0.50904141575 -0.860742027004,0.510392837604 -0.859941365049,0.511743000114 -0.859138581274,0.51309189995 -0.85833367766,0.514439533782 -0.857526656194,0.515785898285 -0.856717518865,0.517130990138 -0.855906267671,0.518474806022 -0.855092904614,0.519817342621 -0.854277431699,0.521158596622 -0.85345985094,0.522498564716 -0.852640164354,0.523837243597 -0.851818373963,0.525174629961 -0.850994481795,0.52651072051 -0.850168489882,0.527845511945 -0.849340400263,0.529179000974 -0.848510214982,0.530511184307 -0.847677936085,0.531842058656 -0.846843565628,0.533171620737 -0.846007105668,0.534499867271 -0.84516855827,0.535826794979 -0.844327925502,0.537152400588 -0.843485209439,0.538476680827 -0.84264041216,0.539799632428 -0.84179353575,0.541121252127 -0.840944582298,0.542441536663 -0.840093553899,0.543760482779 -0.839240452652,0.54507808722 -0.838385280663,0.546394346734 -0.837528040042,0.547709258075 -0.836668732904,0.549022817998 -0.835807361368,0.550335023262 -0.834943927561,0.551645870628 -0.834078433613,0.552955356864 -0.833210881659,0.554263478737 -0.832341273841,0.55557023302 -0.831469612303,0.556875616488 -0.830595899196,0.558179625922 -0.829720136676,0.559482258102 -0.828842326905,0.560783509816 -0.827962472047,0.562083377852 -0.827080574275,0.563381859003 -0.826196635763,0.564678950066 -0.825310658693,0.56597464784 -0.824422645251,0.567268949127 -0.823532597628,0.568561850734 -0.822640518021,0.569853349472 -0.82174640863,0.571143442153 -0.820850271661,0.572432125595 -0.819952109325,0.573719396617 -0.81905192384,0.575005252043 -0.818149717425,0.576289688702 -0.817245492307,0.577572703422 -0.816339250717,0.57885429304 -0.815430994891,0.580134454392 -0.814520727071,0.58141318432 -0.813608449501,0.582690479669 -0.812694164433,0.583966337286 -0.811777874123,0.585240754026 -0.810859580832,0.586513726741 -0.809939286826,0.587785252292 -0.809016994375,0.589055327542 -0.808092705755,0.590323949356 -0.807166423246,0.591591114605 -0.806238149135,0.592856820161 -0.805307885711,0.594121062902 -0.80437563527,0.595383839708 -0.803441400112,0.596645147464 -0.802505182542,0.597904983058 -0.801566984871,0.59916334338 -0.800626809413,0.600420225326 -0.799684658487,0.601675625795 -0.798740534419,0.602929541689 -0.797794439539,0.604181969914 -0.796846376179,0.605432907381 -0.795896346681,0.606682351002 -0.794944353388,0.607930297695 -0.793990398648,0.60917674438 -0.793034484816,0.610421687982 -0.79207661425,0.611665125429 -0.791116789314,0.612907053653 -0.790155012376,0.61414746959 -0.789191285809,0.615386370179 -0.78822561199,0.616623752364 -0.787257993304,0.61785961309 -0.786288432137,0.61909394931 -0.785316930881,0.620326757977 -0.784343491933,0.621558036049 -0.783368117696,0.622787780488 -0.782390810577,0.624015988261 -0.781411572985,0.625242656336 -0.780430407338,0.626467781687 -0.779447316057,0.627691361291 -0.778462301567,0.628913392129 -0.777475366299,0.630133871185 -0.776486512687,0.631352795449 -0.775495743172,0.632570161913 -0.774503060199,0.633785967573 -0.773508466216,0.635000209429 -0.772511963678,0.636212884485 -0.771513555043,0.637423989749 -0.770513242776,0.638633522232 -0.769511029343,0.639841478951 -0.768506917219,0.641047856925 -0.76750090888,0.642252653177 -0.766493006809,0.643455864734 -0.765483213493,0.644657488628 -0.764471531423,0.645857521893 -0.763457963096,0.647055961569 -0.762442511011,0.648252804699 -0.761425177676,0.64944804833 -0.7604059656,0.650641689512 -0.759384877298,0.651833725301 -0.758361915289,0.653024152754 -0.757337082097,0.654212968936 -0.756310380251,0.655400170912 -0.755281812285,0.656585755753 -0.754251380736,0.657769720534 -0.753219088147,0.658952062334 -0.752184937064,0.660132778235 -0.75114893004,0.661311865324 -0.75011106963,0.662489320691 -0.749071358396,0.663665141432 -0.748029798903,0.664839324646 -0.746986393721,0.666011867434 -0.745941145424,0.667182766905 -0.744894056592,0.668352020168 -0.743845129807,0.669519624339 -0.742794367659,0.670685576537 -0.741741772739,0.671849873885 -0.740687347645,0.67301251351 -0.739631094979,0.674173492543 -0.738573017346,0.675332808121 -0.737513117358,0.676490457382 -0.73645139763,0.67764643747 -0.735387860781,0.678800745533 -0.734322509436,0.679953378722 -0.733255346223,0.681104334195 -0.732186373775,0.682253609109 -0.73111559473,0.683401200631 -0.73004301173,0.684547105929 -0.728968627421,0.685691322174 -0.727892444455,0.686833846544 -0.726814465487,0.68797467622 -0.725734693176,0.689113808387 -0.724653130187,0.690251240234 -0.723569779188,0.691386968955 -0.722484642853,0.692520991747 -0.721397723859,0.693653305813 -0.720309024888,0.694783908358 -0.719218548626,0.695912796592 -0.718126297763,0.697039967731 -0.717032274995,0.698165418993 -0.715936483022,0.699289147602 -0.714838924546,0.700411150784 -0.713739602276,0.701531425771 -0.712638518925,0.702649969799 -0.711535677209,0.703766780108 -0.71043107985,0.704881853942 -0.709324729572,0.705995188551 -0.708216629106,0.707106781187 -0.707106781187,0.708216629106 -0.705995188551,0.709324729572 -0.704881853942,0.71043107985 -0.703766780108,0.711535677209 -0.702649969799,0.712638518925 -0.701531425771,0.713739602276 -0.700411150784,0.714838924546 -0.699289147602,0.715936483022 -0.698165418993,0.717032274995 -0.697039967731,0.718126297763 -0.695912796592,0.719218548626 -0.694783908358,0.720309024888 -0.693653305813,0.721397723859 -0.692520991747,0.722484642853 -0.691386968955,0.723569779188 -0.690251240234,0.724653130187 -0.689113808387,0.725734693176 -0.68797467622,0.726814465487 -0.686833846544,0.727892444455 -0.685691322174,0.728968627421 -0.684547105929,0.73004301173 -0.683401200631,0.73111559473 -0.682253609109,0.732186373775 -0.681104334195,0.733255346223 -0.679953378722,0.734322509436 -0.678800745533,0.735387860781 -0.67764643747,0.73645139763 -0.676490457382,0.737513117358 -0.675332808121,0.738573017346 -0.674173492543,0.739631094979 -0.67301251351,0.740687347645 -0.671849873885,0.741741772739 -0.670685576537,0.742794367659 -0.669519624339,0.743845129807 -0.668352020168,0.744894056592 -0.667182766905,0.745941145424 -0.666011867434,0.746986393721 -0.664839324646,0.748029798903 -0.663665141432,0.749071358396 -0.662489320691,0.75011106963 -0.661311865324,0.75114893004 -0.660132778235,0.752184937064 -0.658952062334,0.753219088147 -0.657769720534,0.754251380736 -0.656585755753,0.755281812285 -0.655400170912,0.756310380251 -0.654212968936,0.757337082097 -0.653024152754,0.758361915289 -0.651833725301,0.759384877298 -0.650641689512,0.7604059656 -0.64944804833,0.761425177676 -0.648252804699,0.762442511011 -0.647055961569,0.763457963096 -0.645857521893,0.764471531423 -0.644657488628,0.765483213493 -0.643455864734,0.766493006809 -0.642252653177,0.76750090888 -0.641047856925,0.768506917219 -0.639841478951,0.769511029343 -0.638633522232,0.770513242776 -0.637423989749,0.771513555043 -0.636212884485,0.772511963678 -0.635000209429,0.773508466216 -0.633785967573,0.774503060199 -0.632570161913,0.775495743172 -0.631352795449,0.776486512687 -0.630133871185,0.777475366299 -0.628913392129,0.778462301567 -0.627691361291,0.779447316057 -0.626467781687,0.780430407338 -0.625242656336,0.781411572985 -0.624015988261,0.782390810577 -0.622787780488,0.783368117696 -0.621558036049,0.784343491933 -0.620326757977,0.785316930881 -0.61909394931,0.786288432137 -0.61785961309,0.787257993304 -0.616623752364,0.78822561199 -0.615386370179,0.789191285809 -0.61414746959,0.790155012376 -0.612907053653,0.791116789314 -0.611665125429,0.79207661425 -0.610421687982,0.793034484816 -0.60917674438,0.793990398648 -0.607930297695,0.794944353388 -0.606682351002,0.795896346681 -0.605432907381,0.796846376179 -0.604181969914,0.797794439539 -0.602929541689,0.798740534419 -0.601675625795,0.799684658487 -0.600420225326,0.800626809413 -0.59916334338,0.801566984871 -0.597904983058,0.802505182542 -0.596645147464,0.803441400112 -0.595383839708,0.80437563527 -0.594121062902,0.805307885711 -0.592856820161,0.806238149135 -0.591591114605,0.807166423246 -0.590323949356,0.808092705755 -0.589055327542,0.809016994375 -0.587785252292,0.809939286826 -0.586513726741,0.810859580832 -0.585240754026,0.811777874123 -0.583966337286,0.812694164433 -0.582690479669,0.813608449501 -0.58141318432,0.814520727071 -0.580134454392,0.815430994891 -0.57885429304,0.816339250717 -0.577572703422,0.817245492307 -0.576289688702,0.818149717425 -0.575005252043,0.81905192384 -0.573719396617,0.819952109325 -0.572432125595,0.820850271661 -0.571143442153,0.82174640863 -0.569853349472,0.822640518021 -0.568561850734,0.823532597628 -0.567268949127,0.824422645251 -0.56597464784,0.825310658693 -0.564678950066,0.826196635763 -0.563381859003,0.827080574275 -0.562083377852,0.827962472047 -0.560783509816,0.828842326905 -0.559482258102,0.829720136676 -0.558179625922,0.830595899196 -0.556875616488,0.831469612303 -0.55557023302,0.832341273841 -0.554263478737,0.833210881659 -0.552955356864,0.834078433613 -0.551645870628,0.834943927561 -0.550335023262,0.835807361368 -0.549022817998,0.836668732904 -0.547709258075,0.837528040042 -0.546394346734,0.838385280663 -0.54507808722,0.839240452652 -0.543760482779,0.840093553899 -0.542441536663,0.840944582298 -0.541121252127,0.84179353575 -0.539799632428,0.84264041216 -0.538476680827,0.843485209439 -0.537152400588,0.844327925502 -0.535826794979,0.84516855827 -0.534499867271,0.846007105668 -0.533171620737,0.846843565628 -0.531842058656,0.847677936085 -0.530511184307,0.848510214982 -0.529179000974,0.849340400263 -0.527845511945,0.850168489882 -0.52651072051,0.850994481795 -0.525174629961,0.851818373963 -0.523837243597,0.852640164354 -0.522498564716,0.85345985094 -0.521158596622,0.854277431699 -0.519817342621,0.855092904614 -0.518474806022,0.855906267671 -0.517130990138,0.856717518865 -0.515785898285,0.857526656194 -0.514439533782,0.85833367766 -0.51309189995,0.859138581274 -0.511743000114,0.859941365049 -0.510392837604,0.860742027004 -0.50904141575,0.861540565163 -0.507688737887,0.862336977557 -0.506334807353,0.86313126222 -0.504979627488,0.863923417193 -0.503623201636,0.86471344052 -0.502265533143,0.865501330253 -0.500906625361,0.866287084447 -0.499546481641,0.867070701164 -0.498185105339,0.867852178471 -0.496822499816,0.868631514438 -0.495458668432,0.869408707144 -0.494093614554,0.87018375467 -0.492727341548,0.870956655104 -0.491359852787,0.871727406539 -0.489991151644,0.872496007073 -0.488621241497,0.87326245481 -0.487250125725,0.874026747859 -0.485877807712,0.874788884333 -0.484504290844,0.875548862354 -0.48312957851,0.876306680044 -0.481753674102,0.877062335535 -0.480376581014,0.877815826961 -0.478998302645,0.878567152464 -0.477618842395,0.879316310191 -0.476238203668,0.880063298291 -0.474856389871,0.880808114923 -0.473473404412,0.881550758248 -0.472089250705,0.882291226435 -0.470703932165,0.883029517656 -0.46931745221,0.883765630089 -0.467929814261,0.884499561918 -0.466541021741,0.885231311332 -0.465151078077,0.885960876527 -0.4637599867,0.886688255701 -0.462367751041,0.887413447059 -0.460974374535,0.888136448814 -0.459579860621,0.888857259179 -0.45818421274,0.889575876378 -0.456787434334,0.890292298637 -0.455389528851,0.891006524188 -0.45399049974,0.891718551269 -0.452590350452,0.892428378124 -0.451189084442,0.893136003 -0.449786705168,0.893841424151 -0.44838321609,0.894544639838 -0.446978620671,0.895245648325 -0.445572922377,0.895944447882 -0.444166124676,0.896641036785 -0.442758231039,0.897335413316 -0.44134924494,0.898027575761 -0.439939169856,0.898717522412 -0.438528009266,0.899405251566 -0.437115766651,0.900090761528 -0.435702445497,0.900774050605 -0.43428804929,0.901455117112 -0.43287258152,0.902133959368 -0.431456045681,0.902810575698 -0.430038445267,0.903484964433 -0.428619783775,0.904157123908 -0.427200064707,0.904827052466 -0.425779291565,0.905494748453 -0.424357467855,0.906160210221 -0.422934597085,0.90682343613 -0.421510682766,0.907484424541 -0.420085728412,0.908143173825 -0.418659737537,0.908799682356 -0.417232713662,0.909453948514 -0.415804660306,0.910105970685 -0.414375580993,0.91075574726 -0.41294547925,0.911403276635 -0.411514358605,0.912048557214 -0.410082222589,0.912691587404 -0.408649074736,0.913332365617 -0.407214918582,0.913970890274 -0.405779757666,0.914607159799 -0.404343595529,0.915241172621 -0.402906435714,0.915872927177 -0.401468281767,0.916502421907 -0.400029137237,0.917129655259 -0.398589005675,0.917754625684 -0.397147890635,0.918377331641 -0.395705795671,0.918997771593 -0.394262724343,0.91961594401 -0.392818680211,0.920231847366 -0.391373666837,0.920845480141 -0.389927687788,0.921456840821 -0.388480746631,0.922065927899 -0.387032846937,0.92267273987 -0.385583992277,0.923277275238 -0.384134186228,0.923879532511 -0.382683432365,0.924479510204 -0.381231734269,0.925077206834 -0.379779095522,0.925672620929 -0.378325519707,0.926265751019 -0.376871010412,0.92685659564 -0.375415571225,0.927445153335 -0.373959205738,0.92803142265 -0.372501917543,0.928615402141 -0.371043710237,0.929197090365 -0.369584587417,0.929776485888 -0.368124552685,0.93035358728 -0.366663609641,0.930928393117 -0.365201761892,0.931500901981 -0.363739013043,0.932071112458 -0.362275366705,0.932639023143 -0.360810826488,0.933204632634 -0.359345396006,0.933767939535 -0.357879078875,0.934328942457 -0.356411878713,0.934887640014 -0.354943799141,0.93544403083 -0.353474843779,0.93599811353 -0.352005016254,0.936549886748 -0.350534320191,0.937099349122 -0.34906275922,0.937646499297 -0.347590336971,0.938191335922 -0.346117057077,0.938733857654 -0.344642923175,0.939274063153 -0.343167938899,0.939811951086 -0.341692107891,0.940347520127 -0.340215433792,0.940880768954 -0.338737920245,0.941411696251 -0.337259570896,0.941940300709 -0.335780389393,0.942466581022 -0.334300379384,0.942990535893 -0.332819544523,0.943512164028 -0.331337888463,0.944031464141 -0.329855414859,0.94454843495 -0.32837212737,0.94506307518 -0.326888029655,0.94557538356 -0.325403125376,0.946085358828 -0.323917418198,0.946592999723 -0.322430911786,0.947098304995 -0.320943609807,0.947601273395 -0.319455515932,0.948101903684 -0.317966633832,0.948600194626 -0.316476967182,0.94909614499 -0.314986519655,0.949589753555 -0.313495294931,0.950081019101 -0.312003296688,0.950569940416 -0.310510528609,0.951056516295 -0.309016994375,0.951540745537 -0.307522697673,0.952022626946 -0.306027642189,0.952502159334 -0.304531831612,0.952979341517 -0.303035269633,0.953454172319 -0.301537959944,0.953926650567 -0.300039906241,0.954396775097 -0.298541112219,0.954864544747 -0.297041581577,0.955329958363 -0.295541318014,0.955793014798 -0.294040325232,0.956253712909 -0.292538606935,0.956712051559 -0.291036166828,0.957168029617 -0.289533008618,0.957621645958 -0.288029136015,0.958072899462 -0.286524552728,0.958521789017 -0.28501926247,0.958968313515 -0.283513268955,0.959412471854 -0.2820065759,0.959854262938 -0.280499187022,0.960293685677 -0.278991106039,0.960730738987 -0.277482336674,0.961165421789 -0.275972882649,0.961597733011 -0.274462747688,0.962027671586 -0.272951935517,0.962455236454 -0.271440449865,0.962880426559 -0.26992829446,0.963303240852 -0.268415473035,0.96372367829 -0.26690198932,0.964141737836 -0.265387847052,0.964557418458 -0.263873049965,0.96497071913 -0.262357601798,0.965381638833 -0.26084150629,0.965790176553 -0.259324767181,0.966196331282 -0.257807388214,0.966600102017 -0.256289373133,0.967001487762 -0.254770725683,0.967400487528 -0.253251449612,0.967797100329 -0.251731548668,0.968191325187 -0.250211026602,0.968583161129 -0.248689887165,0.968972607188 -0.24716813411,0.969359662404 -0.245645771192,0.969744325821 -0.244122802168,0.97012659649 -0.242599230795,0.970506473469 -0.241075060833,0.970883955819 -0.239550296042,0.971259042609 -0.238024940184,0.971631732915 -0.236498997024,0.972002025815 -0.234972470325,0.972369920398 -0.233445363856,0.972735415754 -0.231917681383,0.973098510982 -0.230389426677,0.973459205187 -0.228860603507,0.973817497477 -0.227331215647,0.97417338697 -0.225801266869,0.974526872787 -0.224270760949,0.974877954055 -0.222739701664,0.975226629909 -0.22120809279,0.975572899489 -0.219675938108,0.975916761939 -0.218143241397,0.976258216411 -0.216610006439,0.976597262064 -0.215076237017,0.97693389806 -0.213541936916,0.977268123568 -0.212007109922,0.977599937765 -0.210471759821,0.977929339831 -0.208935890402,0.978256328953 -0.207399505455,0.978580904325 -0.20586260877,0.978903065147 -0.204325204139,0.979222810622 -0.202787295357,0.979540139962 -0.201248886216,0.979855052384 -0.199709980514,0.980167547112 -0.198170582048,0.980477623373 -0.196630694615,0.980785280403 -0.195090322016,0.981090517443 -0.193549468051,0.98139333374 -0.192008136522,0.981693728546 -0.190466331231,0.981991701121 -0.188924055984,0.982287250729 -0.187381314586,0.98258037664 -0.185838110843,0.982871078132 -0.184294448562,0.983159354488 -0.182750331554,0.983445204995 -0.181205763627,0.98372862895 -0.179660748593,0.984009625651 -0.178115290264,0.984288194407 -0.176569392453,0.984564334529 -0.175023058975,0.984838045337 -0.173476293645,0.985109326155 -0.171929100279,0.985378176313 -0.170381482696,0.985644595149 -0.168833444713,0.985908582005 -0.16728499015,0.986170136229 -0.165736122828,0.986429257176 -0.164186846569,0.986685944208 -0.162637165195,0.98694019669 -0.16108708253,0.987192013995 -0.159536602398,0.987441395502 -0.157985728626,0.987688340595 -0.15643446504,0.987932848666 -0.154882815468,0.98817491911 -0.153330783737,0.988414551331 -0.151778373678,0.988651744738 -0.150225589121,0.988886498745 -0.148672433897,0.989118812772 -0.147118911839,0.989348686247 -0.145565026779,0.989576118603 -0.144010782552,0.989801109278 -0.142456182993,0.990023657717 -0.140901231938,0.990243763371 -0.139345933222,0.990461425697 -0.137790290685,0.990676644158 -0.136234308163,0.990889418222 -0.134677989497,0.991099747366 -0.133121338527,0.99130763107 -0.131564359092,0.99151306882 -0.130007055036,0.991716060111 -0.1284494302,0.99191660444 -0.126891488428,0.992114701314 -0.125333233564,0.992310350244 -0.123774669453,0.992503550747 -0.12221579994,0.992694302346 -0.120656628871,0.99288260457 -0.119097160095,0.993068456955 -0.117537397458,0.993251859042 -0.115977344809,0.99343281038 -0.114417005998,0.99361131052 -0.112856384873,0.993787359023 -0.111295485288,0.993960955455 -0.109734311091,0.994132099387 -0.108172866136,0.994300790397 -0.106611154275,0.994467028068 -0.105049179362,0.994630811991 -0.10348694525,0.994792141762 -0.101924455795,0.994951016981 -0.100361714851,0.995107437258 -0.098798726275,0.995261402206 -0.097235493922,0.995412911446 -0.095672021651,0.995561964603 -0.094108313319,0.99570856131 -0.092544372783,0.995852701205 -0.090980203904,0.995994383933 -0.089415810539,0.996133609143 -0.087851196551,0.996270376493 -0.086286365798,0.996404685645 -0.084721322142,0.996536536267 -0.083156069445,0.996665928034 -0.081590611568,0.996792860627 -0.080024952375,0.996917333733 -0.078459095728,0.997039347045 -0.076893045491,0.997158900261 -0.075326805528,0.997275993086 -0.073760379704,0.997390625232 -0.072193771883,0.997502796416 -0.070626985931,0.997612506361 -0.069060025714,0.997719754796 -0.067492895099,0.997824541457 -0.065925597951,0.997926866086 -0.064358138139,0.998026728428 -0.062790519529,0.998124128239 -0.06122274599,0.998219065278 -0.05965482139,0.998311539311 -0.058086749598,0.998401550109 -0.056518534482,0.998489097451 -0.054950179912,0.99857418112 -0.053381689759,0.998656800906 -0.051813067891,0.998736956606 -0.05024431818,0.998814648022 -0.048675444496,0.998889874962 -0.04710645071,0.998962637241 -0.045537340693,0.999032934678 -0.043968118318,0.999100767101 -0.042398787456,0.999166134343 -0.040829351979,0.999229036241 -0.039259815759,0.999289472641 -0.03769018267,0.999347443393 -0.036120456584,0.999402948355 -0.034550641374,0.99945598739 -0.032980740915,0.999506560366 -0.031410759078,0.999554667159 -0.029840699739,0.99960030765 -0.02827056677,0.999643481727 -0.026700364047,0.999684189283 -0.025130095443,0.999722430218 -0.023559764834,0.999758204437 -0.021989376093,0.999791511852 -0.020418933095,0.999822352381 -0.018848439715,0.999850725947 -0.017277899829,0.999876632482 -0.015707317312,0.99990007192 -0.014136696038,0.999921044204 -0.012566039883,0.999939549282 -0.010995352723,0.999955587109 -0.009424638433,0.999969157645 -0.007853900889,0.999980260856 -0.006283143966,0.999988896716 -0.004712371539,0.999995065202 -0.003141587486,0.9999987663 -0.001570795681,1 0,20000000 0,20000000 -20000000,0 -20000000))";
      const char *exp =
        "TBD";
      doClipTest(inp, exp, r);
    }

}
