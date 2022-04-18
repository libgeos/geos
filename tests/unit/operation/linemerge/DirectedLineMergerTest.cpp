//
// Test Suite for geos::operation::linemerge::DirectedLineMerger class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/operation/linemerge/DirectedLineMerger.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/util/IllegalArgumentException.h>
// std
#include <string>

namespace tut {
//
// Test Group
//

struct test_directedlinemerger_data {
    typedef geos::operation::linemerge::DirectedLineMerger DirectedLineMerger;
    typedef std::vector<geos::geom::Geometry*> GeomVect;
    typedef std::vector<geos::geom::LineString*> LineVect;

    geos::io::WKTReader wktreader;
    geos::io::WKTWriter wktwriter;

    typedef geos::geom::Geometry Geom;
    typedef geos::geom::Geometry::Ptr GeomPtr;

    std::vector<const geos::geom::Geometry*> inpGeoms;
    std::vector<const geos::geom::Geometry*> expGeoms;

    test_directedlinemerger_data()
        : wktreader(), wktwriter()
    {
        wktwriter.setTrim(true);
    }

    ~test_directedlinemerger_data()
    {
        delAll(inpGeoms);
        delAll(expGeoms);
    }

    GeomPtr
    readWKT(const std::string& inputWKT)
    {
        return GeomPtr(wktreader.read(inputWKT));
    }

    void
    readWKT(const char* const* inputWKT, std::vector<const Geom*>& geoms)
    {
        for(const char* const* ptr = inputWKT; *ptr; ++ptr) {
            geoms.push_back(readWKT(*ptr).release());
        }
    }

    void
    doTest(const char* const* inputWKT,
           const char* const* expectedWKT,
           bool compareDirections = true)
    {
        DirectedLineMerger directedLineMerger;

        readWKT(inputWKT, inpGeoms);
        readWKT(expectedWKT, expGeoms);

        directedLineMerger.add(&inpGeoms);
        auto mrgGeoms = directedLineMerger.getMergedLineStrings();
        compare(expGeoms, mrgGeoms, compareDirections);
    }

    template <class TargetContainer>
    void
    delAll(TargetContainer& geoms)
    {
        for(auto& g : geoms) {
            delete g;
        }
    }

    template <class TargetContainer1, class TargetContainer2>
    static void
    compare(TargetContainer1& expectedGeometries,
            TargetContainer2& actualGeometries,
            bool compareDirections)
    {
        ensure_equals(actualGeometries.size(), expectedGeometries.size());
        for(auto & g : expectedGeometries) {
            ensure(contains(actualGeometries, g, compareDirections));
        }
    }

