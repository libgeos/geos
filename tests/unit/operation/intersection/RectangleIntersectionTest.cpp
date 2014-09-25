// 
// Test Suite for geos::operation::intersection::RectangleIntersection class.

// tut
#include <tut.hpp>
// geos
#include <geos/operation/intersection/Rectangle.h>
#include <geos/operation/intersection/RectangleIntersection.h>
#include <geos/geom/GeometryFactory.h>
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
        geos::geom::GeometryFactory gf;
        geos::io::WKTReader wktreader;
        geos::io::WKTWriter wktwriter;

        typedef geos::geom::Geometry::AutoPtr GeomPtr;
        typedef geos::geom::Geometry Geom;
        typedef geos::operation::intersection::Rectangle Rectangle;
        typedef geos::operation::intersection::RectangleIntersection RectangleIntersection;

        test_rectangleintersectiontest_data()
          : gf(),
            wktreader(&gf)
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
}
