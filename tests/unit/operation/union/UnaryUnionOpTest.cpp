// $Id$
// 
// Test Suite for geos::operation::geounion::UnaryUnionOp class.

// tut
#include <tut.hpp>
// geos
#include <geos/operation/union/UnaryUnionOp.h>
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
    struct test_unaryuniontest_data
    {
        geos::geom::GeometryFactory gf;
        geos::io::WKTReader wktreader;
        geos::io::WKTWriter wktwriter;

        typedef geos::geom::Geometry::AutoPtr GeomPtr;
        typedef geos::geom::Geometry Geom;
        typedef geos::operation::geounion::UnaryUnionOp UnaryUnionOp;

        test_unaryuniontest_data()
          : gf(),
            wktreader(&gf)
        {
          wktwriter.setTrim(true);
        }

        void delAll(std::vector<Geom*>& geoms)
        {
          for (size_t i=0; i<geoms.size(); ++i) delete geoms[i];
        }

        GeomPtr readWKT(const std::string& inputWKT)
        {
            return GeomPtr(wktreader.read(inputWKT));
        }

        void readWKT(const char* const* inputWKT, std::vector<Geom*>& geoms)
        {
          for (const char* const* ptr=inputWKT; *ptr; ++ptr) {
            geoms.push_back(readWKT(*ptr).release());
          }
        }

        static GeomPtr normalize(const Geom& g)
        {
          GeomPtr g2 ( g.clone() );
          g2->normalize();
          return g2;
        }

        bool isEqual(const Geom& a, const Geom& b)
        {
          using std::cout;
          using std::endl;
          GeomPtr a2 = normalize(a);
          GeomPtr b2 = normalize(b);
          bool eq = a2->equalsExact(b2.get());
          if  ( ! eq ) {
            cout << "OBTAINED: " << wktwriter.write(b2.get()) << endl;
          }
          return eq;
        }

        void doTest(const char* const* inputWKT, const std::string& expectedWKT)
        {
          std::vector<Geom*> geoms;
          readWKT(inputWKT, geoms);

          GeomPtr result;
          if ( geoms.empty() )
            result = UnaryUnionOp::Union(geoms, gf);
          else
            result = UnaryUnionOp::Union(geoms);

          bool ok = isEqual(*readWKT(expectedWKT), *result);
          delAll(geoms);

          ensure(ok);
        }

    };

    typedef test_group<test_unaryuniontest_data> group;
    typedef group::object object;

    group test_unaryuniontest_group("geos::operation::geounion::UnaryUnionOp");

    template<>
    template<>
    void object::test<1>()
    {
        static char const* const geoms[] = { NULL };
        doTest(geoms, "GEOMETRYCOLLECTION EMPTY");
    }

    template<>
    template<>
    void object::test<2>()
    {
        static char const* const geoms[] = {
          "POINT (1 1)", "POINT (2 2)",
          NULL
        };
        doTest(geoms, "MULTIPOINT ((1 1), (2 2))");
    }

    template<>
    template<>
    void object::test<3>()
    {
        static char const* const geoms[] = {
          "GEOMETRYCOLLECTION (POLYGON ((0 0, 0 90, 90 90, 90 0, 0 0)),   POLYGON ((120 0, 120 90, 210 90, 210 0, 120 0)),  LINESTRING (40 50, 40 140),  LINESTRING (160 50, 160 140),  POINT (60 50),  POINT (60 140),  POINT (40 140))",
          NULL
        };
        doTest(geoms, "GEOMETRYCOLLECTION (POINT (60 140),   LINESTRING (40 90, 40 140), LINESTRING (160 90, 160 140), POLYGON ((0 0, 0 90, 40 90, 90 90, 90 0, 0 0)), POLYGON ((120 0, 120 90, 160 90, 210 90, 210 0, 120 0)))");
    }

    template<>
    template<>
    void object::test<4>()
    {
        static char const* const geoms[] = 
        {
            "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
            "MULTIPOLYGON (((20 0, 20 10, 40 10, 40 0, 20 0)),((5 5, 5 8, 8 8, 8 5, 5 5)))",
            "POINT (5 5)",
            "POINT (-5 5)",
            "LINESTRING (-10 -10, -10 0, -10 20)",
            "LINESTRING (-10 2, 10 2)",
            NULL
        };
        doTest(geoms, "GEOMETRYCOLLECTION (POLYGON ((0 0, 0 2, 0 10, 10 10, 10 2, 10 0, 0 0)), POLYGON ((20 0, 20 10, 40 10, 40 0, 20 0)), LINESTRING (-10 -10, -10 0, -10 2), LINESTRING (-10 2, 0 2), LINESTRING (-10 2, -10 20), POINT (-5 5))");

    }

    template<>
    template<>
    void object::test<5>()
    {
        static char const* const geoms[] = 
        {
            "LINESTRING (40 60, 120 110)",
            "POINT (120 110)",
            "POINT (40 60)",
            "POINT (100 70)",
            "POINT (80 50)",
            NULL
        };
        doTest(geoms, "GEOMETRYCOLLECTION (POINT (80 50), POINT (100 70), LINESTRING (40 60, 120 110))");
    }

    template<>
    template<>
    void object::test<6>()
    {
        static char const* const geoms[] = 
        {
            "LINESTRING (0 0, 10 0, 5 -5, 5 5)",
            NULL
        };
        doTest(geoms, "MULTILINESTRING ((0 0, 5 0), (5 0, 10 0, 5 -5, 5 0), (5 0, 5 5))");
    }

} // namespace tut

