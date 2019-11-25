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

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_polygonizetest_data {
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
    void
    delAll(T& cnt)
    {
        for(typename T::iterator i = cnt.begin(), e = cnt.end(); i != e; ++i) {
            delete *i;
        }
    }

    template <class T>
    void
    printAll(std::ostream& os, T& cnt)
    {
        for(typename T::iterator i = cnt.begin(), e = cnt.end(); i != e; ++i) {
            os << **i << std::endl;
        }
    }

    template <class T>
    bool
    contains(T& cnt, const Geom* g)
    {
        for(typename T::iterator i = cnt.begin(), e = cnt.end(); i != e; ++i) {
            const auto& element = *i;
            if(element->equalsExact(g)) {
                return true;
            }
        }
        return false;
    }

    template <class T, class S>
    bool
    compare(T& ex, S& ob)
    {
        using std::cout;
        using std::endl;

        if(ex.size() != ob.size()) {
            cout << "Expected " << ex.size() << " polygons, obtained "
                 << ob.size() << endl;
            return false;
        }
        for(typename T::iterator i = ex.begin(), e = ex.end(); i != e; ++i) {
            if(! contains(ob, i->get())) {
                cout << "Expected " << wktwriter.write(i->get())
                     << " not found" << endl;
                return false;
            }
        }

        return true;

    }

    bool
    doTest(const std::vector<std::string> & inputWKT, const std::vector<std::string> & expectWKT, bool onlyPolygonal)
    {
        using std::cout;
        using std::endl;

        std::vector<std::unique_ptr<geos::geom::Geometry>> inputGeoms, expectGeoms;

        for (const auto& wkt : inputWKT) {
            inputGeoms.push_back(wktreader.read(wkt));
        }

        for (const auto& wkt : expectWKT) {
            auto g = wktreader.read(wkt);
            g->normalize();
            expectGeoms.push_back(std::move(g));
        }

        Polygonizer polygonizer(onlyPolygonal);
        for (const auto& p : inputGeoms) {
            polygonizer.add(p.get());
        }

        auto retGeoms = polygonizer.getPolygons();
        for (const auto& g : retGeoms) {
            g->normalize();
        }

        bool ok = compare(expectGeoms, retGeoms);
        if(! ok) {
            cout << "EXPECTED(" << expectGeoms.size() << "): " << std::endl;
            printAll(cout, expectGeoms);
            cout << "OBTAINED(" << retGeoms.size() << "): " << std::endl;
            printAll(cout, retGeoms);
            cout << endl;

            ensure("not all expected geometries in the obtained set", 0);
        }

        return ok;
    }

};

typedef test_group<test_polygonizetest_data> group;
typedef group::object object;

group test_polygonizetest_group("geos::operation::polygonize::Polygonize");

// test1() in JTS
template<>
template<>
void object::test<1>
()
{
    std::vector<std::string> inp {
        "LINESTRING EMPTY",
        "LINESTRING EMPTY",
    };

    std::vector<std::string> exp{};

    doTest(inp, exp, false);
}

// test2() in JTS
template<>
template<>
void object::test<2>
()
{
    std::vector<std::string> inp{
        "LINESTRING (100 180, 20 20, 160 20, 100 180)",
        "LINESTRING (100 180, 80 60, 120 60, 100 180)",
    };

    std::vector<std::string> exp{
        "POLYGON ((100 180, 120 60, 80 60, 100 180))",
        "POLYGON ((100 180, 160 20, 20 20, 100 180), (100 180, 80 60, 120 60, 100 180))"
    };

    doTest(inp, exp, false);
}

// JTS test3
template<>
template<>
void object::test<3>()
{
    std::vector<std::string> inp{
        "LINESTRING (0 0, 4 0)",
        "LINESTRING (4 0, 5 3)",
        "LINESTRING (5 3, 4 6, 6 6, 5 3)",
        "LINESTRING (5 3, 6 0)",
        "LINESTRING (6 0, 10 0, 5 10, 0 0)",
        "LINESTRING (4 0, 6 0)"
    };

    std::vector<std::string> exp{
        "POLYGON ((5 3, 4 0, 0 0, 5 10, 10 0, 6 0, 5 3), (5 3, 6 6, 4 6, 5 3))",
        "POLYGON ((5 3, 4 6, 6 6, 5 3))",
        "POLYGON ((4 0, 5 3, 6 0, 4 0))"
    };

    doTest(inp, exp, false);
}

