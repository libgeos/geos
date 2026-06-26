#include <tut/tut.hpp>

#include <geos/geom/Geometry.h>
#include <geos/noding/GeometryNoder.h>

#include "utility.h"

#include <vector>

using geos::geom::Geometry;
using geos::noding::GeometryNoder;

namespace tut {

struct test_geometrynoder_data {
    geos::io::WKTReader reader_;
};

typedef test_group<test_geometrynoder_data> group;
typedef group::object object;

group test_geometrynoder_group("geos::noding::GeometryNoder");

template<>
template<>
void object::test<1>()
{
    set_test_name("single input") ;

    auto input = reader_.read("MULTILINESTRING ((0 0, 10 10, 10 0, 0 10))");

    auto result = GeometryNoder::node(*input);
    ensure(result != nullptr);

    auto expected = reader_.read("MULTILINESTRING ((0 0, 5 5), (5 5, 10 10, 10 0, 5 5), (5 5, 0 10))");

    ensure_equals_exact_geometry_xyzm( result.get(), expected.get(), 0);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("two inputs, output all edges") ;

    auto input1 = reader_.read("LINESTRING (0 0, 10 10)");
    auto input2 = reader_.read("LINESTRING (0 10, 10 0)");

    auto result = GeometryNoder::node(*input1, *input2);
    ensure(result != nullptr);

    auto expected = reader_.read("MULTILINESTRING ((0 0, 5 5), (5 5, 10 10), (0 10, 5 5), (5 5, 10 0))");

    ensure_equals_exact_geometry_xyzm(result.get(), expected.get(), 0);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("two inputs, only output edges from first") ;

    auto input1 = reader_.read("LINESTRING (0 0, 10 10)");
    auto input2 = reader_.read("LINESTRING (0 10, 10 0)");

    GeometryNoder noder(*input1, *input2);
    noder.setOnlyFirstGeomEdges(true);

    auto result = noder.getNoded();
    ensure(result != nullptr);

    auto expected = reader_.read("MULTILINESTRING ((0 0, 5 5), (5 5, 10 10))");

    ensure_equals_exact_geometry_xyzm(result.get(), expected.get(), 0);
}

// Compare a normalized result slot against expected WKT.
namespace {
void check_slot(geos::io::WKTReader& reader, const Geometry* got, const std::string& wkt)
{
    auto expected = reader.read(wkt);
    auto g = got->clone();
    g->normalize();
    expected->normalize();
    ensure_equals_exact_geometry_xyzm(g.get(), expected.get(), 0);
}
}

template<>
template<>
void object::test<4>()
{
    set_test_name("collection: crossing lines keep per-member identity");

    auto a = reader_.read("LINESTRING (0 0, 10 10)");
    auto b = reader_.read("LINESTRING (0 10, 10 0)");

    std::vector<const Geometry*> input{a.get(), b.get()};
    auto result = GeometryNoder::nodeCollection(input);

    ensure_equals("1:1 mapping", result.size(), 2u);
    check_slot(reader_, result[0].get(), "MULTILINESTRING ((0 0, 5 5), (5 5, 10 10))");
    check_slot(reader_, result[1].get(), "MULTILINESTRING ((0 10, 5 5), (5 5, 10 0))");
}

template<>
template<>
void object::test<5>()
{
    set_test_name("collection: shared linework retained in both members");

    // The two members overlap along (5 0)-(10 0); that shared edge must
    // appear in BOTH outputs (not dissolved as GEOSNode would).
    auto a = reader_.read("LINESTRING (0 0, 10 0)");
    auto b = reader_.read("LINESTRING (5 0, 10 0, 10 10)");

    std::vector<const Geometry*> input{a.get(), b.get()};
    auto result = GeometryNoder::nodeCollection(input);

    ensure_equals("1:1 mapping", result.size(), 2u);
    check_slot(reader_, result[0].get(), "MULTILINESTRING ((0 0, 5 0), (5 0, 10 0))");
    check_slot(reader_, result[1].get(), "MULTILINESTRING ((5 0, 10 0), (10 0, 10 10))");
}

template<>
template<>
void object::test<6>()
{
    set_test_name("collection: non-linear member yields empty slot");

    auto a = reader_.read("LINESTRING (0 0, 10 10)");
    auto pt = reader_.read("POINT (5 5)");
    auto b = reader_.read("LINESTRING (0 10, 10 0)");

    std::vector<const Geometry*> input{a.get(), pt.get(), b.get()};
    auto result = GeometryNoder::nodeCollection(input);

    ensure_equals("1:1 mapping", result.size(), 3u);
    check_slot(reader_, result[0].get(), "MULTILINESTRING ((0 0, 5 5), (5 5, 10 10))");
    ensure("point slot empty", result[1]->isEmpty());
    ensure_equals("point slot type", result[1]->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);
    check_slot(reader_, result[2].get(), "MULTILINESTRING ((0 10, 5 5), (5 5, 10 0))");
}

template<>
template<>
void object::test<7>()
{
    set_test_name("collection: gridSize snap-rounding nodes near-coincident input");

    // b's endpoint (5 0.4) lies just off a; snap-rounding to a grid of 1.0
    // pulls it onto (5 0), creating a node that splits a there.
    auto a = reader_.read("LINESTRING (0 0, 10 0)");
    auto b = reader_.read("LINESTRING (5 0.4, 5 10)");

    std::vector<const Geometry*> input{a.get(), b.get()};
    auto result = GeometryNoder::nodeCollection(input, 1.0);

    ensure_equals("1:1 mapping", result.size(), 2u);
    check_slot(reader_, result[0].get(), "MULTILINESTRING ((0 0, 5 0), (5 0, 10 0))");
    check_slot(reader_, result[1].get(), "MULTILINESTRING ((5 0, 5 10))");
}

template<>
template<>
void object::test<8>()
{
    set_test_name("collection: curved member noded and returned as arcs");

    // Vertical line crosses the top of a unit semicircle arc at (0 1).
    auto line = reader_.read("LINESTRING (0 -1, 0 2)");
    auto arc = reader_.read("CIRCULARSTRING (-1 0, 0 1, 1 0)");

    std::vector<const Geometry*> input{line.get(), arc.get()};
    auto result = GeometryNoder::nodeCollection(input);

    ensure_equals("1:1 mapping", result.size(), 2u);

    // The line slot splits at (0 1).
    ensure_equals("line slot type", result[0]->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);
    ensure_equals("line split count", result[0]->getNumGeometries(), 2u);

    // The arc slot stays curved: a MultiCurve of two CircularStrings.
    ensure_equals("arc slot type", result[1]->getGeometryTypeId(), geos::geom::GEOS_MULTICURVE);
    ensure_equals("arc split count", result[1]->getNumGeometries(), 2u);
    for (std::size_t i = 0; i < result[1]->getNumGeometries(); i++) {
        ensure_equals("arc piece is a CircularString",
            result[1]->getGeometryN(i)->getGeometryTypeId(), geos::geom::GEOS_CIRCULARSTRING);
    }
}

template<>
template<>
void object::test<9>()
{
    set_test_name("collection: mixed types — polygon boundary noded, point slot empty");

    // A line crosses a polygon; a point contributes nothing. Consistent
    // with node(): areal members are reduced to noded boundary linework,
    // points yield an empty slot.
    auto line = reader_.read("LINESTRING (-5 5, 15 5)");
    auto poly = reader_.read("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");
    auto pt   = reader_.read("POINT (3 3)");

    std::vector<const Geometry*> input{line.get(), poly.get(), pt.get()};
    auto result = GeometryNoder::nodeCollection(input);

    ensure_equals("1:1 mapping", result.size(), 3u);

    // Line slot: split where it crosses the polygon edges.
    check_slot(reader_, result[0].get(),
        "MULTILINESTRING ((-5 5, 0 5), (0 5, 10 5), (10 5, 15 5))");

    // Polygon slot: boundary returned as noded linework, split at the
    // two points where the line crosses it.
    check_slot(reader_, result[1].get(),
        "MULTILINESTRING ((0 0, 10 0, 10 5), (10 5, 10 10, 0 10, 0 5), (0 5, 0 0))");

    // Point slot: empty MultiLineString.
    ensure("point slot empty", result[2]->isEmpty());
    ensure_equals("point slot type", result[2]->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);
}

template<>
template<>
void object::test<10>()
{
    set_test_name("single input: mixed-type GeometryCollection flattens to noded linework");

    // The existing node() path: points are dropped, polygon boundaries are
    // noded as lines along with the linear members, and everything is
    // dissolved into one MultiLineString.
    auto input = reader_.read(
        "GEOMETRYCOLLECTION ("
        "LINESTRING (-5 5, 15 5),"
        "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0)),"
        "POINT (3 3))");

    auto result = GeometryNoder::node(*input);
    ensure(result != nullptr);

    check_slot(reader_, result.get(),
        "MULTILINESTRING ("
        "(-5 5, 0 5), (0 5, 10 5), (10 5, 15 5),"
        "(0 0, 10 0, 10 5), (10 5, 10 10, 0 10, 0 5), (0 5, 0 0))");
}

template<>
template<>
void object::test<11>()
{
    set_test_name("collection: gridSize is ignored (not applied) when input is curved");

    // Snap-rounding cannot node arcs — if the curve detection failed and a
    // positive gridSize selected SnapRoundingNoder, the arc paths would
    // throw UnsupportedOperationException. A positive gridSize here must be
    // silently ignored in favour of exact arc noding, giving the same
    // curve-preserving result as the gridSize == 0 case (test<8>).
    auto line = reader_.read("LINESTRING (0 -1, 0 2)");
    auto arc = reader_.read("CIRCULARSTRING (-1 0, 0 1, 1 0)");

    std::vector<const Geometry*> input{line.get(), arc.get()};
    auto result = GeometryNoder::nodeCollection(input, 1.0);   // gridSize > 0

    ensure_equals("1:1 mapping", result.size(), 2u);

    // Line slot still splits at (0 1).
    ensure_equals("line slot type", result[0]->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);
    ensure_equals("line split count", result[0]->getNumGeometries(), 2u);

    // Arc slot stays curved: exact arc noding ran, gridSize was not applied.
    ensure_equals("arc slot type", result[1]->getGeometryTypeId(), geos::geom::GEOS_MULTICURVE);
    ensure_equals("arc split count", result[1]->getNumGeometries(), 2u);
    for (std::size_t i = 0; i < result[1]->getNumGeometries(); i++) {
        ensure_equals("arc piece is a CircularString",
            result[1]->getGeometryN(i)->getGeometryTypeId(), geos::geom::GEOS_CIRCULARSTRING);
    }
}

template<>
template<>
void object::test<12>()
{
    set_test_name("collection: multi-part member stays in one slot (MultiLineString)");

    // A MultiLineString member's parts must all land in its single slot,
    // proving slot identity is per top-level member, not per component.
    auto a = reader_.read("MULTILINESTRING ((0 0, 10 0), (0 5, 10 5))");
    auto b = reader_.read("LINESTRING (5 -1, 5 6)");

    std::vector<const Geometry*> input{a.get(), b.get()};
    auto result = GeometryNoder::nodeCollection(input);

    ensure_equals("1:1 mapping", result.size(), 2u);

    // Both parts of member 0 split where member 1 crosses — all four pieces
    // in the single slot 0.
    check_slot(reader_, result[0].get(),
        "MULTILINESTRING ((0 0, 5 0), (5 0, 10 0), (0 5, 5 5), (5 5, 10 5))");
    check_slot(reader_, result[1].get(),
        "MULTILINESTRING ((5 -1, 5 0), (5 0, 5 5), (5 5, 5 6))");
}

template<>
template<>
void object::test<13>()
{
    set_test_name("collection: multi-part member stays in one slot (MultiPolygon)");

    // Both polygons' boundaries are noded into the single MultiPolygon slot.
    auto a = reader_.read(
        "MULTIPOLYGON ("
        "((0 0, 4 0, 4 4, 0 4, 0 0)),"
        "((6 0, 10 0, 10 4, 6 4, 6 0)))");
    auto b = reader_.read("LINESTRING (-1 2, 11 2)");

    std::vector<const Geometry*> input{a.get(), b.get()};
    auto result = GeometryNoder::nodeCollection(input);

    ensure_equals("1:1 mapping", result.size(), 2u);

    // Slot 0: each square's closed ring is broken at its two crossing
    // points and at its own start/end vertex -> 3 pieces each, 6 total,
    // all in one slot.
    ensure_equals("polygon slot type", result[0]->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);
    ensure_equals("polygon slot parts", result[0]->getNumGeometries(), 6u);

    // Slot 1: the line crosses four boundary points -> 5 segments.
    ensure_equals("line slot parts", result[1]->getNumGeometries(), 5u);
}

template<>
template<>
void object::test<14>()
{
    set_test_name("collection: identical members are preserved independently per slot");

    // Two identical lines plus a crosser. In-slot dedup must not dissolve
    // the duplicate across slots: each identical member yields its own
    // full (split) output.
    auto a = reader_.read("LINESTRING (0 0, 10 0)");
    auto b = reader_.read("LINESTRING (0 0, 10 0)");
    auto c = reader_.read("LINESTRING (5 -5, 5 5)");

    std::vector<const Geometry*> input{a.get(), b.get(), c.get()};
    auto result = GeometryNoder::nodeCollection(input);

    ensure_equals("1:1 mapping", result.size(), 3u);
    check_slot(reader_, result[0].get(), "MULTILINESTRING ((0 0, 5 0), (5 0, 10 0))");
    check_slot(reader_, result[1].get(), "MULTILINESTRING ((0 0, 5 0), (5 0, 10 0))");
    check_slot(reader_, result[2].get(), "MULTILINESTRING ((5 -5, 5 0), (5 0, 5 5))");
}

template<>
template<>
void object::test<15>()
{
    set_test_name("collection: polygon with hole — exterior and interior rings both noded");

    auto a = reader_.read(
        "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (3 3, 7 3, 7 7, 3 7, 3 3))");
    auto b = reader_.read("LINESTRING (-1 5, 11 5)");

    std::vector<const Geometry*> input{a.get(), b.get()};
    auto result = GeometryNoder::nodeCollection(input);

    ensure_equals("1:1 mapping", result.size(), 2u);

    // Exterior ring (3 pieces: 2 crossings + start vertex) + hole ring
    // (3 pieces likewise) = 6, all in the single polygon slot.
    ensure_equals("polygon slot type", result[0]->getGeometryTypeId(), geos::geom::GEOS_MULTILINESTRING);
    ensure_equals("polygon slot parts", result[0]->getNumGeometries(), 6u);

    // Line crosses exterior (0, 10) and hole (3, 7) at y=5 -> 5 segments.
    ensure_equals("line slot parts", result[1]->getNumGeometries(), 5u);
}

template<>
template<>
void object::test<16>()
{
    set_test_name("collection: Z and M preserved and interpolated at new nodes");

    auto a = reader_.read("LINESTRING ZM (0 0 0 0, 10 10 10 100)");
    auto b = reader_.read("LINESTRING ZM (0 10 50 0, 10 0 50 0)");

    std::vector<const Geometry*> input{a.get(), b.get()};
    auto result = GeometryNoder::nodeCollection(input);

    ensure_equals("1:1 mapping", result.size(), 2u);

    // Original vertices keep their Z/M exactly; the shared constructed node
    // at (5 5) carries the interpolated value (GEOS averages the two
    // crossing segments at the node: Z=(5+50)/2=27.5, M=(50+0)/2=25).
    check_slot(reader_, result[0].get(),
        "MULTILINESTRING ZM ((0 0 0 0, 5 5 27.5 25), (5 5 27.5 25, 10 10 10 100))");
    check_slot(reader_, result[1].get(),
        "MULTILINESTRING ZM ((0 10 50 0, 5 5 27.5 25), (5 5 27.5 25, 10 0 50 0))");
}

template<>
template<>
void object::test<17>()
{
    set_test_name("collection: Z carried through curved-member noding");

    auto line = reader_.read("LINESTRING Z (0 -1 5, 0 2 5)");
    auto arc = reader_.read("CIRCULARSTRING Z (-1 0 0, 0 1 10, 1 0 20)");

    std::vector<const Geometry*> input{line.get(), arc.get()};
    auto result = GeometryNoder::nodeCollection(input);

    ensure_equals("1:1 mapping", result.size(), 2u);
    ensure("line slot has Z", result[0]->hasZ());
    ensure_equals("arc slot type", result[1]->getGeometryTypeId(), geos::geom::GEOS_MULTICURVE);
    ensure("arc slot has Z", result[1]->hasZ());
}

template<>
template<>
void object::test<18>()
{
    set_test_name("collection: negative gridSize behaves as exact (no snapping)");

    // b's endpoint (5 0.4) is off a. With snap-rounding it would pull onto
    // (5 0) and split a (see test<7>); with exact noding (gridSize <= 0)
    // there is no intersection, so nothing is split.
    auto a = reader_.read("LINESTRING (0 0, 10 0)");
    auto b = reader_.read("LINESTRING (5 0.4, 5 10)");

    std::vector<const Geometry*> input{a.get(), b.get()};
    auto result = GeometryNoder::nodeCollection(input, -1.0);

    ensure_equals("1:1 mapping", result.size(), 2u);
    check_slot(reader_, result[0].get(), "MULTILINESTRING ((0 0, 10 0))");
    check_slot(reader_, result[1].get(), "MULTILINESTRING ((5 0.4, 5 10))");
}

} // namespace tut