// 
// Test Suite for geos::operation::polygonize::Polygonizer class.
//
// Port of junit/operation/polygonize/PolygonizeTest.java
//

// tut
#include <tut.hpp>
// geos
#include <geos/operation/polygonize/Polygonizer.h>
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
    struct test_polygonizetest_data
    {
        geos::geom::GeometryFactory gf;
        geos::io::WKTReader wktreader;
        geos::io::WKTWriter wktwriter;

        typedef geos::geom::Geometry::AutoPtr GeomPtr;
        typedef geos::geom::Geometry Geom;
        typedef geos::geom::Polygon Poly;
        typedef geos::operation::polygonize::Polygonizer Polygonizer;

        test_polygonizetest_data()
          : gf(),
            wktreader(&gf)
        {
          wktwriter.setTrim(true);
        }

        template <class T>
        void delAll(T& cnt)
        {
          for (typename T::iterator i=cnt.begin(), e=cnt.end(); i!=e; ++i) {
            delete *i;
          }
        }

        template <class T>
        void printAll(std::ostream& os, T& cnt)
        {
          for (typename T::iterator i=cnt.begin(), e=cnt.end(); i!=e; ++i) {
            os << **i;
          }
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

        template <class T>
        bool contains( T& cnt, const Geom* g) 
        {
          for (typename T::iterator i=cnt.begin(), e=cnt.end(); i!=e; ++i) {
            const Geom* element = *i;
            if (element->equalsExact(g)) {
              return true;
            }
          }
          return false;
        }

        template <class T, class S>
        bool compare( T& ex, S& ob) 
        {
          using std::cout;
          using std::endl;

          if ( ex.size() != ob.size() ) {
            cout << "Expected " << ex.size() << " polygons, obtained " 
                 << ob.size() << endl;
            return false;
          }
          for (typename T::iterator i=ex.begin(), e=ex.end(); i!=e; ++i) {
            if ( ! contains(ob, *i) ) {
              cout << "Expected " << wktwriter.write(*i)
                   << " not found" << endl;
              return false;
            }
          }

          return true;
        
        }

        bool doTest(const char* const* inputWKT, const char* const* expectWKT)
        {
          using std::cout;
          using std::endl;

          std::vector<Geom*> inputGeoms, expectGeoms;

          readWKT(inputWKT, inputGeoms);
          readWKT(expectWKT, expectGeoms);

          Polygonizer polygonizer;
          polygonizer.add(&inputGeoms);

          std::auto_ptr< std::vector<Poly*> > retGeoms;
          retGeoms.reset( polygonizer.getPolygons() );

          delAll(inputGeoms);

          bool ok = compare(expectGeoms, *retGeoms);
          if  ( ! ok ) {
            cout << "OBTAINED(" << retGeoms->size() << "): ";
            printAll(cout, *retGeoms);
            cout << endl;

            ensure( "not all expected geometries in the obtained set", 0 );
          }

          delAll(expectGeoms);
          delAll(*retGeoms);

          return ok;
        }

    };

    typedef test_group<test_polygonizetest_data> group;
    typedef group::object object;

    group test_polygonizetest_group("geos::operation::polygonize::Polygonizer");

    // test1() in JTS
    template<>
    template<>
    void object::test<1>()
    {
        static char const* const inp[] = {
            "LINESTRING EMPTY",
            "LINESTRING EMPTY",
            NULL
        };

        static char const* const exp[] = {
            NULL
        };

        doTest(inp, exp);
    }

    // test2() in JTS
    template<>
    template<>
    void object::test<2>()
    {
        static char const* const inp[] = {
            "LINESTRING (100 180, 20 20, 160 20, 100 180)",
            "LINESTRING (100 180, 80 60, 120 60, 100 180)",
            NULL
        };

        static char const* const exp[] = {
            "POLYGON ((100 180, 120 60, 80 60, 100 180))",
            "POLYGON ((100 180, 160 20, 20 20, 100 180), (100 180, 80 60, 120 60, 100 180))",
            NULL
        };

        doTest(inp, exp);
    }

} // namespace tut