// JTS testPolygonal1
template<>
template<>
void object::test<4>()
{
    std::vector<std::string> inp{
            "LINESTRING (100 100, 100 300, 300 300, 300 100, 100 100)",
            "LINESTRING (150 150, 150 250, 250 250, 250 150, 150 150)"
    };

    std::vector<std::string> exp{
            "POLYGON ((100 100, 100 300, 300 300, 300 100, 100 100), (150 150, 150 250, 250 250, 250 150, 150 150))"
    };

    doTest(inp, exp, true);
}

// JTS testPolygonal2
template<>
template<>
void object::test<5>()
{
    std::vector<std::string> inp{
            "LINESTRING (100 100, 100 0, 0 0, 0 100, 100 100)",
            "LINESTRING (10 10, 10 30, 20 30)",
            "LINESTRING (20 30, 30 30, 30 20)",
            "LINESTRING (30 20, 30 10, 10 10)",
            "LINESTRING (40 40, 40 20, 30 20)",
            "LINESTRING (30 20, 20 20, 20 30)",
            "LINESTRING (20 30, 20 40, 40 40))"
    };

    std::vector<std::string> exp{
            "POLYGON ((0 0, 0 100, 100 100, 100 0, 0 0), (10 10, 30 10, 30 20, 40 20, 40 40, 20 40, 20 30, 10 30, 10 10))",
            "POLYGON ((20 20, 20 30, 30 30, 30 20, 20 20))"
    };

    doTest(inp, exp, true);
}

// JTS testPolygonal_OuterOnly_1
template<>
template<>
void object::test<6>()
{
    // Two adjacent squares, but since we only get polygonal output
    // we only get one of the squares back.
    std::vector<std::string> inp{
            "LINESTRING (10 10, 10 20, 20 20)",
            "LINESTRING (20 20, 20 10)",
            "LINESTRING (20 10, 10 10)",
            "LINESTRING (20 20, 30 20, 30 10, 20 10)"
    };

    std::vector<std::string> exp{
            "POLYGON ((20 20, 20 10, 10 10, 10 20, 20 20))"
    };

    doTest(inp, exp, true);
}

// JTS testPolygonal_OuterOnly_2
template<>
template<>
void object::test<7>()
{
    std::vector<std::string> inp{
            "LINESTRING (100 400, 200 400, 200 300)",
            "LINESTRING (200 300, 150 300)",
            "LINESTRING (150 300, 100 300, 100 400)",
            "LINESTRING (200 300, 250 300, 250 200)",
            "LINESTRING (250 200, 200 200)",
            "LINESTRING (200 200, 150 200, 150 300)",
            "LINESTRING (250 200, 300 200, 300 100, 200 100, 200 200)"
    };

    std::vector<std::string> exp{
         "POLYGON ((150 300, 100 300, 100 400, 200 400, 200 300, 150 300))",
         "POLYGON ((200 200, 250 200, 300 200, 300 100, 200 100, 200 200))"
    };

    doTest(inp, exp, true);
}

// JTS testPolygonal_OuterOnly_Checkerboard
template<>
template<>
void object::test<8>()
{
    std::vector<std::string> inp{
            "LINESTRING (10 20, 20 20)",
            "LINESTRING (10 20, 10 30)",
            "LINESTRING (20 10, 10 10, 10 20)",
            "LINESTRING (10 30, 20 30)",
            "LINESTRING (10 30, 10 40, 20 40)",
            "LINESTRING (30 10, 20 10)",
            "LINESTRING (20 20, 20 10)",
            "LINESTRING (20 20, 30 20)",
            "LINESTRING (20 30, 20 20)",
            "LINESTRING (20 30, 30 30)",
            "LINESTRING (20 40, 20 30)",
            "LINESTRING (20 40, 30 40)",
            "LINESTRING (40 20, 40 10, 30 10)",
            "LINESTRING (30 20, 30 10)",
            "LINESTRING (30 20, 40 20)",
            "LINESTRING (30 30, 30 20)",
            "LINESTRING (30 30, 40 30)",
            "LINESTRING (30 40, 30 30)",
            "LINESTRING (30 40, 40 40, 40 30)",
            "LINESTRING (40 30, 40 20)"
    };

    std::vector<std::string> exp{
            "POLYGON ((10 20, 20 20, 20 10, 10 10, 10 20))",
            "POLYGON ((20 30, 10 30, 10 40, 20 40, 20 30))",
            "POLYGON ((30 20, 20 20, 20 30, 30 30, 30 20))",
            "POLYGON ((30 10, 30 20, 40 20, 40 10, 30 10))",
            "POLYGON ((30 40, 40 40, 40 30, 30 30, 30 40))"
    };

    doTest(inp, exp, true);
}

} // namespace tut

