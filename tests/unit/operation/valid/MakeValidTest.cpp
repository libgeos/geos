#include <tut/tut.hpp>
// geos
#include <geos/constants.h> // for std::isnan
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/MultiPolygon.h>
#include <geos/operation/valid/MakeValid.h>
#include <geos/operation/overlayng/OverlayNG.h>
#include <geos/operation/overlayng/OverlayNGRobust.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/util.h>

#include <algorithm>
#include <limits>
#include <random>
#include <utility.h>

using namespace geos::geom;
using namespace geos::operation::valid;

namespace tut {
//
// Test Group
//

struct test_makevalid_data {
    test_makevalid_data() {}
};

typedef test_group<test_makevalid_data> group;
typedef group::object object;

group test_makevalid_group("geos::operation::valid::MakeValid");

// Split a geometry into per-dimension geometry lists. Overlay operations
// refuse mixed-dimension inputs, so equivalence checks are done per
// dimension (area/line/point).
void
splitByDimension(const Geometry* g,
                 std::vector<const Geometry*>& polys,
                 std::vector<const Geometry*>& lines,
                 std::vector<const Geometry*>& points)
{
    switch(g->getGeometryTypeId()) {
        case GEOS_GEOMETRYCOLLECTION:
            for(std::size_t i = 0; i < g->getNumGeometries(); i++) {
                splitByDimension(g->getGeometryN(i), polys, lines, points);
            }
            break;
        case GEOS_POLYGON:
        case GEOS_MULTIPOLYGON: polys.push_back(g); break;
        case GEOS_LINESTRING:
        case GEOS_MULTILINESTRING: lines.push_back(g); break;
        case GEOS_POINT:
        case GEOS_MULTIPOINT: points.push_back(g); break;
        default: break;
    }
}

bool
hasEmptyElement(const Geometry* g)
{
    if(g->isEmpty()) {
        return true;
    }
    if(g->getNumGeometries() > 1 ||
       g->getGeometryTypeId() == GEOS_GEOMETRYCOLLECTION) {
        for(std::size_t i = 0; i < g->getNumGeometries(); i++) {
            if(hasEmptyElement(g->getGeometryN(i))) {
                return true;
            }
        }
    }
    return false;
}

// Build a unit square polygon with lower-left corner (x, y).
static std::unique_ptr<Polygon>
makeSquare(const GeometryFactory* gf, double x, double y)
{
    auto cs = std::make_unique<CoordinateSequence>();
    cs->add(x, y);
    cs->add(x + 1, y);
    cs->add(x + 1, y + 1);
    cs->add(x, y + 1);
    cs->add(x, y);
    return gf->createPolygon(gf->createLinearRing(std::move(cs)));
}

// Check that two geometries cover the same area/line/point sets, by
// asserting their per-dimension symmetric differences are empty.
void
ensureTopologicallyEqual(const Geometry* lhs, const Geometry* rhs,
                         const std::string& context)
{
    std::vector<const Geometry*> lp, ll, ln, rp, rl, rn;
    splitByDimension(lhs, lp, ll, ln);
    splitByDimension(rhs, rp, rl, rn);
    const char* dimName[] = {"area", "line", "point"};
    std::vector<const Geometry*>* ldims[] = {&lp, &ll, &ln};
    std::vector<const Geometry*>* rdims[] = {&rp, &rl, &rn};
    using geos::operation::overlayng::OverlayNG;
    using geos::operation::overlayng::OverlayNGRobust;
    for(std::size_t d = 0; d < 3; d++) {
        bool lEmpty = ldims[d]->empty();
        bool rEmpty = rdims[d]->empty();
        if(lEmpty != rEmpty) {
            std::string what = context + ": " + dimName[d] + " present in one input only";
            fail(what);
        }
        if(lEmpty) {
            continue;
        }
        std::vector<std::unique_ptr<Geometry>> lgeoms;
        std::vector<std::unique_ptr<Geometry>> rgeoms;
        for(const Geometry* g : *ldims[d]) {
            lgeoms.push_back(g->clone());
        }
        for(const Geometry* g : *rdims[d]) {
            rgeoms.push_back(g->clone());
        }
        auto lg = lhs->getFactory()->buildGeometry(std::move(lgeoms));
        auto rg = rhs->getFactory()->buildGeometry(std::move(rgeoms));
        auto symdif = OverlayNGRobust::Overlay(lg.get(), rg.get(), OverlayNG::SYMDIFFERENCE);
        ensure(context + ": " + dimName[d] + " symmetric difference not empty",
               symdif->isEmpty());
    }
}

//
// Test Cases
//

// https://github.com/libgeos/geos/issues/265
template<>
template<>
void object::test<1>
()
{
    auto cs = geos::detail::make_unique<CoordinateSequence>();
    cs->add(2.22, 2.28);
    cs->add(7.67, 2.06);
    cs->add(10.98, 7.70);
    cs->add(9.39, 5.00);
    cs->add(7.96, 7.12);
    cs->add(6.77, 5.16);
    cs->add(7.43, 6.24);
    cs->add(3.70, 7.22);
    cs->add(5.72, 5.77);
    cs->add(4.18, 10.74);
    cs->add(2.20, 6.83);
    cs->add(2.22, 2.28);

    auto gf = GeometryFactory::getDefaultInstance();
    auto lr = gf->createLinearRing(std::move(cs));
    auto errplyg = gf->createPolygon(std::move(lr));

    ensure(!errplyg->isValid());

    MakeValid mkvalid;
    auto validGeom = mkvalid.build(errplyg.get());

    ensure(validGeom->isValid());
}

// template<>
// template<>
// void object::test<2>
// ()
// {


//     std::ifstream ifs("GoesBathymetryBug.txt");
//     std::string content((std::istreambuf_iterator<char>(ifs)),
//                        (std::istreambuf_iterator<char>()));

//     geos::io::WKTReader reader;
//     auto geom(reader.read(content));

//     // auto gf = GeometryFactory::getDefaultInstance();

//     // auto cs = gf->getCoordinateSequenceFactory()->create(std::move(v));
//     // auto lr = gf->createLinearRing(std::move(cs));
//     // auto errplyg = gf->createPolygon(std::move(lr));

//     // ensure(!errplyg->isValid());

//     MakeValid mkvalid;
//     auto validGeom = mkvalid.build(geom.get());
//     ensure("MakeValid output is not valid", validGeom->isValid());

//     geos::io::WKTWriter writer;
//     writer.setOutputDimension(2);
//     writer.setTrim(true);
//     std::string result = writer.write(validGeom.get());
//     std::cout << result << std::endl;

// }

template<>
template<>
void object::test<3>()
{
    auto gf = GeometryFactory::getDefaultInstance();

    auto mp = gf->createMultiPolygon();

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->isValid());
}