    template <class TargetContainer>
    static bool
    contains(TargetContainer& actualGeometries,
             const Geom* g, bool exact)
    {
        for(const auto& e : actualGeometries) {
            Geom* element = dynamic_cast<Geom*>(e.get());
            if(exact && element->equalsExact(g)) {
                return true;
            }
            if(!exact && element->equals(g)) {
                return true;
            }
        }

        return false;
    }

private:
    // noncopyable
    test_directedlinemerger_data(test_directedlinemerger_data const& other) = delete;
    test_directedlinemerger_data& operator=(test_directedlinemerger_data const& rhs) = delete;
};

typedef test_group<test_directedlinemerger_data> group;
typedef group::object object;

group test_directedlinemerger_group("geos::operation::linemerge::DirectedLineMerger");

//
// Test Cases
//

// Requested for PostGIS https://trac.osgeo.org/postgis/ticket/4939
template<> template<>
void object::test<1>()
{
    const char* inpWKT[] = {
        "MULTILINESTRING ((-29 -27, 1 2), (-29 -27, -45 -33, -46 -32))",
        nullptr
    };
    const char* expWKT[] = {
        "LINESTRING (-29 -27, 1 2)",
        "LINESTRING (-29 -27, -45 -33, -46 -32)",
        nullptr
    };

    doTest(inpWKT, expWKT);
}

// Basic functionality
template<> template<>
void object::test<2>()
{
    const char* inpWKT[] = {
        "LINESTRING (0 0, 1 1)",
        "LINESTRING (1 1, 2 2)",
        "LINESTRING (2 2, 3 3)",
        nullptr
    };
    const char* expWKT[] = {
        "LINESTRING (0 0, 1 1, 2 2, 3 3)",
        nullptr
    };

    doTest(inpWKT, expWKT);
}

// Multiple outgoing edges
template<> template<>
void object::test<3>()
{
    const char* inpWKT[] = {
        "LINESTRING (0 0, 1 1)",
        "LINESTRING (1 1, 2 2)",
        "LINESTRING (2 2, 3 3)",
        "LINESTRING (2 2, 3 2)",
        nullptr
    };
    const char* expWKT[] = {
        "LINESTRING (0 0, 1 1, 2 2)",
        "LINESTRING (2 2, 3 3)",
        "LINESTRING (2 2, 3 2)",
        nullptr
    };

    doTest(inpWKT, expWKT);
}

// Multiple incoming edges
template<> template<>
void object::test<4>()
{
    const char* inpWKT[] = {
        "LINESTRING (0 0, 1 1)",
        "LINESTRING (0 1, 1 1)",
        "LINESTRING (1 1, 2 2)",
        "LINESTRING (2 2, 3 3)",
        nullptr
    };
    const char* expWKT[] = {
        "LINESTRING (0 0, 1 1)",
        "LINESTRING (0 1, 1 1)",
        "LINESTRING (1 1, 2 2, 3 3)",
        nullptr
    };

    doTest(inpWKT, expWKT);
}

// Check:
// - line strings no not cross points with multiple outgoing or
//   incoming edges: (1, 1), (3, 2);
// - both paths from (1, 1) to (3, 2) are merged into line strings
// - all edges are present in result
template<> template<>
void object::test<5>()
{
    //             4, 3
    //              /
    //   2, 2 *----* 3, 2
    //       /    /
    // 1, 1 *----* 2, 1
    //     /
    //   0, 0
    const char* inpWKT[] = {
        "LINESTRING (0 0, 1 1)",
        "LINESTRING (1 1, 2 2)",
        "LINESTRING (1 1, 2 1)",
        "LINESTRING (2 2, 3 2)",
        "LINESTRING (2 1, 3 2)",
        "LINESTRING (3 2, 4 3)",
        nullptr
    };
    const char* expWKT[] = {
        "LINESTRING (0 0, 1 1)",
        "LINESTRING (1 1, 2 2, 3 2)",
        "LINESTRING (1 1, 2 1, 3 2)",
        "LINESTRING (3 2, 4 3)",
        nullptr
    };

    doTest(inpWKT, expWKT);
}

// Lines with opposide directions are not merged
template<> template<>
void object::test<6>()
{
    const char* inpWKT[] = {
        "LINESTRING (0 0, 1 2, 2 1)",
        "LINESTRING (4 2, 3 2, 2 1)",
        nullptr
    };
    const char* expWKT[] = {
        "LINESTRING (0 0, 1 2, 2 1)",
        "LINESTRING (4 2, 3 2, 2 1)",
        nullptr
    };

    doTest(inpWKT, expWKT);
}

// Two intersecting/crossing segments must not merge
template<> template<>
void object::test<7>()
{
    const char* inpWKT[] = {
        "LINESTRING (0 0, 100 100)", "LINESTRING (0 100, 100 0)",
        nullptr
    };
    const char* expWKT[] = {
        "LINESTRING (0 0, 100 100)", "LINESTRING (0 100, 100 0)",
        nullptr
    };

    doTest(inpWKT, expWKT);
}

template<> template<>
void object::test<8>()
{
    const char* inpWKT[] = {
        "LINESTRING EMPTY",
        "LINESTRING EMPTY",
        nullptr
    };
    const char* expWKT[] = {
        nullptr
    };

    doTest(inpWKT, expWKT);
}

// Don't blame me for this, I'm just copying LineMerger tests :P
template<> template<>
void object::test<9>()
{
    const char* inpWKT[] = {
        nullptr
    };
    const char* expWKT[] = {
        nullptr
    };

    doTest(inpWKT, expWKT);
}

// testSingleUniquePoint()
template<> template<>
void object::test<10>()
{
    const char* inpWKT[] = {
        "LINESTRING (10642 31441, 10642 31441)",
        nullptr
    };
    const char* expWKT[] = {
        nullptr
    };

    doTest(inpWKT, expWKT);
}

// Loop
template<> template<>
void object::test<11>
()
{
    const char* inpWKT[] = {
        "LINESTRING(0 0, 0 5)",
        "LINESTRING(0 5, 5 5)",
        "LINESTRING(5 5, 5 0)",
        "LINESTRING(5 0, 0 0)",
        nullptr
    };
    const char* expWKT[] = {
        "LINESTRING(0 0, 0 5, 5 5, 5 0, 0 0)",
        nullptr
    };

    doTest(inpWKT, expWKT);
}

} // namespace tut

