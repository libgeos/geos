//
// Test Suite for geos::operation::relateng::RelateGeometry class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Location.h>
#include <geos/operation/relateng/RelateGeometry.h>
#include <geos/operation/relateng/DimensionLocation.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::relateng;
using geos::io::WKTReader;
// using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_relategeometry_data {

    WKTReader r;
    // WKTWriter w;

    void checkDimension(const std::string& wkt, int expectedDim, int expectedDimReal)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        RelateGeometry rgeom(geom.get());
        ensure_equals("checkNodeLocation 1", rgeom.getDimension(), expectedDim);
        ensure_equals("checkNodeLocation 2", rgeom.getDimensionReal(), expectedDimReal);
    }

};

typedef test_group<test_relategeometry_data> group;
typedef group::object object;

group test_relategeometry_group("geos::operation::relateng::RelateGeometry");


// testUniquePoints
template<>
template<>
void object::test<1> ()
{
    std::unique_ptr<Geometry> geom = r.read("MULTIPOINT ((0 0), (5 5), (5 0), (0 0))");
    RelateGeometry rgeom(geom.get());
    auto pts = rgeom.getUniquePoints();
    ensure_equals("Unique pts size", pts.size(), 3u);
}

// testBoundary
template<>
template<>
void object::test<2> ()
{
    std::unique_ptr<Geometry> geom = r.read("MULTILINESTRING ((0 0, 9 9), (9 9, 5 1))");
    RelateGeometry rgeom(geom.get());
    ensure("hasBoundary", rgeom.hasBoundary());
}

// testHasDimension
template<>
template<>
void object::test<3> ()
{
    std::unique_ptr<Geometry> geom = r.read("GEOMETRYCOLLECTION (POLYGON ((1 9, 5 9, 5 5, 1 5, 1 9)), LINESTRING (1 1, 5 4), POINT (6 5))");
    RelateGeometry rgeom(geom.get());
    ensure("hasDimension 0", rgeom.hasDimension(0));
    ensure("hasDimension 1", rgeom.hasDimension(1));
    ensure("hasDimension 2", rgeom.hasDimension(2));
}

// testDimension
template<>
template<>
void object::test<4> ()
{
    checkDimension("POINT (0 0)", 0, 0);
}

template<>
template<>
void object::test<5> ()
{
    checkDimension("LINESTRING (0 0, 0 0)", 1, 0);
}

template<>
template<>
void object::test<6> ()
{
    checkDimension("LINESTRING (0 0, 9 9)", 1, 1);
}

template<>
template<>
void object::test<7> ()
{
    checkDimension("POLYGON ((1 9, 5 9, 5 5, 1 5, 1 9))", 2, 2);
}

template<>
template<>
void object::test<8> ()
{
    checkDimension("GEOMETRYCOLLECTION (POLYGON ((1 9, 5 9, 5 5, 1 5, 1 9)), LINESTRING (1 1, 5 4), POINT (6 5))", 2, 2);
}

template<>
template<>
void object::test<9> ()
{
    checkDimension("GEOMETRYCOLLECTION (POLYGON EMPTY, LINESTRING (1 1, 5 4), POINT (6 5))", 2, 1);
}

template<>
template<>
void object::test<10> ()
{
    checkDimension("LINESTRING (0 0, 0 0, 9 9)", 1, 1);
}






} // namespace tut
