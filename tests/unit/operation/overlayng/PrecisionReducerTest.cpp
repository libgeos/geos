//
// Test Suite for geos::operation::overlayng::PrecisionReducer class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/PrecisionReducer.h>

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
struct test_precisionreducer_data {

    WKTReader r;
    WKTWriter w;

    void
    checkReduce(const std::string& wkt, double gridSize, const std::string& wkt_expected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        std::unique_ptr<Geometry> expected = r.read(wkt_expected);
        PrecisionModel pm(1.0/gridSize);
        std::unique_ptr<Geometry> result = PrecisionReducer::reducePrecision(geom.get(), &pm);
        ensure_equals_geometry(result.get(), expected.get());
    }
};

typedef test_group<test_precisionreducer_data> group;
typedef group::object object;

group test_precisionreducer_group("geos::operation::overlayng::PrecisionReducer");

//
// Test Cases
//

 // testPolygonGore
template<>
template<>
void object::test<1> ()
{
    checkReduce("POLYGON ((2 1, 9 1, 9 5, 3 5, 9 5.3, 9 9, 2 9, 2 1))",
        1, "POLYGON ((9 1, 2 1, 2 9, 9 9, 9 5, 9 1))");
}

// testPolygonGore2
template<>
template<>
void object::test<2> ()
{
    checkReduce("POLYGON ((9 1, 1 1, 1 9, 9 9, 9 5, 5 5.1, 5 4.9, 9 4.9, 9 1))",
        1, "POLYGON ((9 1, 1 1, 1 9, 9 9, 9 5, 9 1))");
}

// testPolygonGoreToHole
template<>
template<>
void object::test<3> ()
{
    checkReduce("POLYGON ((9 1, 1 1, 1 9, 9 9, 9 5, 5 5.9, 5 4.9, 9 4.9, 9 1))",
        1, "POLYGON ((9 1, 1 1, 1 9, 9 9, 9 5, 9 1), (9 5, 5 6, 5 5, 9 5))");
}

// testPolygonSpike
template<>
template<>
void object::test<4> ()
{
    checkReduce("POLYGON ((1 1, 9 1, 5 1.4, 5 5, 1 5, 1 1))",
        1, "POLYGON ((5 5, 5 1, 1 1, 1 5, 5 5))");
}

// testPolygonNarrowHole
template<>
template<>
void object::test<5> ()
{
    checkReduce("POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9), (2 5, 8 5, 8 5.3, 2 5))",
        1, "POLYGON ((9 1, 1 1, 1 9, 9 9, 9 1))");
}

// testPolygonWideHole
template<>
template<>
void object::test<6> ()
{
    checkReduce("POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9), (2 5, 8 5, 8 5.8, 2 5))",
        1, "POLYGON ((9 1, 1 1, 1 9, 9 9, 9 1), (8 5, 8 6, 2 5, 8 5))");
}

// testMultiPolygonGap
template<>
template<>
void object::test<7> ()
{
    checkReduce("MULTIPOLYGON (((1 9, 9.1 9.1, 9 9, 9 4, 1 4.3, 1 9)), ((1 1, 1 4, 9 3.6, 9 1, 1 1)))",
        1, "POLYGON ((9 1, 1 1, 1 4, 1 9, 9 9, 9 4, 9 1))");
}

// testMultiPolygonGapToHole
template<>
template<>
void object::test<8> ()
{
    checkReduce("MULTIPOLYGON (((1 9, 9 9, 9.05 4.35, 6 4.35, 4 6, 2.6 4.25, 1 4, 1 9)), ((1 1, 1 4, 9 4, 9 1, 1 1)))",
        1, "POLYGON ((9 1, 1 1, 1 4, 1 9, 9 9, 9 4, 9 1), (6 4, 4 6, 3 4, 6 4))");
}

// testPolygonBoxEmpty
template<>
template<>
void object::test<9> ()
{
    checkReduce("POLYGON ((1 1.4, 7.3 1.4, 7.3 1.2, 1 1.2, 1 1.4))",
        1, "POLYGON EMPTY");
}

// testPolygonThinEmpty
template<>
template<>
void object::test<10> ()
{
    checkReduce("POLYGON ((1 1.4, 3.05 1.4, 3 4.1, 6 5, 3.2 4, 3.2 1.4, 7.3 1.4, 7.3 1.2, 1 1.2, 1 1.4))",
        1, "POLYGON EMPTY");
}

//
template<>
template<>
void object::test<11> ()
{
    checkReduce("LINESTRING(-3 6, 9 1)",
        2, "LINESTRING (-2 6, 10 2)");
}

// testCollapsedLine
template<>
template<>
void object::test<12> ()
{
    checkReduce("LINESTRING(1 1, 1 9, 1.1 1)",
        1, "LINESTRING (1 1, 1 9)");
}

// testCollapsedNodedLine
template<>
template<>
void object::test<13> ()
{
    checkReduce("LINESTRING(1 1, 3 3, 9 9, 5.1 5, 2.1 2)",
        1, "MULTILINESTRING ((1 1, 2 2), (2 2, 3 3), (3 3, 5 5), (5 5, 9 9))");
}

//
template<>
template<>
void object::test<14> ()
{
    checkReduce("POLYGON ((2 1, 3 1, 3 2, 2 1))",
    // checkReduce("POLYGON ((0 0, 4 0, 4 4, 0 4, 0 0), (1 1, 1 2, 2 1, 1 1), (1 2, 1 3, 2 3, 1 2), (2 3, 3 3, 3 2, 2 3))",
        1.0/10, "POLYGON ((2 1, 3 1, 3 2, 2 1))");
}



} // namespace tut
