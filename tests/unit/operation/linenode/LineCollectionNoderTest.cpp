// Test Suite for geos::operation::linenode::LineCollectionNoder

#include <tut/tut.hpp>
#include <geos/operation/linenode/LineCollectionNoder.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>

#include "utility.h"

using geos::geom::Geometry;
using geos::io::WKTReader;
using geos::operation::linenode::LineCollectionNoder;

namespace tut {

struct test_linecollectionnoder_data {
    WKTReader r;

    std::vector<std::unique_ptr<Geometry>> readArray(
        std::initializer_list<const char*> wkts)
    {
        std::vector<std::unique_ptr<Geometry>> v;
        for (const char* wkt : wkts)
            v.push_back(r.read(wkt));
        return v;
    }

    void checkResult(
        const std::vector<std::unique_ptr<Geometry>>& input,
        std::initializer_list<const char*> expectedWkts,
        double gridSize = 0.0)
    {
        auto actual = LineCollectionNoder::node(input, gridSize);
        std::vector<const char*> exp(expectedWkts);
        ensure_equals("result count", actual.size(), exp.size());
        for (std::size_t i = 0; i < exp.size(); i++) {
            auto e = r.read(exp[i]);
            ensure_equals_exact_geometry_xyzm(actual[i].get(), e.get(), 1e-9);
        }
    }
};

typedef test_group<test_linecollectionnoder_data> group;
typedef group::object object;
group test_linecollectionnoder_group("operation::linenode::LineCollectionNoder");

// Two crossing linestrings: each split at the crossing point
template<> template<> void object::test<1>()
{
    set_test_name("two crossing linestrings");
    checkResult(
        readArray({"LINESTRING (0 0, 10 10)", "LINESTRING (0 10, 10 0)"}),
        {"MULTILINESTRING ((0 0, 5 5), (5 5, 10 10))",
         "MULTILINESTRING ((0 10, 5 5), (5 5, 10 0))"});
}

// Non-intersecting: each becomes a single-segment MultiLineString
template<> template<> void object::test<2>()
{
    set_test_name("non-intersecting: each kept as single segment");
    checkResult(
        readArray({"LINESTRING (0 0, 5 5)", "LINESTRING (10 0, 15 5)"}),
        {"MULTILINESTRING ((0 0, 5 5))", "MULTILINESTRING ((10 0, 15 5))"});
}

// Three-way: three lines that all cross at (5,5)
template<> template<> void object::test<3>()
{
    set_test_name("three linestrings intersecting pairwise");
    checkResult(
        readArray({
            "LINESTRING (0 5, 10 5)",
            "LINESTRING (5 0, 5 10)",
            "LINESTRING (0 0, 10 10)"}),
        {"MULTILINESTRING ((0 5, 5 5), (5 5, 10 5))",
         "MULTILINESTRING ((5 0, 5 5), (5 5, 5 10))",
         "MULTILINESTRING ((0 0, 5 5), (5 5, 10 10))"});
}

// Issue 877: near-duplicate rings — snap-rounding converges where IteratedNoder would not
template<> template<> void object::test<4>()
{
    set_test_name("issue 877 near-duplicate rings with snap-rounding");
    auto input = readArray({
        "LINESTRING (125635 6696, 131951 6376, 132163 474.0000000000043, 128381 1569.9999999999986, 125635 6696)",
        "LINESTRING (125635 6696, 131951 6376, 132163 474, 128381 1570, 125635 6696)"
    });
    auto actual = LineCollectionNoder::node(input, 1.0);
    ensure_equals("count preserved", actual.size(), std::size_t(2));
    ensure("member 0 non-empty", !actual[0]->isEmpty());
    ensure("member 1 non-empty", !actual[1]->isEmpty());
}

// MULTILINESTRING member: all component LineStrings extracted and noded as a unit
template<> template<> void object::test<5>()
{
    set_test_name("MULTILINESTRING member is noded as a unit");
    checkResult(
        readArray({
            "MULTILINESTRING ((0 0, 10 10), (0 10, 10 0))",
            "LINESTRING (5 -1, 5 11)"}),
        {"MULTILINESTRING ((0 0, 5 5), (5 5, 10 10), (0 10, 5 5), (5 5, 10 0))",
         "MULTILINESTRING ((5 -1, 5 5), (5 5, 5 11))"});
}

// Identical members: both preserved independently, linework not dissolved
template<> template<> void object::test<6>()
{
    set_test_name("identical members each keep their own linework");
    checkResult(
        readArray({
            "LINESTRING (0 0, 10 0)",
            "LINESTRING (0 0, 10 0)"}),
        {"MULTILINESTRING ((0 0, 10 0))",
         "MULTILINESTRING ((0 0, 10 0))"});
}

// Duplicate segments within same member: deduplication removes the second copy
template<> template<> void object::test<7>()
{
    set_test_name("duplicate segments within one member are deduplicated");
    auto input = readArray({
        "MULTILINESTRING ((0 0, 10 0), (0 0, 10 0))",
        "LINESTRING (5 -5, 5 5)"
    });
    auto actual = LineCollectionNoder::node(input);
    ensure_equals("count preserved", actual.size(), std::size_t(2));
    ensure_equals("member 0 deduplicated to 2 segments",
        (int)actual[0]->getNumGeometries(), 2);
}

// Empty member: produces an empty MultiLineString in the corresponding output slot
template<> template<> void object::test<8>()
{
    set_test_name("empty member produces empty output slot");
    auto input = readArray({
        "LINESTRING EMPTY",
        "LINESTRING (0 0, 10 0)"
    });
    auto actual = LineCollectionNoder::node(input);
    ensure_equals("count preserved", actual.size(), std::size_t(2));
    ensure("member 0 output is empty", actual[0]->isEmpty());
    ensure("member 1 output is non-empty", !actual[1]->isEmpty());
}

// Empty collection: returns an empty result vector
template<> template<> void object::test<9>()
{
    set_test_name("empty collection returns empty result");
    std::vector<const Geometry*> empty;
    auto actual = LineCollectionNoder::node(empty);
    ensure_equals("empty result", actual.size(), std::size_t(0));
}

// Non-linear member (POLYGON): no LineString components, output slot is empty
template<> template<> void object::test<10>()
{
    set_test_name("polygon member yields empty output slot");
    auto input = readArray({
        "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
        "LINESTRING (5 -5, 5 15)"
    });
    auto actual = LineCollectionNoder::node(input);
    ensure_equals("count preserved", actual.size(), std::size_t(2));
    ensure("polygon member output is empty", actual[0]->isEmpty());
}

// Nested GEOMETRYCOLLECTION member: all linear components extracted
template<> template<> void object::test<11>()
{
    set_test_name("nested GEOMETRYCOLLECTION member");
    checkResult(
        readArray({
            "GEOMETRYCOLLECTION (LINESTRING (0 0, 10 0), LINESTRING (0 5, 10 5))",
            "LINESTRING (5 -1, 5 10)"}),
        {"MULTILINESTRING ((0 0, 5 0), (5 0, 10 0), (0 5, 5 5), (5 5, 10 5))",
         "MULTILINESTRING ((5 -1, 5 0), (5 0, 5 5), (5 5, 5 10))"});
}

// gridSize < 0: treated as 0 (uses IteratedNoder), does not crash
template<> template<> void object::test<12>()
{
    set_test_name("gridSize < 0 falls back to standard noding");
    auto input = readArray({"LINESTRING (0 0, 10 10)", "LINESTRING (0 10, 10 0)"});
    auto actual = LineCollectionNoder::node(input, -1.0);
    ensure_equals("count preserved", actual.size(), std::size_t(2));
    ensure_equals("member 0 split at crossing", (int)actual[0]->getNumGeometries(), 2);
}

// Z propagation: new node point gets Z averaged from both lines' interpolated values
template<> template<> void object::test<13>()
{
    set_test_name("Z values averaged at new node points from both intersecting lines");
    // Line 0: Z 0->1 at t=0.5 -> zp=0.5; Line 1: Z 5->10 at t=0.5 -> zq=7.5; avg=4.0
    checkResult(
        readArray({
            "LINESTRING Z (0 0 0, 1 1 1)",
            "LINESTRING Z (0 1 5, 1 0 10)"}),
        {"MULTILINESTRING Z ((0 0 0, 0.5 0.5 4), (0.5 0.5 4, 1 1 1))",
         "MULTILINESTRING Z ((0 1 5, 0.5 0.5 4), (0.5 0.5 4, 1 0 10))"});
}

// M propagation: new node point gets M averaged from both lines' interpolated values
template<> template<> void object::test<14>()
{
    set_test_name("M values averaged at new node points from both intersecting lines");
    // Line 0: M 0->1 at t=0.5 -> mp=0.5; Line 1: M 5->10 at t=0.5 -> mq=7.5; avg=4.0
    checkResult(
        readArray({
            "LINESTRING M (0 0 0, 1 1 1)",
            "LINESTRING M (0 1 5, 1 0 10)"}),
        {"MULTILINESTRING M ((0 0 0, 0.5 0.5 4), (0.5 0.5 4, 1 1 1))",
         "MULTILINESTRING M ((0 1 5, 0.5 0.5 4), (0.5 0.5 4, 1 0 10))"});
}

// ZM propagation: both Z and M averaged at new node points
template<> template<> void object::test<15>()
{
    set_test_name("ZM values averaged at new node points from both intersecting lines");
    // Line 0 at t=0.5: Z=0.5 M=150; Line 1 at t=0.5: Z=75 M=7.5; avg Z=37.75 M=78.75
    checkResult(
        readArray({
            "LINESTRING ZM (0 0 0 100, 1 1 1 200)",
            "LINESTRING ZM (0 1 50 5, 1 0 100 10)"}),
        {"MULTILINESTRING ZM ((0 0 0 100, 0.5 0.5 37.75 78.75), (0.5 0.5 37.75 78.75, 1 1 1 200))",
         "MULTILINESTRING ZM ((0 1 50 5, 0.5 0.5 37.75 78.75), (0.5 0.5 37.75 78.75, 1 0 100 10))"});
}

} // namespace tut