template<>
template<>
void object::test<4>()
{
    geos::io::WKBReader reader;
    std::stringstream input;
    // From PostGIS test: https://github.com/postgis/postgis/blob/5e310cf6ad646702e5574eb3aa2391021dcdd8c5/liblwgeom/cunit/cu_geos.c#L147
    input << "0103000000010000000900000062105839207df640378941e09d491c41ced67431387df640c667e7d398491"
             "c4179e92631387df640d9cef7d398491c41fa7e6abcf87df640cdcccc4c70491c41e3a59bc4527df64052b8"
             "1e053f491c41cdcccccc5a7ef640e3a59bc407491c4104560e2da27df640aaf1d24dd3481c41e9263108c67"
             "bf64048e17a1437491c4162105839207df640378941e09d491c41";

    auto g = reader.readHEX(input);

    geos::operation::valid::MakeValid mv;

    auto result = mv.build(g.get());

    ensure_equals_geometry(result.get(),
                           "POLYGON((92127.546 463452.075,92117.173 463439.755,92133.675 463425.942,"
                           "92122.136 463412.826,92092.377 463437.77,92114.014 463463.469,92115.512 463462.207,"
                           "92115.51207431706 463462.2069374289,92127.546 463452.075))");
}

// MultiPolygon with disjoint valid parts and one invalid (self-intersecting)
// part: the valid parts must not be dragged through the polygonal
// re-building algorithm and must come out with identical coordinates.
// https://github.com/libgeos/geos/issues/1504
template<>
template<>
void object::test<5>()
{
    geos::io::WKTReader reader;
    auto mp = reader.read(
        "MULTIPOLYGON (((0 0, 1 0, 1 1, 0 1, 0 0)), ((10 10, 11 10, 11 11, 10 11, 10 10)),"
        " ((20 20, 21 21, 21 20, 20 21, 20 20)))");

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->isValid());
    // Valid parts kept as-is (exact clone, ring orientation included),
    // invalid part split into its two lobes, all packaged as a single
    // MultiPolygon of areas in input order.
    auto expected = reader.read(
        "MULTIPOLYGON (((0 0, 1 0, 1 1, 0 1, 0 0)), ((10 10, 11 10, 11 11, 10 11, 10 10)),"
        " ((20.5 20.5, 20 20, 20 21, 20.5 20.5)), ((20.5 20.5, 21 21, 21 20, 20.5 20.5)))");
    ensure(result->equalsExact(expected.get()));
}

