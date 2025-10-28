//
// Test Suite for geos::simplify::DouglasPeuckerSimplifierTest

#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequenceFilter.h>
#include <geos/simplify/DouglasPeuckerSimplifier.h>
#include <geos/util.h>
// std
#include <string>
#include <memory>

namespace tut {
using namespace geos::simplify;

//
// Test Group
//

// Common data used by tests
struct test_dpsimp_data {
    geos::io::WKTReader wktreader;
    geos::io::WKTWriter wktwriter;

    typedef geos::geom::Geometry::Ptr GeomPtr;

    test_dpsimp_data()
        :
        wktreader()
    {}

    void
    checkDP(const std::string& wkt, double tolerance, const std::string& wkt_expected)
    {
        GeomPtr g(wktreader.read(wkt));
        GeomPtr simplified = DouglasPeuckerSimplifier::simplify(g.get(), tolerance);

        ensure("Simplified geometry is invalid!", simplified->isValid());

        GeomPtr exp(wktreader.read(wkt_expected));
        ensure_equals_exact_geometry_xyzm(exp.get(), simplified.get(), 0);
    }

    void
    checkDPNoChange(const std::string& wkt, double tolerance)
    {
        checkDP(wkt, tolerance, wkt);
    }
};

typedef test_group<test_dpsimp_data> group;
typedef group::object object;

group test_dpsimp_group("geos::simplify::DouglasPeuckerSimplifier");

//
// Test Cases
//

// 1 - testPolygonWithFlatVertices
template<>
template<>
void object::test<1>()
{
    checkDP("POLYGON ((20 220, 40 220, 60 220, 80 220, 100 220, 120 220, 140 220, 140 180, 100 180, 60 180, 20 180, 20 220))",
        10.0,
        "POLYGON ((20 220, 140 220, 140 180, 20 180, 20 220))");
}

// 2 - PolygonReductionWithSplit
template<>
template<>
void object::test<2>()
{
    checkDP("POLYGON ((40 240, 160 241, 280 240, 280 160, 160 240, 40 140, 40 240))",
        1,
        "MULTIPOLYGON (((40 240, 160 240, 40 140, 40 240)), ((160 240, 280 240, 280 160, 160 240)))");
}

// 3 - PolygonReduction
template<>
template<>
void object::test<3>()
{
    checkDP("POLYGON ((120 120, 121 121, 122 122, 220 120, 180 199, 160 200, 140 199, 120 120))",
        10,
        "POLYGON ((120 120, 220 120, 180 199, 160 200, 140 199, 120 120))");
}

// 4 - PolygonWithTouchingHole
template<>
template<>
void object::test<4>()
{
    checkDP("POLYGON ((80 200, 240 200, 240 60, 80 60, 80 200), (120 120, 220 120, 180 199, 160 200, 140 199, 120 120))",
        10,
        "POLYGON ((80 200, 240 200, 240 60, 80 60, 80 200), (120 120, 220 120, 180 199, 160 200, 140 199, 120 120))");
}

// 5 - FlattishPolygon
template<>
template<>
void object::test<5>()
{
    checkDP("POLYGON ((0 0, 50 0, 53 0, 55 0, 100 0, 70 1,  60 1, 50 1, 40 1, 0 0))",
        10,
        "POLYGON EMPTY");
}

// 6 - TinySquare
template<>
template<>
void object::test<6>()
{
    checkDP("POLYGON ((0 5, 5 5, 5 0, 0 0, 0 1, 0 5))",
        10,
        "POLYGON EMPTY");
}

// TinyHole
template<>
template<>
void object::test<7>()
{
    checkDP("POLYGON ((10 10, 10 310, 370 310, 370 10, 10 10), (160 190, 180 190, 180 170, 160 190))",
        30,
        "POLYGON ((10 10, 10 310, 370 310, 370 10, 10 10))");
}

// 7 - TinyLineString
template<>
template<>
void object::test<8>()
{
    checkDP("LINESTRING (0 5, 1 5, 2 5, 5 5)",
        10,
        "LINESTRING (0 5, 5 5)");
}

// 8 - MultiPoint
template<>
template<>
void object::test<9>()
{
    checkDPNoChange("MULTIPOINT(80 200, 240 200, 240 60, 80 60, 80 200, 140 199, 120 120)",
        10);
}

// 9 - MultiLineString
template<>
template<>
void object::test<10>()
{
    checkDP("MULTILINESTRING((0 0, 50 0, 70 0, 80 0, 100 0), (0 0, 50 1, 60 1, 100 0) )",
        10,
        "MULTILINESTRING ((0 0, 100 0), (0 0, 100 0))");
}

// 10 - GeometryCollection
template<>
template<>
void object::test<11>()
{
    checkDPNoChange("GEOMETRYCOLLECTION (MULTIPOINT (80 200, 240 200, 240 60, 80 60, 80 200, 140 199, 120 120), POLYGON ((80 200, 240 200, 240 60, 80 60, 80 200)), LINESTRING (80 200, 240 200, 240 60, 80 60, 80 200, 140 199, 120 120))",
      10.0);
}

// 11 - A kind of reversed simplification
template<>
template<>
void object::test<12>()
{
    using namespace geos::geom;

    std::string
    wkt("MULTIPOLYGON(((0.561648 1,1 1,1 0,0.468083 0,0.52758 0.00800554,0.599683 0.0280924, 0.601611 0.265374, \
                        0.622693 0.316765,0.69507 0.357497,0.695623 0.429711,0.655111 0.502298, 0.696467 0.543147,0.840712 0.593546, \
                        0.882583 0.66546,0.852357 0.748213,0.84264 0.789567,0 .832667,0.832667 0.841202,0.740538 0.873004, \
                        0.617349 0.905045,0.566576 0.977697,0.561648 1)),((0 0.801979,0.0308575 0.786234,0.0705513 0.631135, \
                        0.141616 0.527248,0.233985 0.505872,0.264777 0.526263,0.336631 0.505009,0.356603 0.422321,0.355803 0.350038, \
                        0.375252 0.205364,0.415206 0.0709182,0.45479 0,0 0,0 0,0 0.801979)))");

    GeomPtr g(wktreader.read(wkt));
    std::size_t const gN = g->getNumPoints();
    ensure_equals(gN, std::size_t(37));

    // 1) Simplify with 1/2048
    double const d1 = 1 / 2048.0;
    GeomPtr simplified1 = DouglasPeuckerSimplifier::simplify(g.get(), d1);
    ensure(simplified1->isValid());
    ensure(simplified1->equals(g.get()));
    std::size_t const simplifiedN1 = simplified1->getNumPoints();
    ensure_equals(simplifiedN1, std::size_t(36));
    //std::string const simplifiedWkd = wktwriter.write(simplified1.get());

    // 2) Multiply points by 2047
    struct Multiplier : public CoordinateSequenceFilter {
        double f;
        Multiplier(double p_f) : f(p_f) {}
        void
        filter_rw(CoordinateSequence& seq, std::size_t i) override
        {
            seq.setOrdinate(i, CoordinateSequence::X, seq[i].x * f);
            seq.setOrdinate(i, CoordinateSequence::Y, seq[i].y * f);
        }
        void
        filter_ro(const CoordinateSequence& seq, std::size_t i) override
        {
            ::geos::ignore_unused_variable_warning(seq);
            ::geos::ignore_unused_variable_warning(i);
        }
        bool
        isDone() const override
        {
            return false;
        }
        bool
        isGeometryChanged() const override
        {
            return true;
        }
    };

    Multiplier m(2047);
    g->apply_rw(m);
    std::size_t const multipliedN = g->getNumPoints();
    ensure_equals(multipliedN, std::size_t(37));
    //std::string const multipliedWkt = wktwriter.write(g.get());

    // 3) Simplify with 1.0
    double const d2 = 1.0;
    GeomPtr simplified2 = DouglasPeuckerSimplifier::simplify(g.get(), d2);
    ensure(simplified2->isValid());
    ensure(simplified2->equals(g.get()));
    std::size_t const simplifiedN2 = simplified2->getNumPoints();
    ensure_equals(simplifiedN2, std::size_t(36));
    //std::string const simplifiedWkt2 = wktwriter.write(simplified2.get());
}

// 13 - Polygon with inner ring whose extent is less than the simplify distance (#741)
template<>
template<>
void object::test<13>()
{
    checkDP(
        "POLYGON ((0 0,0 1,1 1,0 0),(0.1 0.1,0.2 0.1,0.2 0.2,0.1 0.1))",
        0.5,
        "POLYGON ((0 0,0 1,1 1,0 0))"
        );
}

/**
* Test that a polygon made invalid by simplification
* is fixed in a sensible way.
* Fixed by buffer(0) area-base orientation
* See https://github.com/locationtech/jts/issues/498
*/
template<>
template<>
void object::test<14>()
{
    checkDP(
        "POLYGON ((21.32686 47.78723, 21.32386 47.79023, 21.32186 47.80223, 21.31486 47.81023, 21.32786 47.81123, 21.33986 47.80223, 21.33886 47.81123, 21.32686 47.82023, 21.32586 47.82723, 21.32786 47.82323, 21.33886 47.82623, 21.34186 47.82123, 21.36386 47.82223, 21.40686 47.81723, 21.32686 47.78723))",
        0.0036,
        "POLYGON ((21.32686 47.78723, 21.31486 47.81023, 21.32786 47.81123, 21.33986 47.80223, 21.328068201892744 47.823286782334385, 21.33886 47.82623, 21.34186 47.82123, 21.40686 47.81723, 21.32686 47.78723))"
        );
}

