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
    template<> template<> void object::test<208>()
    {
      Rectangle r(3.0481343214686657e-14, -20000000.000000, 20000000.000000, -1.000000);
      const char *clip =
        "POLYGON((3.0481343214686657e-14 -20000000, 200000000 -20000000, 200000000 -1, 3.0481343214686657e-14 -1, 3.0481343214686657e-14 -20000000))";
      const char *inp =
        "POLYGON ((0.000000000000030481343214686657 -20000000.000000000000000000000000000000, 0.000000000000030481343214686657 -1.000000000000000000000000000000, 0.001570795680861262382313481289 -0.999998766299703523152686557296, 0.003141587485909849188081555127 -0.999995065201858102099663483386, 0.004712371539403610240515085650 -0.999988896715595876329985003395, 0.006283143965589040730312930805 -0.999980260856136893288237388333, 0.007853900888741325789221825460 -0.999969157644789441441446342651, 0.009424638433173900334272055090 -0.999955587108949495167564691656, 0.010995352723248016935109205860 -0.999939549282101047822379769059, 0.012566039883382304140346796828 -0.999921044203815778672606029431, 0.014136696038062333477536292037 -0.999900071919753052895885048201, 0.015707317311850176044796256747 -0.999876632481660143625390446687, 0.017277899829393969510782369525 -0.999850725947371343771408191969, 0.018848439715437478175763530430 -0.999822352380808410110546446958, 0.020418933094829658236868397125 -0.999791511851980452263433107873, 0.021989376092534214379714541110 -0.999758204436983377583203491668, 0.023559764833639161574208031880 -0.999722430217999891155500336026, 0.025130095443366386870343021087 -0.999684189283299273753868874337, 0.026700364047081214663448278657 -0.999643481727237270817454373173, 0.028270566770301961551092873037 -0.999600307650255759384094744746, 0.029840699738709498128885755364 -0.999554667158882415023413159361, 0.031410759078156803847381439709 -0.999506560365730711836818045413, 0.032980740914678535746773491155 -0.999455987389499256323688314296, 0.034550641374500583313800206042 -0.999402948354971898403675822919, 0.036120456584049616399756388319 -0.999347443393017287327495523641, 0.037690182669962650485739885653 -0.999289472640588205543110689177, 0.039259815759096622356238981411 -0.999229036240721790740337837633, 0.040829351978537910261568555370 -0.999166134342538980739334419923, 0.042398787455611923469245283513 -0.999100767101243847356784044678, 0.043968118317892622426423798743 -0.999032934678123485383594015730, 0.045537340693212115250165794578 -0.998962637240547790540290407080, 0.047106450709670177889876185873 -0.998889874961968682320900825289, 0.048675444495643829800890500792 -0.998814648021919770926047021931, 0.050244318179796874923592753248 -0.998736956606016135218339968560, 0.051813067891089449601427219250 -0.998656800905953545566262619104, 0.053381689758787591315591924968 -0.998574181119508241799564984831, 0.054950179912472765786368711360 -0.998489097450536489120054284285, 0.056518534482051442646710626150 -0.998401550108973467878570318135, 0.058086749597764629482465892352 -0.998311539310833828686497781746, 0.059654821390197419750389684623 -0.998219065278210138103531789966, 0.061222745990288533757262001700 -0.998124128239272989659980339638, 0.062790519529339894333475058374 -0.998026728428269893633739684446, 0.064358138139026133117681638396 -0.997926866085525388072596797429, 0.065925597951404180108170294261 -0.997824541457439706526599820791, 0.067492895098922742191938084488 -0.997719754796488889070360528422, 0.069060025714431913512747485129 -0.997612506361223450035424775706, 0.070626985931192695633562550483 -0.997502796416268266987970037007, 0.072193771882886517698985073821 -0.997390625232321803572688168060, 0.073760379703624784353266363723 -0.997275993086155221334365705843, 0.075326805527958437536106828247 -0.997158900260611935628674018517, 0.076893045490887462767304327826 -0.997039347044607060510656992847, 0.078459095727870464820341567247 -0.996917333733125965444799021498, 0.080024952374834132373671025107 -0.996792860627224608371932390583, 0.081590611568182869195453577049 -0.996665928034027759352397879411, 0.083156069444808231039267809592 -0.996536536266729222610649685521, 0.084721322142098556828848643363 -0.996404685644590282223020949459, 0.086286365797948391676008839113 -0.996270376492939147006211442203, 0.087851196550768090309802005322 -0.996133609143170284383472790068, 0.089415810539493323361170951102 -0.995994383932743532206188774580, 0.090980203903594625280959462543 -0.995852701205183432620060557383, 0.092544372783086858991197232172 -0.995708561310077899797477130051, 0.094108313318538819314262866556 -0.995561964603077664826003001508, 0.095672021651082711501956623579 -0.995412911445895831619168347970, 0.097235493922423629764573149714 -0.995261402206305989537327150174, 0.098798726274849216211215718886 -0.995107437258142213387657193380, 0.100361714851238986723203083784 -0.994951016981297731156530517183, 0.101924455795074045405534945985 -0.994792141761724035831093715387, 0.103486945250446424338086615080 -0.994630811991429886198545773368, 0.105049179362068742515923247538 -0.994467028068480418667718367942, 0.106611154275283614989433544906 -0.994300790396996370112958629761, 0.108172866136073214660129337972 -0.994132099387152412539592205576, 0.109734311091068764687506131850 -0.993960955455177042061620795721, 0.111295485287560072529267074515 -0.993787359023350913567185216380, 0.112856384873504980714820078447 -0.993611310520005730495540774427, 0.114417005997538942518865212605 -0.993432810379523911770149879885, 0.115977344808984458857104016261 -0.993251859042336704419540183153, 0.117537397457860626204251275340 -0.993068456954923628465792262432, 0.119097160094892629000895567515 -0.992882604569810922612305148505, 0.120656628871521190426996383849 -0.992694302345570989132284012157, 0.122215799939912120319895905141 -0.992503550746820950578808151477, 0.123774669452965821458967354829 -0.992310350244221206494898979145, 0.125333233564326740339112120637 -0.992114701314474989324310172378, 0.126891488428392817944256876217 -0.991916604440326477032385810162, 0.128449430200325120932092204384 -0.991716060110560015949943135638, 0.130007055036057195263055064061 -0.991513068819998788505643005919, 0.131564359092304572484977143176 -0.991307631069503591980662804417, 0.133121338526574345406672250647 -0.991099747365971839307974278199, 0.134677989497174466215767552058 -0.990889418222335671693201675225, 0.136234308163223460930169039784 -0.990676644157561514525411894283, 0.137790290684659727515892768679 -0.990461425696648189997972622223, 0.139345933222251056049501016787 -0.990243763370625806885527708801, 0.140901231937604121124962830436 -0.990023657716554539298670079006, 0.142456182993173974260514569323 -0.989801109277523183394009720359, 0.144010782552273425283217989090 -0.989576118602647825106544132723, 0.145565026779082618002547633296 -0.989348686247070840948936165660, 0.147118911838658439350524531619 -0.989118812771958677565464768122, 0.148672433896943956277425513690 -0.988886498744501407642815138388, 0.150225589120777908158643754177 -0.988651744737910842530936861294, 0.151778373677904199201549317877 -0.988414551331419088953111895535, 0.153330783736981279830047242285 -0.988174919110277327760627485986, 0.154882815467591666847013698316 -0.987932848665754148598239225976, 0.156434465040251324818854072873 -0.987688340595134439681146432122, 0.157985728626419186237939129569 -0.987441395501717611438152744086, 0.159536602398506560662738706924 -0.987192013994815931177129186835, 0.161087082529886571613531032199 -0.986940196689753634906594470522, 0.162637165194903648979263266483 -0.986685944207864817911968202679, 0.164186846568882882646533971638 -0.986429257176492213510243800556, 0.165736122828139542662029271014 -0.986170136228985527715451553377, 0.167284990149988460617080932025 -0.985908582004699995948726609640, 0.168833444712753522054526911234 -0.985644595148994717703772039386, 0.170381482695776964586542590041 -0.985378176313230880190019433940, 0.171929100279428981323803782288 -0.985109326154770537087301818246, 0.173476293645117018993317969944 -0.984838045336974721166711788101, 0.175023058975295270345284848190 -0.984564334529202001000669497444, 0.176569392453474027782078792370 -0.984288194406806371539175870566, 0.178115290264229148009533787445 -0.984009625651136254909090439469, 0.179660748593211461177077126194 -0.983728628949532390990384556062, 0.181205763627156180017863107423 -0.983445204995326172081604454434, 0.182750331553892281233331118528 -0.983159354487838088587636775628, 0.184294448562351942388914949333 -0.982871078132375730618264242366, 0.185838110842579923298600874659 -0.982580376640232455720536108856, 0.187381314585743058431788199414 -0.982287250728685168432718910481, 0.188924055984139444008818031762 -0.981991701120993099038969376124, 0.190466331231208096941287521986 -0.981693728546395361078680252831, 0.192008136521538141927578635659 -0.981393333740109730101153218129, 0.193549468050878276104143083103 -0.981090517443330534241852092237, 0.195090322016146178185636017588 -0.980785280403226877865563437808, 0.196630694615437862093898502280 -0.980477623372940865209557159687, 0.198170582048037058342515592813 -0.980167547111585490959839717107, 0.199709980514424623176950035486 -0.979855052384243307983524573501, 0.201248886216287836692373502956 -0.979540139961963762793573096133, 0.202787295356529895240527139322 -0.979222810621762196348072393448, 0.204325204139279126280825948925 -0.978903065146616957470371289673, 0.205862608769898536298370572695 -0.978580904325468514670660624688, 0.207399505454994970143900445692 -0.978256328953216347521504303586, 0.208935890402428686707381189080 -0.977929339830718169501722059067, 0.210471759821322546013533383302 -0.977599937764787041416525426030, 0.212007109922071446117541881904 -0.977268123568189817085283266351, 0.213541936916351593467311431596 -0.976933898059645033917774981091, 0.215076237017129967554751601710 -0.976597262063820914512746185210, 0.216610006438673619033608019890 -0.976258216411333368256464382284, 0.218143241396558940081717992143 -0.975916761938743770876669714198, 0.219675938107681101296719816673 -0.975572899488557077063433098374, 0.221208092790263322058308403939 -0.975226629909219711045409439976, 0.222739701663866224157217743596 -0.974877954055117568188393306627, 0.224270760949397157668627755811 -0.974526872786573461482362290553, 0.225801266869119582336722373839 -0.974173386969845678251544995874, 0.227331215646662226914642701558 -0.973817497477125204596859475714, 0.228860603507028581571347558565 -0.973459205186534060061376294470, 0.230389426676606168253869100226 -0.973098510982122855139664352464, 0.231917681383175755538417206481 -0.972735415753868681854044098145, 0.233445363855920795526088795668 -0.972369920397672893308538277779, 0.234972470325436638693972213332 -0.972002025815358994265125147649, 0.236498997023739859768554083530 -0.971631732914670198653084298712, 0.238024940184277611354701775781 -0.971259042609267542189854793833, 0.239550296041936811031192178234 -0.970883955818727328868078529922, 0.241075060833053522735269780242 -0.970506473468538799487248525111, 0.242599230795422199369326676788 -0.970126596490102133252264593466, 0.244122802168304980918733804174 -0.969744325820726116305081632163, 0.245645771192440909302945328818 -0.969359662403625588211752983625, 0.247168134110055365271207961086 -0.968972607187919221516381185211, 0.248689887164869172231362881575 -0.968583161128627412317371181416, 0.250211026602107922123252592428 -0.968191325186669504709868760983, 0.251731548668511273536552153018 -0.967797100328861792384316231619, 0.253251449612342194317449184382 -0.967400487527914854091193319618, 0.254770725683396148664172642384 -0.967001487762431333194967919553, 0.256289373133010478511550900294 -0.966600102016903606205744381441, 0.257807388214073618382116137582 -0.966196331281711007221701947856, 0.259324767181034199214906266207 -0.965790176553117718505347966129, 0.260841506289910485261174244442 -0.965381638833270216970561250491, 0.262357601798299477913190003164 -0.964970719130194609647332981694, 0.263873049965386241577647297163 -0.964557418457794413235717456701, 0.265387847051953063015616862685 -0.964141737835848000592875450820, 0.266901989320388721704802037493 -0.963723678290006047220117579855, 0.268415473034697704690643149661 -0.963303240851788866727645199717, 0.269928294460509310415119443860 -0.962880426558583968343896231090, 0.271440449865087141123609626447 -0.962455236453643614424890984083, 0.272951935517337984649088866718 -0.962027671586082266941275520367, 0.274462747687821362330140573249 -0.961597733010873589876155165257, 0.275972882648758299772850932641 -0.961165421788848339801347719913, 0.277482336674040985791123148374 -0.960730738986691368275216973416, 0.278991106039241598679723210807 -0.960293685676939512418925914972, 0.280499187021621798621140442265 -0.959854262937978153225060395926, 0.282006575900141609469784498287 -0.959412471854039217156184804480, 0.283513268955468800136543450208 -0.958968313515198511609582965320, 0.285019262469988099439888173947 -0.958521789017372283225881801627, 0.286524552727810188912371813785 -0.958072899462315663576816859859, 0.288029136014780973162885402417 -0.957621645957618672362343659188, 0.289533008618490739216611018492 -0.957168029616704774120705678797, 0.291036166828283426877277406675 -0.956712051558826992447848169832, 0.292538606935265510511356978895 -0.956253712909065911595973830117, 0.294040325232315324921472665665 -0.955793014798326678871376316238, 0.295541318014092391219804767388 -0.955329958363335896009971293097, 0.297041581577046076567683030589 -0.954864544746639509753549646121, 0.298541112219425308627052118027 -0.954396775096599148113796218240, 0.300039906241287235300063684917 -0.953926650567390121970845484611, 0.301537959944506550602483230250 -0.953454172318997761337300289597, 0.303035269632784765025945716843 -0.952979341517215416956787521485, 0.304531831611658976299850110081 -0.952502159333640685545674386958, 0.306027642188511306287068691745 -0.952022626945673189347019160778, 0.307522697672577838279295292523 -0.951540745536511578528404697863, 0.309016994374957831848149680809 -0.951056516295150200512864557822, 0.310510528608622660140525795214 -0.950569940416376546465926367091, 0.312003296688425080240847364621 -0.950081019100768364715747793525, 0.313495294931108225977567371956 -0.949589753554690330084042670933, 0.314986519655314822774272442985 -0.949096144990291268328519436182, 0.316476967181596013922728616308 -0.948600194625501380585319566308, 0.317966633832420741967439425935 -0.948101903684028690655338778015, 0.319455515932184630489842902534 -0.947601273395356824558177777362, 0.320943609807219087937113499720 -0.947098304994741013729253609199, 0.322430911785800411450964020332 -0.946592999723205985596052869369, 0.323917418198158835185296311465 -0.946085358827542077797545516660, 0.325403125376487523112700728234 -0.945575383560302906715833159979, 0.326888029654951672853258060059 -0.945063075179801592717865332816, 0.328372127369697675014492688206 -0.944548434950108317664785317902, 0.329855414858861883953267124525 -0.944031464141046550153646421677, 0.331337888462579888138037631506 -0.943512164028190492004455336428, 0.332819544522995447444202454790 -0.942990535892861303501888414758, 0.334300379384269541471752518191 -0.942466581022124882949242419272, 0.335780389392589306840619656214 -0.941940300708787536798638484470, 0.337259570896177141019478540329 -0.941411696251393759204972866428, 0.338737920245299806154548605264 -0.940880768954222457267633217270, 0.340215433792277255342639818991 -0.940347520127283953428332097246, 0.341692107891491569926500915244 -0.939811951086316654802033099259, 0.343167938899396285368226244827 -0.939274063152784055574784360942, 0.344642923174525106499999083098 -0.938733857653871073267737301649, 0.346117057077500955841742324992 -0.938191335922481162157282597036, 0.347590336971044910896466717531 -0.937646499297232760561371378571, 0.349062759219985307979072786111 -0.937099349122455960170441358059, 0.350534320191266679511699067007 -0.936549886748189508445250339719, 0.352005016253958580296767877371 -0.935998113530177144880894957168, 0.353474843779264580323484779001 -0.935444030829864492382341722987, 0.354943799140531535130094198394 -0.934887640014395504550748228212, 0.356411878713257967987715346680 -0.934328942456609246036691729387, 0.357879078875103617818353995972 -0.933767939535036339826490348059, 0.359345396005897765867587168032 -0.933204632633895858617734120344, 0.360810826487648783622574910623 -0.932639023143091439038698808872, 0.362275366704552570507047448700 -0.932071112458208284046179414872, 0.363739013043001768732409573204 -0.931500901980509610211811377667, 0.365201761891594534059635179801 -0.930928393116933095008391774172, 0.366663609641143417583464270137 -0.930353587280087435118502980913, 0.368124552684684469561204878119 -0.929776485888248793720833873522, 0.369584587417486010174627608649 -0.929197090365357691865710876300, 0.371043710237057344780708945109 -0.928615402141014789627604386624, 0.372501917543158089785038100672 -0.928031422650478110547567212052, 0.373959205737806610336804169492 -0.927445153334658822785740994732, 0.375415571225289124157598052989 -0.926856595640118352541492185992, 0.376871010412168638836760692357 -0.926265751019064276228220933262, 0.378325519707293556059823913529 -0.925672620929346878781984742091, 0.379779095521806886459614815976 -0.925077206834455711970122138155, 0.381231734269154687311242923897 -0.924479510203515930655271404248, 0.382683432365095332894355806275 -0.923879532511284407014784392231, 0.384134186227708063210428690581 -0.923277275238146732938560035109, 0.385583992277401921278112695290 -0.922672739870112557092340921372, 0.387032846936924634917431831127 -0.922065927898813031404756657139, 0.388480746631371276489375077290 -0.921456840821496259152922903013, 0.389927687788193422235849538993 -0.920845480141024075315669961128, 0.391373666837207423441213904880 -0.920231847365868160792956587102, 0.392818680210603843327987760858 -0.919615944010106489692191189533, 0.394262724342955839240687510028 -0.918997771593419221503040716925, 0.395705795671227988918872142676 -0.918377331641085481450659244729, 0.397147890634785283303642700048 -0.917754625683979141648194399750, 0.398589005675401730766083119306 -0.917129655258565157360806097131, 0.400029137237269238891457234786 -0.916502421906896014291987739853, 0.401468281767006329729952085472 -0.915872927176607620758375105652, 0.402906435713666910558572453738 -0.915241172620915643953765084007, 0.404343595528749155665337866594 -0.914607159798611735190831950604, 0.405779757666204055066572209398 -0.913970890274059422075936254259, 0.407214918582444296291100727103 -0.913332365617190444773143553903, 0.408649074736352979630993331739 -0.912691587403501092268243155559, 0.410082222589292333392307909889 -0.912048557214047761476649611723, 0.411514358605112484656984861431 -0.911403276635443515552026383375, 0.412945479250160285555892869525 -0.910755747259854198105699651933, 0.414375580993287695452664820550 -0.910105970684994103336862281139, 0.415804660305860829261348499131 -0.909453948514122201274290091533, 0.417232713661768617185998664354 -0.908799682356038585062663059944, 0.418659737537431353437966663478 -0.908143173825079808025861893839, 0.420085728411809411486643739408 -0.907484424541115441975591693335, 0.421510682766412125843658031954 -0.906823436129543969386190838122, 0.422934597085306285269012960271 -0.906160210221288453524834949349, 0.424357467855124736999528067827 -0.905494748452792985737858089124, 0.425779291565075435066489717428 -0.904827052466018244558654259890, 0.427200064706949711457184548635 -0.904157123908437720949393678893, 0.428619783775130991365642785240 -0.903484964433033610475831665099, 0.430038445266603619465684005263 -0.902810575698292483437512601085, 0.431456045680961408628206754656 -0.902133959368201621131788670027, 0.432872581520416299660780623526 -0.901455117112244574961721355066, 0.434288049289806854513784628580 -0.900774050605396947588587863720, 0.435702445496607138064604214378 -0.900090761528122840218202327378, 0.437115766650934989279164710752 -0.899405251566369967619607450615, 0.438528009265561069529582027826 -0.898717522411566216433698173205, 0.439939169855917078244544882182 -0.898027575760614649169610856916, 0.441349244940104468160058104331 -0.897335413315890284557951872557, 0.442758231038903271592488408714 -0.896641036785234990524884324259, 0.444166124675780260577795388599 -0.895944447881953931478449248971, 0.445572922376897828655728517333 -0.895245648324810905371862190805, 0.446978620671122595098267993308 -0.894544639838024346900624550472, 0.448383216090033620560006966116 -0.893841424151263108655030009686, 0.449786705167931455395802231578 -0.893136002999641909205763568025, 0.451189084441846244288853995386 -0.892428378123717336301012892363, 0.452590350451546552523751643093 -0.891718551269483405974369816249, 0.453990499739547803681460891312 -0.891006524188367343697336764308, 0.455389528851120828356613401411 -0.890292298637225254509530714131, 0.456787434334300357363645161968 -0.889575876378337571104282233136, 0.458184212739893514942934871215 -0.888857259179405057025746828003, 0.459579860621488534011547244518 -0.888136448813544143732201519015, 0.460974374535462916302464009277 -0.887413447059282933793156189495, 0.462367751040992258637629674922 -0.886688255700556204885742772603, 0.463759986700058579600636221585 -0.885960876526701635036431525805, 0.465151078077458701720559020032 -0.885231311332455028662025142694, 0.466541021740812966722700139144 -0.884499561917945986699862714886, 0.467929814260573506690121803331 -0.883765630088693354693418768875, 0.469317452210032959314389700012 -0.883029517655601003944809690438, 0.470703932165332517012501511999 -0.882291226434953279600392761495, 0.472089250705470808711083918752 -0.881550758248410337714062734449, 0.473473404412311948963321128758 -0.880808114923003704355153331562, 0.474856389870594308710849418276 -0.880063298291132056760943669360, 0.476238203667938730934139357487 -0.879316310190556449377652370458, 0.477618842394857079369785424205 -0.878567152464395650923734137905, 0.478998302644760509672039461293 -0.877815826961121925542386179586, 0.480376581013967962618949059106 -0.877062335534556369864844782569, 0.481753674101714546296193475428 -0.876306680043864028029076962412, 0.483129578510160084814373249174 -0.875548862353549672832286887569, 0.484504290844397111914787501519 -0.874788884333453253816514916252, 0.485877807712459586220177243376 -0.874026747858744901265026783221, 0.487250125725331273418561295330 -0.873262454809920707354820024193, 0.488621241496953795380164820017 -0.872496007072797730153013162635, 0.489991151644235400919313860868 -0.871727406538509552724747209140, 0.491359852787058848377910180716 -0.870956655103501620196482235770, 0.492727341548290176387325800533 -0.870183754669526465796991487878, 0.494093614553786864007633994333 -0.869408707143639047920657958457, 0.495458668432405990866840284070 -0.868631514438192087190770962479, 0.496822499816012896900474515860 -0.867852178470831403522822711238, 0.498185105339489120446216929849 -0.867070701164491031143199961662, 0.499546481640741057983490236438 -0.866287084447388444630178128136, 0.500906625360708068761539379921 -0.865501330253020006999520319368, 0.502265533143370523916360070871 -0.864713440520156195745471450209, 0.503623201635758799277198249911 -0.863923417192836495814844965935, 0.504979627487960658349663845001 -0.863131262220364958714924341621, 0.506334807353130300633381466469 -0.862336977557305206509852268937, 0.507688737887496022160860320582 -0.861540565163475768883927230490, 0.509041415750368875237086285779 -0.860742027003945087137992686621, 0.510392837604150773067601676303 -0.859941365049026518185826262197, 0.511743000114342372341980080819 -0.859138581274273893662041245989, 0.513091899949552177062628288695 -0.858333677660476523918475777464, 0.514439533781503754994446353521 -0.857526656193653868953674646036, 0.515785898285044619449024594360 -0.856717518865051319565395715472, 0.517130990138154333912723359390 -0.855906267671134757257789260620, 0.518474806021952283607845402003 -0.855092904613585780282392079243, 0.519817342620706446254530419537 -0.854277431699297040701424066356, 0.521158596621841274654229891894 -0.853459850940366693272665088443, 0.522498564715945690295484382659 -0.852640164354094176601961407869, 0.523837243596781743093515615328 -0.851818373962974551005800094572, 0.525174629961292382951398849400 -0.850994481794693946596908062929, 0.526510720509609564388142644020 -0.850168489882124345236036333517, 0.527845511945062906278280934202 -0.849340400263318695550651682424, 0.529179000974187019323835556861 -0.848510214981505916931325828045, 0.530511184306730387838513252063 -0.847677936085085570461217230331, 0.531842058655663030286575576611 -0.846843565627623084957065202616, 0.533171620737184825955523592711 -0.846007105667844538920974173379, 0.534499867270733397539572706592 -0.845168558269631997603710260591, 0.535826794978992659856942282204 -0.844327925502017628822670758382, 0.537152400587900147321818167256 -0.843485209439179151047483173897, 0.538476680826656006750852156983 -0.842640412160434837396394414100, 0.539799632427730324835124520177 -0.841793535750237964521147659980, 0.541121252126871565835131150379 -0.840944582298171816603371553356, 0.542441536663114454164258404489 -0.840093553898944689350969383668, 0.543760482778788190039165328926 -0.839240452652384449905298424710, 0.545078087219524332063258498238 -0.838385280663433762882164046459, 0.546394346734264901854771778744 -0.837528040042144428234394126775, 0.547709258075270266630241167150 -0.836668732903672607292833163228, 0.549022817998127576899491941731 -0.835807361368273049606614222284, 0.550335023261758204959903650888 -0.834943927561294430006455513649, 0.551645870628426071569094801816 -0.834078433613173908511839726998, 0.552955356863745750573002624151 -0.833210881659431801260495831229, 0.554263478736689907400148058514 -0.832341273840666140415578411194, 0.555570233019598069823530295253 -0.831469612302548011228964242036, 0.556875616488183844410286837956 -0.830595899195815445814616850839, 0.558179625921543354216680654645 -0.829720136676268649189580628445, 0.559482258102162899326970091352 -0.828842326904764670203462628706, 0.560783509815927061481488635764 -0.827962472047211850423309442704, 0.562083377852126586660119755834 -0.827080574274564606085391460510, 0.563381859003466267665771738393 -0.826196635762817876980079745408, 0.564678950066073048752457452792 -0.825310658693002352492840145715, 0.565974647839503575141861801967 -0.824422645251178143332992931391, 0.567268949126752519696026411111 -0.823532597628430118597009368386, 0.568561850734260021411614616227 -0.822640518020862576697993517882, 0.569853349471919790047991227766 -0.821746408629593028116744335421, 0.571143442153087321777604756790 -0.820850271660747532465052245243, 0.572432125594587004613345015969 -0.819952109325455036348273551994, 0.573719396616720556103530270775 -0.819051923839841933272509777453, 0.575005252043274683870777153061 -0.818149717425026179462577147206, 0.576289688701528968195475499670 -0.817245492307112519903000702470, 0.577572703422263744599263191049 -0.816339250717186715178286249284, 0.578854293039767542339291139797 -0.815430994891309546268587382656, 0.580134454391845522103210441855 -0.814520727070512151613002060913, 0.581413184319826803481134902540 -0.813608449500789698838332242303, 0.582690479668572236526813412638 -0.812694164433096721822380459344, 0.583966337286482728430314637080 -0.811777874123340348333499605360, 0.585240754025506348945384615945 -0.810859580832376192205401821411, 0.586513726741146435017526528100 -0.809939286826001469954405820317, 0.587785252292469362345173067297 -0.809016994374950115798128535971, 0.589055327542112316940858818270 -0.808092705754887230540361997555, 0.590323949356290844647787707800 -0.807166423246402975344437891181, 0.591591114604806622701005380804 -0.806238149135007464707314284169, 0.592856820161055564355478963989 -0.805307885711124660232940186688, 0.594121062902034813291152204329 -0.804375635270087152584039813519, 0.595383839708351292330235082773 -0.803441400112130277300082070724, 0.596645147464228586819956490217 -0.802505182542386452659854967351, 0.597904983057515271305248916178 -0.801566984870879295499435102101, 0.599163343379692237000710974826 -0.800626809412518403163971925096, 0.600420225325880463351779781078 -0.799684658487093247281052299513, 0.601675625794848678573600864183 -0.798740534419267622645577375806, 0.602929541689021131212200543814 -0.797794439538573652015429615858, 0.604181969914484917616448456101 -0.796846376179406679085559517262, 0.605432907380997864521532392246 -0.795896346681018496127535399864, 0.606682351001996189587828212098 -0.794944353387512681052839980111, 0.607930297694601939895164832706 -0.793990398647838047097025082621, 0.609176744379630652481694141898 -0.793034484815782869659983589372, 0.610421687981599014882760911860 -0.792076614249969668257733701466, 0.611665125428732636692075175233 -0.791116789313848878251178575738, 0.612907053652973043966767363600 -0.790155012375693077686378273938, 0.614147469589985894877770533640 -0.789191285808591103112519249407, 0.615386370179168085137177968136 -0.788225611990442720511396146321, 0.616623752363655630581718014582 -0.787257993303951852936961586238, 0.617859613090330994644716611219 -0.786288432136621584511715354893, 0.619093949309830637872664738097 -0.785316930880747610110859113774, 0.620326757976552678464088330657 -0.784343491933412684247173274343, 0.621558036048664219741510805761 -0.783368117696480736888986484701, 0.622787780488109121712625437794 -0.782390810576590767233540191228, 0.624015988260615217519955422176 -0.781411572985150848502655662742, 0.625242656335701862957421326428 -0.780430407338332354783005939680, 0.626467781686687485986908541236 -0.779447316057064076844085320772, 0.627691361290697247277137194033 -0.778462301567026115911573924677, 0.628913392128669812564112362452 -0.777475366298643555396097326593, 0.630133871185365901368413688033 -0.776486512687081242845010820020, 0.631352795449374504244133277098 -0.775495743172237128604251665820, 0.632570161913121209451560389425 -0.774503060198736492658611041406, 0.633785967572875419406841501768 -0.773508466215925838405098602379, 0.635000209428757456109337908856 -0.772511963677867008470911969198, 0.636212884484746221680495636974 -0.771513555043331078486801288818, 0.637423989748686525835807969997 -0.770513242775791806771223946271, 0.638633522232296635401382900454 -0.769511029343420527304431288940, 0.639841478951175268718998268014 -0.768506917219079266345715950592, 0.641047856924809478229576598096 -0.767500908880314636206776413019, 0.642252653176581311811332852812 -0.766493006809352395158896342764, 0.643455864733775806385551732092 -0.765483213493090786094796840189, 0.644657488627588315388550199714 -0.764471531423094208257396076078, 0.645857521893131059087522771733 -0.763457963095587777146988628374, 0.647055961569441229208621280122 -0.762442511011450441138492806203, 0.648252804699488316408917398803 -0.761425177676209430366327524098, 0.649448048330180660592247932072 -0.760405965600033484363962088537, 0.650641689512373444514992115728 -0.759384877297727189926490609651, 0.651833725300875799213429218071 -0.758361915288724430794786712795, 0.653024152754457798408793678391 -0.757337082097082170406565637677, 0.654212968935858119046145020548 -0.756310380251474456692051262507, 0.655400170911790924677120528941 -0.755281812285186204825038203126, 0.656585755752953525998805162089 -0.754251380736106313840139137028, 0.657769720534033042191879303573 -0.753219088146722559606871527649, 0.658952062333714061459488675609 -0.752184937064113934290787710779, 0.660132778234685746454601940059 -0.751148930039945650349864081363, 0.661311865323649050729670761939 -0.750111069630462035107143492496, 0.662489320691323713141684947914 -0.749071358396480535546402279579, 0.663665141432455696346437434840 -0.748029798903385056974002509378, 0.664839324645824181203579428256 -0.746986393721120189859163929214, 0.666011867434248783226280465897 -0.745941145424184659518118678534, 0.667182766904596880053190943727 -0.744894056591624553753661075461, 0.668352020167790272786589866882 -0.743845129807027549695419565978, 0.669519624338812846531254763249 -0.742794367658516252461708972987, 0.670685576536717231732609434403 -0.741741772738741755865987670404, 0.671849873884632575737896331702 -0.740687347644877314145617219765, 0.673012513509770649022811994655 -0.739631094978612235735226931865, 0.674173492543433838797284352040 -0.738573017346144777839356265758, 0.675332808121021699321318010334 -0.737513117358176373272726777941, 0.676490457382038501421561704774 -0.736451397629904969122094371414, 0.677646437470099560762548662751 -0.735387860781018365408101544745, 0.678800745532939120430171442422 -0.734322509435687997836339491187, 0.679953378722416568180619833583 -0.733255346222562498503805272776, 0.681104334194524208001553233771 -0.732186373774760923538451606873, 0.682253609109393810427945936681 -0.731115594729866535850248965289, 0.683401200631303717969444733171 -0.730043011729920254815340285859, 0.684547105928686061560028974782 -0.728968627421413994937893221504, 0.685691322174133310873855862155 -0.727892444455284115534254851809, 0.686833846544405712819525433588 -0.726814465486905314506316244660, 0.687974676220437841855925853451 -0.725734693176083300869549930212, 0.689113808387345927464195938228 -0.724653130187049021593281850073, 0.690251240234434626508175369963 -0.723569779188451778217938681337, 0.691386968955203906617157372239 -0.722484642853352232449992698093, 0.692520991747356040590943848656 -0.721397723859216521979931258102, 0.693653305812802489782598058810 -0.720309024887909266077201664302, 0.694783908357670898503499756771 -0.719218548625686904252063413878, 0.695912796592311866383795404545 -0.718126297763191256962045372347, 0.697039967731305942777453310555 -0.717032274995442642229193097592, 0.698165418993470288100411380583 -0.715936483021833547368828476465, 0.699289147601865779257934718771 -0.714838924546121745606797048822, 0.700411150783803893027368303592 -0.713739602276423634741320256580, 0.701531425770853367396284738788 -0.712638518925207797849452617811, 0.702649969798846862900632004312 -0.711535677209287675815119200706, 0.703766780107888179074393519841 -0.710431079849815794169387572765, 0.704881853942359026810038358235 -0.709324729572276213573900349729, 0.705995188550925578674366533960 -0.708216629106478312571937294706, 0.707106781186545241268959216541 -0.707106781186549904205662642198, 0.708216629106473649635233869049 -0.705995188550930241611069959617, 0.709324729572271550637196924072 -0.704881853942363689746741783893, 0.710431079849811131232684147108 -0.703766780107892842011096945498, 0.711535677209283012878415775049 -0.702649969798851525837335429969, 0.712638518925203134912749192154 -0.701531425770858030332988164446, 0.713739602276419082826919293439 -0.700411150783808666986374191765, 0.714838924546117082670093623165 -0.699289147601870553216940606944, 0.715936483021828884432125050807 -0.698165418993474951037114806240, 0.717032274995437979292489671934 -0.697039967731310605714156736212, 0.718126297763186594025341946690 -0.695912796592316640342801292718, 0.719218548625682352337662450736 -0.694783908357675672462505644944, 0.720309024887904714162800701160 -0.693653305812807263741603946983, 0.721397723859211970065530294960 -0.692520991747360814549949736829, 0.722484642853347680535591734952 -0.691386968955208680576163260412, 0.723569779188447226303537718195 -0.690251240234439400467181258136, 0.724653130187044469678880886931 -0.689113808387350701423201826401, 0.725734693176078748955148967070 -0.687974676220442615814931741625, 0.726814465486900762591915281519 -0.686833846544410486778531321761, 0.727892444455279563619853888667 -0.685691322174138084832861750328, 0.728968627421409443023492258362 -0.684547105928690835519034862955, 0.730043011729915702900939322717 -0.683401200631308602950753083860, 0.731115594729862094958150464663 -0.682253609109398584386951824854, 0.732186373774756371624050643732 -0.681104334194528981960559121944, 0.733255346222557946589404309634 -0.679953378722421342139625721757, 0.734322509435683556944240990561 -0.678800745532943894389177330595, 0.735387860781013813493700581603 -0.677646437470104445743857013440, 0.736451397629900528229995870788 -0.676490457382043386402870055463, 0.737513117358171932380628277315 -0.675332808121026584302626361023, 0.738573017346140336947257765132 -0.674173492543438612756290240213, 0.739631094978607794843128431239 -0.673012513509775534004120345344, 0.740687347644872873253518719139 -0.671849873884637460719204682391, 0.741741772738737314973889169778 -0.670685576536722227736220247607, 0.742794367658511811569610472361 -0.669519624338817731512563113938, 0.743845129807023108803321065352 -0.668352020167795157767898217571, 0.744894056591620112861562574835 -0.667182766904601765034499294416, 0.745941145424180218626020177908 -0.666011867434253779229891279101, 0.746986393721115859989367891103 -0.664839324645829066184887778945, 0.748029798903380616081904008752 -0.663665141432460581327745785529, 0.749071358396476094654303778952 -0.662489320691328598122993298603, 0.750111069630457705237347454386 -0.661311865323653935710979112628, 0.751148930039941320480068043253 -0.660132778234690631435910290747, 0.752184937064109604420991672669 -0.658952062333718946440797026298, 0.753219088146718118714773027023 -0.657769720534037927173187654262, 0.754251380736101983970343098918 -0.656585755752958522002415975294, 0.755281812285181874955242165015 -0.655400170911795920680731342145, 0.756310380251470237844557686913 -0.654212968935863115049755833752, 0.757337082097077840536769599566 -0.653024152754462794412404491595, 0.758361915288720100924990674685 -0.651833725300880795217040031275, 0.759384877297722971078997034056 -0.650641689512378440518602928933, 0.760405965600029265516468512942 -0.649448048330185656595858745277, 0.761425177676205100496531485987 -0.648252804699493312412528212008, 0.762442511011446222290999230609 -0.647055961569446336234534555842, 0.763457963095583447277192590263 -0.645857521893136055091133584938, 0.764471531423089989409902500483 -0.644657488627593311392161012918, 0.765483213493086456225000802078 -0.643455864733780913411465007812, 0.766493006809348176311402767169 -0.642252653176586307814943666017, 0.767500908880310417359282837424 -0.641047856924814474233187411301, 0.768506917219075047498222374998 -0.639841478951180375744911543734, 0.769511029343416308456937713345 -0.638633522232301742427296176174, 0.770513242775787587923730370676 -0.637423989748691632861721245717, 0.771513555043326859639307713223 -0.636212884484751328706408912694, 0.772511963677862900645720856119 -0.635000209428762563135251184576, 0.773508466215921619557605026785 -0.633785967572880526432754777488, 0.774503060198732273811117465812 -0.632570161913126316477473665145, 0.775495743172232909756758090225 -0.631352795449379611270046552818, 0.776486512687077023997517244425 -0.630133871185371008394326963753, 0.777475366298639336548603750998 -0.628913392128675030612328100688, 0.778462301567021897064080349082 -0.627691361290702354303050469753, 0.779447316057059969018894207693 -0.626467781686692704035124279471, 0.780430407338328246957814826601 -0.625242656335706969983334602148, 0.781411572985146740677464549663 -0.624015988260620435568171160412, 0.782390810576586659408349078149 -0.622787780488114339760841176030, 0.783368117696476740086097834137 -0.621558036048669437789726543997, 0.784343491933408576421982161264 -0.620326757976557785490001606377, 0.785316930880743502285668000695 -0.619093949309835744898578013817, 0.786288432136617476686524241813 -0.617859613090336101670629886939, 0.787257993303947745111770473159 -0.616623752363660848629933752818, 0.788225611990438612686205033242 -0.615386370179173303185393706372, 0.789191285808587106309630598844 -0.614147469589991112925986271875, 0.790155012375688969861187160859 -0.612907053652978262014983101835, 0.791116789313844881448289925174 -0.611665125428737854740290913469, 0.792076614249965671454845050903 -0.610421687981604232930976650096, 0.793034484815778872857094938809 -0.609176744379635870529909880133, 0.793990398647834050294136432058 -0.607930297694607157943380570941, 0.794944353387508684249951329548 -0.606682351002001407636043950333, 0.795896346681014499324646749301 -0.605432907381003082569748130481, 0.796846376179402682282670866698 -0.604181969914490135664664194337, 0.797794439538569766234843427810 -0.602929541689026460282718744565, 0.798740534419263625842688725243 -0.601675625794854007644119064935, 0.799684658487089250478163648950 -0.600420225325885681399995519314, 0.800626809412514406361083274533 -0.599163343379697455048926713062, 0.801566984870875298696546451538 -0.597904983057520489353464654414, 0.802505182542382455856966316787 -0.596645147464233915890474690968, 0.803441400112126391519495882676 -0.595383839708356621400753283524, 0.804375635270083266803453625471 -0.594121062902040142361670405080, 0.805307885711120774452353998640 -0.592856820161060893425997164741, 0.806238149135003578926728096121 -0.591591114604811951771523581556, 0.807166423246399089563851703133 -0.590323949356296173718305908551, 0.808092705754883344759775809507 -0.589055327542117646011377019022, 0.809016994374946230017542347923 -0.587785252292474691415691268048, 0.809939286825997584173819632269 -0.586513726741151764088044728851, 0.810859580832372306424815633363 -0.585240754025511678015902816696, 0.811777874123336573575215879828 -0.583966337286488057500832837832, 0.812694164433092836041794271296 -0.582690479668577676619634075905, 0.813608449500785924080048516771 -0.581413184319832132551653103292, 0.814520727070508265832415872865 -0.580134454391850962196031105123, 0.815430994891305771510303657124 -0.578854293039772982432111803064, 0.816339250717182829397700061236 -0.577572703422269073669781391800, 0.817245492307108745144716976938 -0.576289688701534408288296162937, 0.818149717425022404704293421673 -0.575005252043280123963597816328, 0.819051923839838047491923589405 -0.573719396616725885174048471526, 0.819952109325451261589989826462 -0.572432125594592333683863216720, 0.820850271660743757706768519711 -0.571143442153092761870425420057, 0.821746408629589253358460609888 -0.569853349471925230140811891033, 0.822640518020858801939709792350 -0.568561850734265461504435279494, 0.823532597628426454861028105370 -0.567268949126757959788847074378, 0.824422645251174368574709205859 -0.565974647839509015234682465234, 0.825310658692998577734556420182 -0.564678950066078488845278116059, 0.826196635762814213244098482392 -0.563381859003471707758592401660, 0.827080574274560831327107734978 -0.562083377852132026752940419101, 0.827962472047208186687328179687 -0.560783509815932501574309299031, 0.828842326904761006467481365689 -0.559482258102168339419790754619, 0.829720136676264985453599365428 -0.558179625921548905331803780427, 0.830595899195811671056333125307 -0.556875616488189395525409963739, 0.831469612302544347492982979020 -0.555570233019603620938653421035, 0.832341273840662476679597148177 -0.554263478736695458515271184297, 0.833210881659428137524514568213 -0.552955356863751190665823287418, 0.834078433613170244775858463981 -0.551645870628431622684217927599, 0.834943927561290766270474250632 -0.550335023261763645052724314155, 0.835807361368269385870632959268 -0.549022817998133016992312604998, 0.836668732903668943556851900212 -0.547709258075275817745364292932, 0.837528040042140875520715326275 -0.546394346734270341947592442011, 0.838385280663430210168485245958 -0.545078087219529883178381624020, 0.839240452652380897191619624209 -0.543760482778793741154288454709, 0.840093553898941025614988120651 -0.542441536663120005279381530272, 0.840944582298168263889692752855 -0.541121252126877116950254276162, 0.841793535750234522829771321994 -0.539799632427735875950247645960, 0.842640412160431395705018076114 -0.538476680826661446843672820251, 0.843485209439175820378409298428 -0.537152400587905476392336368008, 0.844327925502014298153596882912 -0.535826794978997877905158020440, 0.845168558269628777956938847638 -0.534499867270738504565485982312, 0.846007105667841430296505222941 -0.533171620737189821959134405915, 0.846843565627619976332596252178 -0.531842058655667915267883927299, 0.847677936085082461836748279893 -0.530511184306735272819821602752, 0.848510214981502919329159340123 -0.529179000974191793282841445034, 0.849340400263315808970787657017 -0.527845511945067569214984359860, 0.850168489882121458656172308110 -0.526510720509614227324846069678, 0.850994481794691171039346500038 -0.525174629961296823843497350026, 0.851818373962971886470540994196 -0.523837243596786183985614115954, 0.852640164354091512066702307493 -0.522498564715950020165280420770, 0.853459850940364139759708450583 -0.521158596621845493501723467489, 0.854277431699294487188467428496 -0.519817342620710554079721532617, 0.855092904613583337791737903899 -0.518474806021956280410734052566, 0.855906267671132314767135085276 -0.517130990138158219693309547438, 0.856717518865048988097044002643 -0.515785898285048505229610782408, 0.857526656193651648507625395723 -0.514439533781507529752730079053, 0.858333677660474303472426527151 -0.513091899949555840798609551712, 0.859138581274271784238294458191 -0.511743000114345925055658881320, 0.859941365049024408762079474400 -0.510392837604154214758978014288, 0.860742027003943088736548361339 -0.509041415750372316928462623764, 0.861540565163473881504785367724 -0.507688737887499352829934196052, 0.862336977557303430153012868686 -0.506334807353133520280152879423, 0.863131262220363182358084941370 -0.504979627487963766974132795440, 0.863923417192834719458005565684 -0.503623201635761796879364737833, 0.864713440520154530410934512474 -0.502265533143373521518526558793, 0.865501330253018452687285844149 -0.500906625360710844319100942812, 0.866287084447386890317943652917 -0.499546481640743833541051799330, 0.867070701164489476830965486442 -0.498185105339491784981476030225, 0.867852178470829960232890698535 -0.496822499816015394902279922462, 0.868631514438190754923141412291 -0.495458668432408433357494459415, 0.869408707143637715653028408269 -0.494093614553789195475985707162, 0.870183754669525133529361937690 -0.492727341548292452344526282104, 0.870956655103500398951155148097 -0.491359852787061013312808199771, 0.871727406538508442501722583984 -0.489991151644237454831909417408, 0.872496007072796619929988537478 -0.488621241496955793781609145299, 0.873262454809919708154097861552 -0.487250125725333160797703158096, 0.874026747858744013086607083096 -0.485877807712461418088167874885, 0.874788884333452365638095216127 -0.484504290844398777249324439254, 0.875548862353548784653867187444 -0.483129578510161694637758955650, 0.876306680043863139850657262286 -0.481753674101716100608427950647, 0.877062335534555592708727544959 -0.480376581013969350397729840552, 0.877815826961121259408571404492 -0.478998302644761786428517780223, 0.878567152464394984789919362811 -0.477618842394858300615112511878, 0.879316310190555894266140057880 -0.476238203667939896668315213901, 0.880063298291131501649431356782 -0.474856389870595307911571580917, 0.880808114923003260265943481500 -0.473473404412312892652892060141, 0.881550758248409893624852884386 -0.472089250705471641378352387619, 0.882291226434952946533485373948 -0.470703932165333294168618749609, 0.883029517655600670877902302891 -0.469317452210033625448204475106, 0.883765630088693021626511381328 -0.467929814260574061801634115909, 0.884499561917945764655257789855 -0.466541021740813355300758757949, 0.885231311332454806617420217663 -0.465151078077459034787466407579, 0.885960876526701524014129063289 -0.463759986700058801645241146616, 0.886688255700556204885742772603 -0.462367751040992425171083368696, 0.887413447059282933793156189495 -0.460974374535462971813615240535, 0.888136448813544254754503981530 -0.459579860621488478500396013260, 0.888857259179405057025746828003 -0.458184212739893403920632408699, 0.889575876378337682126584695652 -0.456787434334300079807889005679, 0.890292298637225365531833176647 -0.455389528851120495289706013864, 0.891006524188367565741941689339 -0.453990499739547359592251041249, 0.891718551269483628018974741281 -0.452590350451545997412239330515, 0.892428378123717669367920279910 -0.451189084441845578155039220292, 0.893136002999642242272670955572 -0.449786705167930678239684993969, 0.893841424151263552744239859749 -0.448383216090032787892738497249, 0.894544639838024790989834400534 -0.446978620671121651408697061925, 0.895245648324811460483374503383 -0.445572922376896773943855123434, 0.895944447881954486589961561549 -0.444166124675779094843619532185, 0.896641036785235656658699099353 -0.442758231038901994836010089784, 0.897335413315890950691766647651 -0.441349244940103135892428554143, 0.898027575760615426325728094525 -0.439939169855915579443461638220, 0.898717522411566993589815410814 -0.438528009265559515217347552607, 0.899405251566370855798027150740 -0.437115766650933323944627773017, 0.900090761528123728396622027503 -0.435702445496605361707764814128, 0.900774050605397835767007563845 -0.434288049289805022645793997071, 0.901455117112245463140141055192 -0.432872581520414356770487529502, 0.902133959368202620332510832668 -0.431456045680959354715611198117, 0.902810575698293593660537226242 -0.430038445266601454530785986208, 0.903484964433034609676553827740 -0.428619783775128715408442303669, 0.904157123908438831172418304050 -0.427200064706947324477681604549, 0.904827052466019354781678885047 -0.425779291565072992575835542084, 0.905494748452794095960882714280 -0.424357467855122183486571429967, 0.906160210221289674770162037021 -0.422934597085303565222602628637, 0.906823436129545190631517925794 -0.421510682766409350286096469063, 0.907484424541116774243221243523 -0.420085728411806524906779714001, 0.908143173825081140293491444027 -0.418659737537428355835800175555, 0.908799682356039917330292610131 -0.417232713661765564072680945173, 0.909453948514123644564222104236 -0.415804660305857665125728317435, 0.910105970684995546626794293843 -0.414375580993284364783590945081, 0.910755747259855752417934127152 -0.412945479250156843864516531539, 0.911403276635445180886563321110 -0.411514358605108987454457292188, 0.912048557214049315788884086942 -0.410082222589288725167477878131, 0.912691587403502757602780093293 -0.408649074736349260383860837464, 0.913332365617192110107680491637 -0.407214918582440466021665770313, 0.913970890274061087410473191994 -0.405779757666200169285986021350, 0.914607159798613511547671350854 -0.404343595528745158862449216031, 0.915241172620917531332906946773 -0.402906435713662802733381340659, 0.915872927176609508137516968418 -0.401468281767002110882458509877, 0.916502421906897901671129602619 -0.400029137237264909021661196675, 0.917129655258567044739947959897 -0.398589005675397289873984618680, 0.917754625683981140049638725031 -0.397147890634780731389241736906, 0.918377331641087479852103570011 -0.395705795671223325982168717019, 0.918997771593421330926787504723 -0.394262724342951065281681621855, 0.919615944010108599115937977331 -0.392818680210599013857830641427, 0.920231847365870270216703374899 -0.391373666837202482948754322933, 0.920845480141026184739416748926 -0.389927687788188315209936263273, 0.921456840821498479598972153326 -0.388480746631366113952310570312, 0.922065927898815251850805907452 -0.387032846936919361358064861633, 0.922672739870114777538390171685 -0.385583992277396536696443263281, 0.923277275238148953384609285422 -0.384134186227702567606456796057, 0.923879532511286738483136105060 -0.382683432365089781779232680492, 0.924479510203518262123623117077 -0.381231734269149025173817335599, 0.925077206834458043438473850983 -0.379779095521801057788735533904, 0.925672620929349210250336454919 -0.378325519707287671877793400199, 0.926265751019066718718875108607 -0.376871010412162588121276485253, 0.926856595640120795032146361336 -0.375415571225283017930962614628, 0.927445153334661376298697632592 -0.373959205737800337576715037358, 0.928031422650480664060523849912 -0.372501917543151706002646506022, 0.928615402141017343140561024484 -0.371043710237050905487166119201, 0.929197090365360245378667514160 -0.369584587417479404347631088967, 0.929776485888251458256092973897 -0.368124552684677808223057127179, 0.930353587280090099653762081289 -0.366663609641136645223014056683, 0.930928393116935759543650874548 -0.365201761891587650676882503831, 0.931500901980512385769372940558 -0.363739013042994774327354434718, 0.932071112458211059603740977764 -0.362275366704545520590841078956, 0.932639023143094214596260371763 -0.360810826487641567172914847106, 0.933204632633898634175295683235 -0.359345396005890493906775873256, 0.933767939535039226406354373466 -0.357879078875096179324089007423, 0.934328942456612132616555754794 -0.356411878713250473982299126874, 0.934887640014398391130612253619 -0.354943799140523874591224284814, 0.935444030829867378962205748394 -0.353474843779256864273463634163, 0.935998113530180142483061445091 -0.352005016253950697713293038760, 0.936549886748192506047416827641 -0.350534320191258741417072997137, 0.937099349122458957772607845982 -0.349062759219977258862144253726, 0.937646499297235758163537866494 -0.347590336971036750757235722631, 0.938191335922484270781751547474 -0.346117057077492629169057636318, 0.938733857653874181892206252087 -0.344642923174516724316163163166, 0.939274063152787164199253311381 -0.343167938899387792162087862380, 0.939811951086319763426502049697 -0.341692107891482965698060070281, 0.940347520127287062052801047685 -0.340215433792268484580745280255, 0.940880768954225565892102167709 -0.338737920245290979881502835269, 0.941411696251396978851744279382 -0.337259570896168203724130307819, 0.941940300708790756445409897424 -0.335780389392580258522968961188, 0.942466581022128102596013832226 -0.334300379384260382131799360650, 0.942990535892864634170962290227 -0.332819544522986177081946834733, 0.943512164028193711651226749382 -0.331337888462570562264630780192, 0.944031464141049880822720297147 -0.329855414858852391546406579437, 0.944548434950111648333859193372 -0.328372127369688071585329680602, 0.945063075179805034409241670801 -0.326888029654941958401792589939, 0.945575383560306237384907035448 -0.325403125376477697638932795599, 0.946085358827545519488921854645 -0.323917418198148898689225916314, 0.946592999723209427287429207354 -0.322430911785790363932591162666, 0.947098304994744455420629947184 -0.320943609807208929396438179538, 0.947601273395360266249554115348 -0.319455515932174360926865119836, 0.948101903684032243369017578516 -0.317966633832410361382159180721, 0.948600194625504822276695904293 -0.316476967181585522315145908578, 0.949096144990294821042198236682 -0.314986519655304220144387272740, 0.949589753554693882797721471434 -0.313495294931097512325379739195, 0.950081019100771917429426594026 -0.312003296688414255566357269345, 0.950569940416380210201907630108 -0.310510528608611724443733237422, 0.951056516295153753226543358323 -0.309016994374946785129054660501, 0.951540745536515131242083498364 -0.307522697672566680537897809700, 0.952022626945676853083000423794 -0.306027642188500037523368746406, 0.952502159333644349281655649975 -0.304531831611647596513847702226, 0.952979341517219080692768784502 -0.303035269632773274217640846473, 0.953454172319001425073281552613 -0.301537959944494948771875897364, 0.953926650567393785706826747628 -0.300039906241275522447153889516, 0.954396775096602922872079943772 -0.298541112219413484751839860110, 0.954864544746643173489530909137 -0.297041581577034141670168310156, 0.955329958363339670768255018629 -0.295541318014080345299987584440, 0.955793014798330342607357579254 -0.294040325232303167979353020201, 0.956253712909069686354257555649 -0.292538606935253242546934870916, 0.956712051558830767206131895364 -0.291036166828271047890552836179, 0.957168029616708548878989404329 -0.289533008618478249207583985481, 0.957621645957622447120627384720 -0.288029136014768372131555906890, 0.958072899462319438335100585391 -0.286524552727797476858739855743, 0.958521789017376169006467989675 -0.285019262469975276363953753389, 0.958968313515202286367866690853 -0.283513268955455866038306567134, 0.959412471854043102936770992528 -0.282006575900128564349245152698, 0.959854262937981927983344121458 -0.280499187021608642478298634160, 0.960293685676943287177209640504 -0.278991106039228331514578940187, 0.960730738986695254055803161464 -0.277482336674027607603676415238, 0.961165421788852225581933907961 -0.275972882648744810563101736989, 0.961597733010877475656741353305 -0.274462747687807706586937683824, 0.962027671586086152721861708415 -0.272951935517324273394734746034, 0.962455236453647611227779634646 -0.271440449865073263335801811991, 0.962880426558587854124482419138 -0.269928294460495321605009166888, 0.963303240851792752508231387765 -0.268415473034683604858230410173, 0.963723678290009933000703767902 -0.266901989320374566361238066747, 0.964141737835851997395764101384 -0.265387847051938796649750429424, 0.964557418457798410038606107264 -0.263873049965371864189478401386, 0.964970719130198495427919169742 -0.262357601798284989502718644871, 0.965381638833274213773449901055 -0.260841506289895885828400423634, 0.965790176553121715308236616693 -0.259324767181019488759829982882, 0.966196331281715004024590598419 -0.257807388214058796904737391742, 0.966600102016907603008633032005 -0.256289373132995546011869691938, 0.967001487762435329997856570117 -0.254770725683381049631037740255, 0.967400487527918850894081970182 -0.253251449612326984262011819737, 0.967797100328865789187204882182 -0.251731548668495952458812325858, 0.968191325186673501512757411547 -0.250211026602092490023210302752, 0.968583161128631409120259831980 -0.248689887164853629109018129384, 0.968972607187923218319269835774 -0.247168134110039711126560746379, 0.969359662403629585014641634189 -0.245645771192425171891571267224, 0.969744325820730113107970282726 -0.244122802168289132485057280064, 0.970126596490106130055153244029 -0.242599230795406295424498921420, 0.970506473468542796290137175674 -0.241075060833037535523715177987, 0.970883955818731325670967180486 -0.239550296041920712797335113464, 0.971259042609271538992743444396 -0.238024940184261429854117864124, 0.971631732914674195455972949276 -0.236498997023723595001243324987, 0.972002025815362991068013798213 -0.234972470325420262904358992273, 0.972369920397676890111426928343 -0.233445363855904336469748727723, 0.972735415753872678656932748709 -0.231917681383159240970925907277, 0.973098510982126851942553003028 -0.230389426676589542664075338507, 0.973459205186538056864264945034 -0.228860603507011900470402565588, 0.973817497477129090377445663762 -0.227331215646645434791395246066, 0.974173386969849564032131183922 -0.225801266869102706946748071459, 0.974526872786577347262948478601 -0.224270760949380199011926606545, 0.974877954055121453968979494675 -0.222739701663849182233789747443, 0.975226629909223596825995628024 -0.221208092790246169112577945270, 0.975572899488560962844019286422 -0.219675938107663865084262511118, 0.975916761938747656657255902246 -0.218143241396541620602533839701, 0.976258216411337254037050570332 -0.216610006438656216287697020562, 0.976597262063824800293332373258 -0.215076237017112481542113755495, 0.976933898059648919698361169139 -0.213541936916333996432371122864, 0.977268123568193702865869454399 -0.212007109922053765815874726286, 0.977599937764790816174809151562 -0.210471759821304810200714996427, 0.977929339830722055282308247115 -0.208935890402410867627835955318, 0.978256328953220233302090491634 -0.207399505454977040042052749413, 0.978580904325472289428944350220 -0.205862608769880522929796029530, 0.978903065146620843250957477721 -0.204325204139261029645524558873, 0.979222810621765971106356118980 -0.202787295356511687582923286755, 0.979540139961967537551856821665 -0.201248886216269573523618419131, 0.979855052384246971719505836518 -0.199709980514406248985892489145, 0.980167547111589265718123442639 -0.198170582048018628640306815214, 0.980477623372944528945538422704 -0.196630694615419321369387262166, 0.980785280403230541601544700825 -0.195090322016127554194397930587, 0.981090517443334197977833355253 -0.193549468050859568846178149215, 0.981393333740113393837134481146 -0.192008136521519351402886854885, 0.981693728546399024814661515848 -0.190466331231189223149868894325, 0.981991701120996651752648176625 -0.188924055984120486950672557214, 0.982287250728688832168700173497 -0.187381314585723990351340262350, 0.982580376640236008434214909357 -0.185838110842560771951426090709, 0.982871078132379283331943042867 -0.184294448562332680019437702867, 0.983159354487841641301315576129 -0.182750331553872935597127025176, 0.983445204995329835817585717450 -0.181205763627136778870507782813, 0.983728628949535943704063356563 -0.179660748593191976762994954697, 0.984009625651139807622769239970 -0.178115290264209580328724769060, 0.984288194406809924252854671067 -0.176569392453454349078967311470, 0.984564334529205442692045835429 -0.175023058975275508375446520404, 0.984838045336978273880390588602 -0.173476293645097173756752795271, 0.985109326154773978778678156232 -0.171929100279409052820511760729, 0.985378176313234321881395771925 -0.170381482695756925060948105966, 0.985644595148998159395148377371 -0.168833444712733399262205580271, 0.985908582004703437640102947626 -0.167284990149968282313608369805, 0.986170136228988969406827891362 -0.165736122828119253336254246278, 0.986429257176495544179317676026 -0.164186846568862510054032100015, 0.986685944207868148581042078149 -0.162637165194883193120034547974, 0.986940196689756965575668345991 -0.161087082529866032487575466803, 0.987192013994819261846203062305 -0.159536602398485910514480679012, 0.987441395501720942107226619555 -0.157985728626398452822954254771, 0.987688340595137770350220307591 -0.156434465040230508137142351188, 0.987932848665757479267313101445 -0.154882815467570766898575129744, 0.988174919110280547407398898940 -0.153330783736960296614881826827, 0.988414551331422308599883308489 -0.151778373677883132719657055532, 0.988651744737914062177708274248 -0.150225589120756758410024644945, 0.988886498744504627289586551342 -0.148672433896922695506503941942, 0.989118812771961897212236181076 -0.147118911838637095312876112985, 0.989348686247073949573405116098 -0.145565026779061190698172367775, 0.989576118602651044753315545677 -0.144010782552251914712115876682, 0.989801109277526292018478670798 -0.142456182993152380422685610029, 0.990023657716557647923139029444 -0.140901231937582444020407024254, 0.990243763370628915509996659239 -0.139345933222229295678218363719, 0.990461425696651298622441572661 -0.137790290684637883877883268724, 0.990676644157564623149880844721 -0.136234308163201534025432692943, 0.990889418222338669295368163148 -0.134677989497152428288728742700, 0.991099747365974836910140766122 -0.133121338526552196457330978774, 0.991307631069506589582829292340 -0.131564359092282368024484640046, 0.991513068820001675085507031326 -0.130007055036034907535835714043, 0.991716060110562902529807161045 -0.128449430200302749938146007480, 0.991916604440329363612249835569 -0.126891488428370335928008216797, 0.992114701314477875904174197785 -0.125333233564304175056136614330, 0.992310350244224093074763004552 -0.123774669452943172909265001636, 0.992503550746823726136369714368 -0.122215799939889374625678897246, 0.992694302345573764689845575049 -0.120656628871498333710476913438, 0.992882604569813698169866711396 -0.119097160094869675139861442403, 0.993068456954926293001051362808 -0.117537397457837589076490303341, 0.993251859042339368954799283529 -0.115977344808961310707040581747, 0.993432810379526576305408980261 -0.114417005997515697224287123390, 0.993611310520008395030799874803 -0.112856384873481624397939526716, 0.993787359023353467080141854240 -0.111295485287536619067871868083, 0.993960955455179706596879896097 -0.109734311091045214081596270717, 0.994132099387155077074851305952 -0.108172866136049553031917014323, 0.994300790396998923625915267621 -0.106611154275259856216706566556, 0.994467028068482972180675005802 -0.105049179362044886598681614487, 0.994630811991432328689199948712 -0.103486945250422471276330327328, 0.994792141761726478321747890732 -0.101924455795049981321476195717, 0.994951016981300173647184692527 -0.100361714851214839372417486629, 0.995107437258144655878311368724 -0.098798726274824957838127659215, 0.995261402206308321005678863003 -0.097235493922399274246970435343, 0.995412911445898163087520060799 -0.095672021651058244962051446691, 0.995561964603079996294354714337 -0.094108313318514269507630842782, 0.995708561310080120243526380364 -0.092544372783062198162262745882, 0.995852701205185653066109807696 -0.090980203903569867307510321552, 0.995994383932745863674540487409 -0.089415810539468468243207155410, 0.996133609143172504829522040382 -0.087851196550743124169535747114, 0.996270376492941256429958230001 -0.086286365797923328391227926204, 0.996404685644592391646767737257 -0.084721322142073396399553075753, 0.996536536266731332034396473318 -0.083156069444782973465457587281, 0.996665928034029868776144667208 -0.081590611568157500599340892222, 0.996792860627226606773376715864 -0.080024952374808666633043685579, 0.996917333733127963846243346779 -0.078459095727844901935199573018, 0.997039347044608947889798855613 -0.076893045490861802737647678896, 0.997158900260613934030118343799 -0.075326805527932680361935524616, 0.997275993086157108713507568609 -0.073760379703598930034580405390, 0.997390625232323690951830030826 -0.072193771882860552357996652972, 0.997502796416270043344809437258 -0.070626985931166647025847282748, 0.997612506361225226392264175956 -0.069060025714405753882729754878, 0.997719754796490665427199928672 -0.067492895098896485417405699536, 0.997824541457441482883439221041 -0.065925597951377812311335446793, 0.997926866085527053407133735163 -0.064358138138999682054119944041, 0.998026728428271558968276622181 -0.062790519529313332247610901504, 0.998124128239274543972214814858 -0.061222745990261881465777094036, 0.998219065278211803438068727701 -0.059654821390170663375496218350, 0.998311539310835382998732256965 -0.058086749597737775963057771378, 0.998401550108975022190804793354 -0.056518534482024491982787850475, 0.998489097450537932409986296989 -0.054950179912445711039037377077, 0.998574181119509796111799460050 -0.053381689758760439423745935983, 0.998656800905954988856194631808 -0.051813067891062200565066575564, 0.998736956606017467485969518748 -0.050244318179769521803823550954, 0.998814648021921103193676572118 -0.048675444495616379536606643796, 0.998889874961970014588530375477 -0.047106450709642630481077674176, 0.998962637240549122807919957268 -0.045537340693184463757958724273, 0.999032934678124706628921103402 -0.043968118317864873789702073736, 0.999100767101244957579808669834 -0.042398787455584070749114999899, 0.999166134342540090962359045079 -0.040829351978509967335817520961, 0.999229036240722900963362462790 -0.039259815759068582285973292301, 0.999289472640589204743832851818 -0.037690182669934513270959541842, 0.999347443393018286528217686282 -0.036120456584021375101567485899, 0.999402948354972897604397985560 -0.034550641374472244871096648922, 0.999455987389500144502108014422 -0.032980740914650100159555279333, 0.999506560365731600015237745538 -0.031410759078128271115648573186, 0.999554667158883303201832859486 -0.029840699738680861313744330232, 0.999600307650256536540211982356 -0.028270566770273227591436793205, 0.999643481727238047973571610783 -0.026700364047052383559277544123, 0.999684189283299939887683649431 -0.025130095443337455152210679898, 0.999722430218000557289315111120 -0.023559764833610129242114084036, 0.999758204436984043717018266761 -0.021989376092505084903105938565, 0.999791511851981118397247882967 -0.020418933094800435085192091833, 0.999822352380808965222058759537 -0.018848439715408154410125618483, 0.999850725947371787860618042032 -0.017277899829364548600629802877, 0.999876632481660587714600296749 -0.015707317311820654520682083444, 0.999900071919753496985094898264 -0.014136696038032716543630940009, 0.999921044203816111739513416978 -0.012566039883352591796650266076, 0.999939549282101380889287156606 -0.010995352723218209181621496384, 0.999955587108949828234472079203 -0.009424638433143995436269690913, 0.999969157644789663486051267682 -0.007853900888711325481428282558, 0.999980260856137115332842313364 -0.006283143965558944145366471190, 0.999988896715595987352287465910 -0.004712371539373417378415709322, 0.999995065201858213121965945902 -0.003141587485879560048829262087, 0.999998766299703523152686557296 -0.001570795680830877182748706034, 1.000000000000000000000000000000 0.000000000000000000000000000000, 20000000.000000000000000000000000000000 0.000000000000000000000000000000, 20000000.000000000000000000000000000000 -20000000.000000000000000000000000000000, 0.000000000000030481343214686657 -20000000.000000000000000000000000000000))";

      GeomPtr isect{readWKT(clip)->intersection(readWKT(inp).get())};
      std::string exp = wktwriter.write(isect.get());

      doClipTest(inp, exp, r, 1e-20);
    }
}