// A MultiPolygon whose parts all share envelopes is one connected
// component and takes the unmodified whole-geometry path.
template<>
template<>
void object::test<6>()
{
    geos::io::WKTReader reader;
    auto mp = reader.read(
        "MULTIPOLYGON (((0 0, 1 0, 1 1, 0 1, 0 0)), ((1 0, 2 0, 2 1, 1 1, 1 0)))");

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->isValid());
    // Shared edge becomes a cut edge, exactly as before the optimization.
    ensure_equals_geometry(result.get(),
        "GEOMETRYCOLLECTION (POLYGON ((1 0, 0 0, 0 1, 1 1, 2 1, 2 0, 1 0)),"
        " LINESTRING (1 0, 1 1))");
}

// Bowties touching at a single point interact and are processed together.
template<>
template<>
void object::test<7>()
{
    geos::io::WKTReader reader;
    auto mp = reader.read(
        "MULTIPOLYGON (((0 0, 1 1, 1 0, 0 1, 0 0)), ((1 1, 2 2, 2 1, 1 2, 1 1)))");

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->isValid());
    ensure_equals_geometry(result.get(),
        "MULTIPOLYGON (((0.5 0.5, 0 0, 0 1, 0.5 0.5)), ((0.5 0.5, 1 1, 1 0, 0.5 0.5)),"
        " ((1.5 1.5, 1 1, 1 2, 1.5 1.5)), ((1.5 1.5, 2 2, 2 1, 1.5 1.5)))");
}

// Bowties whose lobes cross each other interact and are processed together.
template<>
template<>
void object::test<8>()
{
    geos::io::WKTReader reader;
    auto mp = reader.read(
        "MULTIPOLYGON (((0 0, 1 1, 1 0, 0 1, 0 0)), ((-0.5 0.25, 0.5 1.25, 0.5 0.25, -0.5 1.25, -0.5 0.25)))");

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->isValid());
    ensure_equals_geometry(result.get(),
        "MULTIPOLYGON (((0.375 0.375, 0 0, 0 0.75, 0.375 0.375)), ((0 0.75, 0 1, 0.125 0.875, 0 0.75)),"
        " ((0.125 0.875, 0.5 1.25, 0.5 0.5, 0.125 0.875)), ((0.5 0.5, 0.5 0.25, 0.375 0.375, 0.5 0.5)),"
        " ((0.5 0.5, 1 1, 1 0, 0.5 0.5)), ((0 0.75, -0.5 0.25, -0.5 1.25, 0 0.75)))");
}

// Empty elements propagate through, as in the whole-geometry path.
template<>
template<>
void object::test<9>()
{
    geos::io::WKTReader reader;
    auto mp = reader.read("MULTIPOLYGON (EMPTY, ((10 10, 11 11, 11 10, 10 10)))");

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->isValid());
    ensure(hasEmptyElement(result.get()));
    ensure_equals_geometry(result.get(),
        "MULTIPOLYGON (EMPTY, ((10 10, 11 11, 11 10, 10 10)))");
}