  /**
   * Test that a collapsed polygon is removed.
   * Tests regression caused by unported JTS code.
   *
   * See https://trac.osgeo.org/geos/ticket/1115
   */
template<>
template<>
void object::test<15>()
{
    checkDP(
        "MULTIPOLYGON (((-76.02716827 36.55671692, -75.99866486 36.55665207, -75.91191864 36.54253006, -75.92480469 36.47397614, -75.97727966 36.4780159, -75.97628784 36.51792526, -76.02716827 36.55671692)), ((-75.90198517 36.55619812, -75.8781662 36.55587387, -75.77315521 36.22925568, -75.78317261 36.22519302, -75.90198517 36.55619812)))",
        0.05,
        "POLYGON ((-76.02716827 36.55671692, -75.91191864 36.54253006, -75.92480469 36.47397614, -76.02716827 36.55671692))"
        );
}

// Test that start point of a polygon can be removed
template<>
template<>
void object::test<16>()
{
    checkDP("POLYGON ((1 0, 2 0, 2 2, 0 2, 0 0, 1 0))",
        0,
        "POLYGON ((2 0, 2 2, 0 2, 0 0, 2 0))");
}

// Test that start point of a closed LineString is not changed
template<>
template<>
void object::test<17>()
{
    checkDPNoChange("LINESTRING (1 0, 2 0, 2 2, 0 2, 0 0, 1 0)",
        0);
}

// testPolygonRemoveFlatEndpoint
// see https://trac.osgeo.org/geos/ticket/1064
template<>
template<>
void object::test<18>()
{
    checkDP(
      "POLYGON ((42 42, 0 42, 0 100, 42 100, 100 42, 42 42))",
        1,
        "POLYGON ((0 42, 0 100, 42 100, 100 42, 0 42))"
        );
}

// testPolygonEndpointCollapse
template<>
template<>
void object::test<19>()
{
    checkDP(
        "POLYGON ((5 2, 9 1, 1 1, 5 2))",
        1,
        "POLYGON EMPTY"
        );
}

// https://github.com/libgeos/geos/issues/1078
template<>
template<>
void object::test<20>()
{
    auto gfact = GeometryFactory::getDefaultInstance();
    auto pt = gfact->createPoint(Coordinate{0,  0});

    try {
        DouglasPeuckerSimplifier::simplify(pt.get(), std::numeric_limits<double>::quiet_NaN());
        fail("Exception not thrown.");
    } catch (const geos::util::IllegalArgumentException&) {
    }
}

template<>
template<>
void object::test<21>()
{
    set_test_name("Z values are preserved");

    checkDP("POLYGON Z ((20 220 5, 40 220 10, 60 220 15, 80 220 20, 100 220 25, 120 220 30, 140 220 35, 140 180 40, 100 180 45, 60 180 50, 20 180 55, 20 220 5))",
        10.0,
        "POLYGON Z ((20 220 5, 140 220 35, 140 180 40, 20 180 55, 20 220 5))");
}

template<>
template<>
void object::test<22>()
{
    set_test_name("M values are preserved");

    checkDP("POLYGON M ((20 220 5, 40 220 10, 60 220 15, 80 220 20, 100 220 25, 120 220 30, 140 220 35, 140 180 40, 100 180 45, 60 180 50, 20 180 55, 20 220 5))",
        10.0,
        "POLYGON M ((20 220 5, 140 220 35, 140 180 40, 20 180 55, 20 220 5))");
}

template<>
template<>
void object::test<23>()
{
    set_test_name("Z/M values preserved when removing polygon start point");

    checkDP("POLYGON ZM ((1 0 5 7, 2 0 10 9, 2 2 15 11, 0 2 20 13, 0 0 25 15, 1 0 5 7))",
        0,
        "POLYGON ZM ((2 0 10 9, 2 2 15 11, 0 2 20 13, 0 0 25 15, 2 0 10 9))");
}

} // namespace tut
