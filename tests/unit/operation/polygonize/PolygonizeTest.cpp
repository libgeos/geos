//
// Test Suite for geos::operation::polygonize::Polygonizer class.
//
// Port of junit/operation/polygonize/PolygonizeTest.java
//

// tut
#include <tut/tut.hpp>
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
        geos::io::WKTReader wktreader;
        geos::io::WKTWriter wktwriter;

        typedef geos::geom::Geometry::Ptr GeomPtr;
        typedef geos::geom::Geometry Geom;
        typedef geos::geom::Polygon Poly;
        typedef geos::operation::polygonize::Polygonizer Polygonizer;

        test_polygonizetest_data()
          : wktreader()
        {
          wktwriter.setTrim(true);
        }

        template <class T>
        void delAll(T& cnt)
        {
          for (const auto i : cnt) delete i;
        }

        template <class T>
        void printAll(std::ostream& os, T& cnt)
        {
          for (const auto i : cnt) os <<  wktwriter.write(i) << "\n";
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
          for (auto element : cnt) {
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
            cout << "\nExpected " << ex.size() << " polygons, obtained "
                 << ob.size() << endl;
            return false;
          }
          for (const auto i : ex ) {
            if ( !contains(ob, i) ) {
              cout << "Expected " << wktwriter.write(i)
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

          std::unique_ptr< std::vector<Poly*> > retGeoms;
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
    void object::test<1>() {
        static char const* const inp[] = {
            "LINESTRING EMPTY",
            "LINESTRING EMPTY",
            nullptr
        };

        static char const* const exp[] = {
            nullptr
        };

        doTest(inp, exp);
    }

    // test2() in JTS
    template<>
    template<>
    void object::test<2>() {
        static char const* const inp[] = {
            "LINESTRING (100 180, 20 20, 160 20, 100 180)",
            "LINESTRING (100 180, 80 60, 120 60, 100 180)",
            nullptr
        };

        static char const* const exp[] = {
            "POLYGON ((100 180, 120 60, 80 60, 100 180))",
            "POLYGON ((100 180, 160 20, 20 20, 100 180), (100 180, 80 60, 120 60, 100 180))",
            nullptr
        };

        doTest(inp, exp);
    }

    template<>
    template<>
    void object::test<3>() {
        static char const* const inp[] = {
					"LINESTRING (100 180, 20 20, 160 20, 100 180)",
					nullptr
				};

				static char const* const exp[] = {
					"POLYGON ((100 180, 160 20, 20 20, 100 180))",
					nullptr
				};

        doTest(inp, exp);
    }

    template<>
    template<>
    void object::test<4>() {
        static char const* const inp[] = {
            "LINESTRING EMPTY",
						nullptr
        };

        static char const* const exp[] = {
						nullptr
        };

        doTest(inp, exp);
    }

    template<>
    template<>
    void object::test<5>() {
        static char const* const inp[] = {
						nullptr
        };

        static char const* const exp[] = {
						nullptr
        };

        doTest(inp, exp);
    }

    template<>
    template<>
    void object::test<6>() {
        static char const* const inp[] = {
            "LINESTRING (100 180, 20 20, 160 20, 100 180)",
            "LINESTRING (100 180, 20 20, 160 20, 100 180)",
						nullptr
        };

        static char const* const exp[] = {
						nullptr
        };

        doTest(inp, exp);
    }

    template<>
    template<>
    void object::test<7>() {
        static char const* const inp[] = {
					"POLYGON ((100 180, 160 20, 20 20, 100 180))",
					nullptr
				};

        static char const* const exp[] = {
					"POLYGON ((100 180, 160 20, 20 20, 100 180))",
						nullptr
        };

        doTest(inp, exp);
    }

    template<>
    template<>
    void object::test<8>() {
        static char const* const inp[] = {
					"POLYGON EMPTY",
					nullptr
				};

        static char const* const exp[] = {
					nullptr
				};

        doTest(inp, exp);
    }

    template<>
    template<>
    void object::test<9>() {
        static char const* const inp[] = {
					"POLYGON EMPTY",
					nullptr
				};

        static char const* const exp[] = {
					nullptr
				};

        doTest(inp, exp);
    }

		template<>
		template<>
		void object::test<10>() {
			static char const* const inp[] = {
				"LINESTRING (0 0 , 10 10)",   // isolated edge
				"LINESTRING (185 221, 100 100)",  //dangling edge
				"LINESTRING (185 221, 88 275, 180 316)",
				"LINESTRING (185 221, 292 281, 180 316)",
				"LINESTRING (189 98, 83 187, 185 221)",
				"LINESTRING (189 98, 325 168, 185 221)",
				nullptr
			};

			static char const* const exp[] = {
				"POLYGON ((185 221, 88 275, 180 316, 292 281, 185 221))",
				"POLYGON ((189 98, 83 187, 185 221, 325 168, 189 98))",
				nullptr
			};

			doTest(inp, exp);
		}

} // namespace tut