// Empty MultiPolygon and fully-collapsed parts.
template<>
template<>
void object::test<10>()
{
    geos::io::WKTReader reader;
    auto mpEmpty = reader.read("MULTIPOLYGON (EMPTY)");
    MakeValid mv;
    auto resultEmpty = mv.build(mpEmpty.get());
    ensure(resultEmpty->isValid());
    ensure_equals_geometry(resultEmpty.get(), "MULTIPOLYGON (EMPTY)");

    auto mpCollapsed = reader.read(
        "MULTIPOLYGON (((0 0, 1 1, 2 2, 1 1, 0 0)), ((10 10, 11 11, 11 10, 10 10)))");
    auto resultCollapsed = mv.build(mpCollapsed.get());
    ensure(resultCollapsed->isValid());
    ensure_equals_geometry(resultCollapsed.get(),
        "GEOMETRYCOLLECTION (POLYGON ((10 10, 11 11, 11 10, 10 10)),"
        " LINESTRING (0 0, 1 1, 2 2))");
}

// Nested shells interact (envelopes overlap) and are processed together.
template<>
template<>
void object::test<11>()
{
    geos::io::WKTReader reader;
    auto mp = reader.read(
        "MULTIPOLYGON (((0 0, 10 0, 10 10, 0 10, 0 0)), ((2 2, 4 2, 4 4, 2 4, 2 2)))");

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->isValid());
    ensure_equals_geometry(result.get(),
        "POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0), (2 2, 4 2, 4 4, 2 4, 2 2))");
}

// Z values are interpolated in the invalid part; a valid part without Z
// is returned unchanged (dimension included), matching the documented
// fast path for already-valid geometries.
// NOTE: the whole-geometry algorithm used to smear interpolated Z values
// onto the valid parts of mixed-dimension inputs; per-component
// processing preserves each part's own coordinates instead.
template<>
template<>
void object::test<12>()
{
    auto gf = GeometryFactory::getDefaultInstance();
    // 2D square (valid) + Z bowtie (invalid), envelope-disjoint
    auto cs = std::make_unique<CoordinateSequence>(5u, 3u);
    cs->setAt(Coordinate(0, 0, 1), 0);
    cs->setAt(Coordinate(1, 1, 2), 1);
    cs->setAt(Coordinate(1, 0, 3), 2);
    cs->setAt(Coordinate(0, 1, 4), 3);
    cs->setAt(Coordinate(0, 0, 1), 4);
    auto bowtieZ = gf->createPolygon(gf->createLinearRing(std::move(cs)));

    std::vector<std::unique_ptr<Geometry>> parts;
    parts.push_back(makeSquare(gf, 10, 10));
    parts.push_back(std::move(bowtieZ));
    auto mp = gf->createMultiPolygon(std::move(parts));

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->isValid());
    ensure_equals(result->getGeometryTypeId(), GEOS_MULTIPOLYGON);
    ensure_equals(result->getNumGeometries(), std::size_t(3));
    bool saw2D = false;
    bool sawZ = false;
    for(std::size_t i = 0; i < result->getNumGeometries(); i++) {
        const Polygon* p = static_cast<const Polygon*>(result->getGeometryN(i));
        if(p->getCoordinateDimension() == 2) {
            saw2D = true;
        } else {
            sawZ = true;
        }
    }
    ensure("the valid non-Z part should keep its 2D coordinates", saw2D);
    ensure("the invalid Z part should keep interpolated Z", sawZ);
    ensure_equals("area of square + bowtie lobes is preserved",
                  result->getArea(), 1.5, 1e-9);
}

// Already-valid MultiPolygon is returned as an exact clone (fast path).
template<>
template<>
void object::test<13>()
{
    geos::io::WKTReader reader;
    auto mp = reader.read(
        "MULTIPOLYGON (((0 0, 1 0, 1 1, 0 1, 0 0)), ((1 1, 2 1, 2 2, 1 2, 1 1)))");
    mp->setSRID(4326);

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->equalsExact(mp.get()));
    ensure_equals(result->getSRID(), 4326);
}

