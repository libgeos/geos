//
// Test Suite for geos::io::EsriJsonWriter
//

#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>

#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/io/EsriJsonWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/util/UnsupportedOperationException.h>

#include <string>

namespace tut {

struct test_esrijsonwriter_data {
    geos::io::WKTReader reader_;
    geos::io::EsriJsonWriter writer_;

    test_esrijsonwriter_data()
    {
        writer_.setTolerance(1e-3);
    }

    std::string write(const std::string& wkt)
    {
        auto g = reader_.read(wkt);
        return writer_.write(*g);
    }

    static std::size_t count(const std::string& text, const std::string& what)
    {
        std::size_t n = 0;
        for (std::size_t pos = text.find(what); pos != std::string::npos; pos = text.find(what, pos + 1)) {
            n++;
        }
        return n;
    }

    static void ensureBalanced(const std::string& text)
    {
        int square = 0, curly = 0;
        for (char c : text) {
            square += (c == '[') - (c == ']');
            curly += (c == '{') - (c == '}');
            ensure(square >= 0 && curly >= 0);
        }
        ensure_equals(square, 0);
        ensure_equals(curly, 0);
    }
};

typedef test_group<test_esrijsonwriter_data> group;
typedef group::object object;

group test_esrijsonwriter_group("geos::io::EsriJsonWriter");

template<>
template<>
void object::test<1>()
{
    set_test_name("points, linear geometry and polygons");

    ensure_equals(write("POINT (1 2.5)"), std::string("{\"x\":1,\"y\":2.5}"));
    ensure_equals(write("POINT EMPTY"), std::string("{\"x\":null}"));
    ensure_equals(write("MULTIPOINT ((1 2), (3 4))"), std::string("{\"points\":[[1,2],[3,4]]}"));
    ensure_equals(write("LINESTRING (0 0, 1 1, 2 0)"), std::string("{\"paths\":[[[0,0],[1,1],[2,0]]]}"));
    ensure_equals(write("MULTILINESTRING ((0 0, 1 1), (5 5, 6 6))"),
                  std::string("{\"paths\":[[[0,0],[1,1]],[[5,5],[6,6]]]}"));
    ensure_equals(write("LINESTRING EMPTY"), std::string("{\"paths\":[]}"));

    // Shells clockwise, holes counterclockwise, whatever the input.
    const std::string cw = "{\"rings\":[[[0,0],[0,10],[10,10],[10,0],[0,0]],[[2,2],[4,2],[4,4],[2,4],[2,2]]]}";
    ensure_equals(write("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0), (2 2, 2 4, 4 4, 4 2, 2 2))"), cw);
    ensure_equals(write("POLYGON ((0 0, 0 10, 10 10, 10 0, 0 0), (2 2, 4 2, 4 4, 2 4, 2 2))"), cw);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("circular arcs");

    ensure_equals(write("CIRCULARSTRING (0 0, 1 1, 2 0)"),
                  std::string("{\"curvePaths\":[[[0,0],{\"c\":[[2,0],[1,1]]}]]}"));
    ensure_equals(write("COMPOUNDCURVE ((0 0, 1 0), CIRCULARSTRING (1 0, 2 1, 3 0), (3 0, 4 0))"),
                  std::string("{\"curvePaths\":[[[0,0],[1,0],{\"c\":[[3,0],[2,1]]},[4,0]]]}"));
    // Writing leaves the exact statistics.
    ensure_equals(writer_.getStats().numSegments, 3u);
    ensure_equals(writer_.getStats().numApproximated, 0u);
    ensure_equals(writer_.getStats().maxDeviation, 0.0);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("elliptic arcs follow the Esri parameter order");

    // Quarter of the ellipse with semi-axes 3 and 2, counterclockwise from (3, 0).
    const std::string json = write(
        "ELLIPTICALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), REFERENCEDIRECTIONS "
        "(VECTOR (1 0), VECTOR (0 1))), UAXISLENGTH 3, VAXISLENGTH 2, "
        "STARTANGLE ANGLE RADIANS (0), ENDANGLE ANGLE RADIANS (1.5707963267948966))");
    // end point, centre, minor, clockwise, rotation, semi-major axis, ratio
    ensure_equals(json, std::string("{\"curvePaths\":[[[3,0],{\"a\":[[0,2],[0,0],1,0,0,3,0.6666666666666666]}]]}"));

    // The same quarter run clockwise from (0, 2) to (3, 0).
    const std::string cw = write(
        "ELLIPTICALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), REFERENCEDIRECTIONS "
        "(VECTOR (1 0), VECTOR (0 1))), UAXISLENGTH 3, VAXISLENGTH 2, "
        "STARTANGLE ANGLE RADIANS (1.5707963267948966), ENDANGLE ANGLE RADIANS (0))");
    ensure_equals(cw, std::string("{\"curvePaths\":[[[0,2],{\"a\":[[3,0],[0,0],1,1,0,3,0.6666666666666666]}]]}"));

    // A major axis along v: the semi-major axis is the v length and the
    // major axis points along +y (rotation pi/2).
    const std::string tall = write(
        "ELLIPTICALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), REFERENCEDIRECTIONS "
        "(VECTOR (1 0), VECTOR (0 1))), UAXISLENGTH 2, VAXISLENGTH 3, "
        "STARTANGLE ANGLE RADIANS (0), ENDANGLE ANGLE RADIANS (1.5707963267948966))");
    ensure("major axis 3", tall.find(",3,0.6666666666666666]}") != std::string::npos);
    ensure("rotation about pi/2", tall.find(",1.5707963267948966,3,") != std::string::npos
           || tall.find(",1.570796326794896") != std::string::npos);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("curved polygons and multi curves");

    const std::string poly = write(
        "CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING (0 0, 5 5, 10 0), (10 0, 0 0)))");
    // Shell clockwise: the arc over the top runs clockwise already.
    ensure_equals(poly, std::string("{\"curveRings\":[[[0,0],{\"c\":[[10,0],[5,5]]},[0,0]]]}"));
    const std::string flipped = write(
        "CURVEPOLYGON (COMPOUNDCURVE ((0 0, 10 0), CIRCULARSTRING (10 0, 5 5, 0 0)))");
    ensure_equals(flipped, std::string("{\"curveRings\":[[[0,0],{\"c\":[[10,0],[5,5]]},[0,0]]]}"));

    const std::string multi = write("MULTICURVE (CIRCULARSTRING (0 0, 1 1, 2 0), (5 5, 6 6))");
    ensure_equals(multi, std::string("{\"curvePaths\":[[[0,0],{\"c\":[[2,0],[1,1]]}],[[5,5],[6,6]]]}"));
}

template<>
template<>
void object::test<5>()
{
    set_test_name("clothoid, spiral and NURBS: Beziers, statistics, tolerance");

    const std::string clothoid =
        "CLOTHOID (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), REFERENCEDIRECTIONS "
        "(VECTOR (1 0), VECTOR (0 1))), SCALEFACTOR 50, STARTDISTANCE 0, ENDDISTANCE 100)";
    writer_.setTolerance(1e-3);
    const std::string loose = write(clothoid);
    ensureBalanced(loose);
    ensure(loose.rfind("{\"curvePaths\":[[[0,0],{\"b\":[[", 0) == 0);
    const auto looseStats = writer_.getStats();
    ensure_equals(count(loose, "\"b\":"), looseStats.numSegments);
    ensure_equals(looseStats.numApproximated, looseStats.numSegments);
    ensure(looseStats.maxDeviation > 0 && looseStats.maxDeviation <= 1e-3);

    writer_.setTolerance(1e-5);
    const std::string tight = write(clothoid);
    ensure(writer_.getStats().numSegments > looseStats.numSegments);
    ensure(writer_.getStats().maxDeviation <= 1e-5);
    ensure(tight != loose);

    const std::string spiral = write(
        "SPIRALCURVE (REFERENCELOCATION AFFINEPLACEMENT (LOCATION (0 0), REFERENCEDIRECTIONS "
        "(VECTOR (1 0), VECTOR (0 1))), LENGTH 100, STARTCURVATURE 0, ENDCURVATURE 0.002, SPIRALTYPE bloss)");
    ensureBalanced(spiral);
    ensure(spiral.find("\"b\"") != std::string::npos);

    // NURBS quarter circle maps exactly to a circular arc.
    const std::string nurbs = write(
        "NURBSCURVE (DEGREE 2, CONTROLPOINTS (NURBSPOINT (WEIGHTEDPOINT (1 0), WEIGHT 1), "
        "NURBSPOINT (WEIGHTEDPOINT (0.7071067811865476 0.7071067811865476), WEIGHT 0.7071067811865476), "
        "NURBSPOINT (WEIGHTEDPOINT (0 1), WEIGHT 1)), KNOTS (KNOT (VALUE 0, MULTIPLICITY 3), KNOT (VALUE 1, MULTIPLICITY 3)))");
    ensure(nurbs.find("\"c\"") != std::string::npos);
    ensure_equals(writer_.getStats().numApproximated, 0u);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("SRID, errors");

    auto g = reader_.read("CIRCULARSTRING (0 0, 1 1, 2 0)");
    g->setSRID(3857);
    ensure_equals(writer_.write(*g),
                  std::string("{\"curvePaths\":[[[0,0],{\"c\":[[2,0],[1,1]]}]],\"spatialReference\":{\"wkid\":3857}}"));

    // No default tolerance.
    geos::io::EsriJsonWriter fresh;
    ensure_THROW(fresh.write(*g), geos::util::IllegalArgumentException);
    ensure_equals(fresh.getStats().numSegments, 0u);
    ensure_THROW(writer_.setTolerance(0.0), geos::util::IllegalArgumentException);
    ensure_THROW(writer_.setTolerance(-1.0), geos::util::IllegalArgumentException);
    ensure_THROW(writer_.setTolerance(std::numeric_limits<double>::infinity()),
                 geos::util::IllegalArgumentException);
    auto coll = reader_.read("GEOMETRYCOLLECTION (POINT (1 1), LINESTRING (0 0, 1 1))");
    ensure_THROW(writer_.write(*coll), geos::util::UnsupportedOperationException);
}

} // namespace tut