// SRID carried by the geometry factory is preserved when invalid parts
// force per-component processing.
template<>
template<>
void object::test<14>()
{
    auto gf = GeometryFactory::create(nullptr, 26910);
    geos::io::WKTReader reader(gf.get());
    auto mp = reader.read(
        "MULTIPOLYGON (((0 0, 1 0, 1 1, 0 1, 0 0)), ((10 10, 11 11, 11 10, 10 11, 10 10)))");

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->isValid());
    ensure_equals(result->getSRID(), 26910);
}

// Differential test: for polygons with pairwise disjoint envelopes the
// MultiPolygon result must cover the same area/line/point sets as the
// equivalent GeometryCollection, which MakeValid processes member-wise.
template<>
template<>
void object::test<15>()
{
    auto gf = GeometryFactory::getDefaultInstance();
    MakeValid mv;

    auto square = [gf](double x, double y) {
        auto cs = std::make_unique<CoordinateSequence>();
        cs->add(x, y);
        cs->add(x + 1, y);
        cs->add(x + 1, y + 1);
        cs->add(x, y + 1);
        cs->add(x, y);
        return gf->createPolygon(gf->createLinearRing(std::move(cs)));
    };
    // self-intersecting bowtie
    auto bowtie = [gf](double x, double y) {
        auto cs = std::make_unique<CoordinateSequence>();
        cs->add(x, y);
        cs->add(x + 1, y + 1);
        cs->add(x + 1, y);
        cs->add(x, y + 1);
        cs->add(x, y);
        return gf->createPolygon(gf->createLinearRing(std::move(cs)));
    };
    // polygon with an outward self-touching spike
    auto spiked = [gf](double x, double y) {
        auto cs = std::make_unique<CoordinateSequence>();
        cs->add(x, y);
        cs->add(x + 2, y);
        cs->add(x + 2, y + 2);
        cs->add(x + 1, y - 1);
        cs->add(x, y + 2);
        cs->add(x, y);
        return gf->createPolygon(gf->createLinearRing(std::move(cs)));
    };

    for(unsigned seed = 1; seed <= 8; seed++) {
        std::mt19937 rng(seed);
        std::uniform_real_distribution<double> jitter(0.0, 0.9);
        std::uniform_real_distribution<double> pick(0.0, 1.0);

        std::vector<std::unique_ptr<Geometry>> parts;
        for(int i = 0; i < 12; i++) {
            for(int j = 0; j < 12; j++) {
                double x = 4.0 * i + jitter(rng);
                double y = 4.0 * j + jitter(rng);
                double p = pick(rng);
                if(p < 0.10) {
                    parts.push_back(bowtie(x, y));
                } else if(p < 0.15) {
                    parts.push_back(spiked(x, y));
                } else {
                    parts.push_back(square(x, y));
                }
            }
        }
        for(int k = 0; k < 3; k++) {
            parts.push_back(bowtie(-20.0 - 2.0 * k, -20.0 - 2.0 * k));
        }
        std::shuffle(parts.begin(), parts.end(), rng);

        std::vector<std::unique_ptr<Geometry>> gcParts;
        gcParts.reserve(parts.size());
        for(const auto& p : parts) {
            gcParts.push_back(p->clone());
        }

        auto mp = gf->createMultiPolygon(std::move(parts));
        auto gc = gf->createGeometryCollection(std::move(gcParts));

        auto resMP = mv.build(mp.get());
        auto resGC = mv.build(gc.get());
        ensure("multipolygon result is valid", resMP->isValid());
        ensure("collection result is valid", resGC->isValid());
        ensureTopologicallyEqual(resMP.get(), resGC.get(),
                                 "seed " + std::to_string(seed));
    }
}

// A component made of several interacting polygons is dissolved together,
// while independent components are processed separately and merged into
// the same area/cut-edges structure the whole-geometry algorithm returns.
template<>
template<>
void object::test<16>()
{
    geos::io::WKTReader reader;
    auto mp = reader.read(
        "MULTIPOLYGON (((0 0, 1 0, 1 1, 0 1, 0 0)), ((0.5 0, 1.5 0, 1.5 1, 0.5 1, 0.5 0)),"
        " ((20 20, 21 21, 21 20, 20 21, 20 20)), ((40 40, 41 40, 41 41, 40 41, 40 40)))");

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->isValid());
    ensure_equals_geometry(result.get(),
        "GEOMETRYCOLLECTION (MULTIPOLYGON (((0.5 0, 0 0, 0 1, 0.5 1, 1 1, 1.5 1, 1.5 0, 1 0, 0.5 0)),"
        " ((20.5 20.5, 20 20, 20 21, 20.5 20.5)), ((20.5 20.5, 21 21, 21 20, 20.5 20.5)),"
        " ((40 40, 40 41, 41 41, 41 40, 40 40))),"
        " MULTILINESTRING ((1 0, 1 1), (0.5 1, 0.5 0)))");
}

// Empty polygons have no usable envelope; they must not disturb the
// component sweep and propagate only when the fast path clones them.
template<>
template<>
void object::test<17>()
{
    geos::io::WKTReader reader;
    // empty polygon + overlapping squares: invalid, reaches the
    // per-component path with a null-envelope part present
    auto mp = reader.read(
        "MULTIPOLYGON (EMPTY, ((0 0, 1 0, 1 1, 0 1, 0 0)),"
        " ((0.5 0, 1.5 0, 1.5 1, 0.5 1, 0.5 0)))");

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->isValid());
    // Same output the whole-geometry algorithm produces: the empty element
    // contributes nothing and is not present in the result.
    ensure(!hasEmptyElement(result.get()));
    ensure_equals_geometry(result.get(),
        "GEOMETRYCOLLECTION (POLYGON ((0.5 0, 0 0, 0 1, 0.5 1, 1 1, 1.5 1, 1.5 0, 1 0, 0.5 0)),"
        " MULTILINESTRING ((1 0, 1 1), (0.5 1, 0.5 0)))");
}

// An empty part and envelope-disjoint invalid parts: the empty part goes
// through the component sweep (its envelope is unusable) and contributes
// nothing to the result, exactly as the whole-geometry algorithm behaves.
template<>
template<>
void object::test<18>()
{
    geos::io::WKTReader reader;
    auto mp = reader.read(
        "MULTIPOLYGON (EMPTY, ((10 10, 11 11, 11 10, 10 11, 10 10)),"
        " ((40 40, 41 40, 41 41, 40 41, 40 40)))");

    MakeValid mv;
    auto result = mv.build(mp.get());

    ensure(result->isValid());
    ensure(!hasEmptyElement(result.get()));
    ensure_equals_geometry(result.get(),
        "MULTIPOLYGON (((10.5 10.5, 10 10, 10 11, 10.5 10.5)),"
        " ((10.5 10.5, 11 11, 11 10, 10.5 10.5)),"
        " ((40 40, 40 41, 41 41, 41 40, 40 40)))");
}

// Non-finite envelopes (Inf/NaN ordinates) are excluded from the
// component sweep before the sort; grouping must never read their
// ordinates. The non-finite linework itself is rejected by the overlay
// algorithm exactly as it is by the whole-geometry path.
template<>
template<>
void object::test<19>()
{
    auto gf = GeometryFactory::getDefaultInstance();
    geos::io::WKTReader reader(gf);
    MakeValid mv;

    auto cs = std::make_unique<CoordinateSequence>();
    const double inf = std::numeric_limits<double>::infinity();
    cs->add(inf, 0.0);
    cs->add(1.0, 0.0);
    cs->add(1.0, 1.0);
    cs->add(inf, 1.0);
    cs->add(inf, 0.0);
    auto infPoly = gf->createPolygon(gf->createLinearRing(std::move(cs)));
    auto square = reader.read("POLYGON ((10 10, 11 10, 11 11, 10 11, 10 10))");

    std::vector<std::unique_ptr<Geometry>> parts;
    parts.push_back(std::move(infPoly));
    parts.push_back(std::move(square));
    auto mp = gf->createMultiPolygon(std::move(parts));

    bool threw = false;
    try {
        mv.build(mp.get());
    } catch(const geos::util::IllegalArgumentException&) {
        threw = true;
    }
    ensure("non-finite linework must fail the same way the"
           " whole-geometry algorithm does", threw);
}

// M values of valid parts survive the pass-through like Z values do.
template<>
template<>
void object::test<20>()
{
    auto gf = GeometryFactory::getDefaultInstance();
    geos::io::WKTReader reader(gf);
    MakeValid mv;

    // valid square with Z and M, envelope-disjoint from an invalid bowtie
    auto cs = std::make_unique<CoordinateSequence>(5u, 4u);
    cs->setAt(Coordinate(10, 10), 0);
    cs->setAt(Coordinate(11, 10), 1);
    cs->setAt(Coordinate(11, 11), 2);
    cs->setAt(Coordinate(10, 11), 3);
    cs->setAt(Coordinate(10, 10), 4);
    for(std::size_t i = 0; i < 5; i++) {
        cs->setZ(i, 5.0);
        cs->setM(i, 7.0);
    }

    std::vector<std::unique_ptr<Geometry>> parts;
    parts.push_back(gf->createPolygon(gf->createLinearRing(std::move(cs))));
    parts.push_back(reader.read("POLYGON ((0 0, 1 1, 1 0, 0 1, 0 0))"));
    auto mp = gf->createMultiPolygon(std::move(parts));

    auto result = mv.build(mp.get());

    ensure(result->isValid());
    ensure_equals(result->getGeometryTypeId(), GEOS_MULTIPOLYGON);
    bool sawM = false;
    for(std::size_t i = 0; i < result->getNumGeometries(); i++) {
        const auto* p = static_cast<const Polygon*>(result->getGeometryN(i));
        const auto* seq = p->getExteriorRing()->getCoordinatesRO();
        if(seq->getM(0) == 7.0) {
            sawM = true;
            ensure_equals("Z kept alongside M", seq->getZ(0), 5.0);
        }
    }
    ensure("the valid part should keep its M values", sawM);
}

// A long-X, Y-disjoint invalid sentinel must not pull envelope-disjoint
// valid squares into the same component. The squares stay pass-through
// clones; the bowtie is repaired on its own. (Complexity of the sweep
// for this shape is covered by perf_makevalid sentinel.)
template<>
template<>
void object::test<21>()
{
    auto gf = GeometryFactory::getDefaultInstance();
    geos::io::WKTReader reader(gf);
    MakeValid mv;

    std::vector<std::unique_ptr<Geometry>> parts;
    // Invalid bowtie spanning X=[0, 20] at Y=100, Y-disjoint from the squares.
    parts.push_back(reader.read(
        "POLYGON ((0 100, 20 101, 20 100, 0 101, 0 100))"));

    std::vector<std::unique_ptr<Geometry>> squares;
    for(double x = 0; x <= 16; x += 4) {
        auto cs = std::make_unique<CoordinateSequence>();
        cs->add(x, 0.0);
        cs->add(x + 1, 0.0);
        cs->add(x + 1, 1.0);
        cs->add(x, 1.0);
        cs->add(x, 0.0);
        squares.push_back(gf->createPolygon(gf->createLinearRing(std::move(cs))));
        parts.push_back(squares.back()->clone());
    }
    auto mp = gf->createMultiPolygon(std::move(parts));

    auto result = mv.build(mp.get());

    ensure(result->isValid());
    ensure_equals(result->getGeometryTypeId(), GEOS_MULTIPOLYGON);
    // Five valid squares kept as-is, plus two bowtie lobes.
    ensure_equals(result->getNumGeometries(), std::size_t(7));
    for(const auto& sq : squares) {
        bool found = false;
        for(std::size_t i = 0; i < result->getNumGeometries(); i++) {
            if(result->getGeometryN(i)->equalsExact(sq.get())) {
                found = true;
                break;
            }
        }
        ensure("each valid square must come out with identical coordinates",
               found);
    }
}

} // namespace tut
