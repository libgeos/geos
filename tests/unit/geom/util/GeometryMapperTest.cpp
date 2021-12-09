//
// Test Suite for geos::geom::util::GeometryCombiner class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/geom/util/GeometryMapper.h>

#include <utility.h>
// std
#include <vector>

namespace tut {

using namespace geos::geom;
using geos::geom::util::GeometryMapper;


// Common data used by tests
struct test_geometrymapper_data {

    geos::io::WKTReader wktreader_;

     GeometryMapper::mapOp KEEP_LINE = [](const Geometry& geom)->std::unique_ptr<Geometry> {
        if (geom.getGeometryTypeId() == GEOS_POINT) {
            return geom.getFactory()->createEmpty(1);
        }
        if (geom.getGeometryTypeId() == GEOS_LINESTRING) {
            return geom.clone();
        }
        return nullptr;
    };

    GeometryMapper::mapOp BOUNDARY = [](const Geometry& geom)->std::unique_ptr<Geometry> {
        return geom.getBoundary();
    };

    void checkFlatMap(const std::string& wkt, int dim, GeometryMapper::mapOp op, const std::string& wktExpected)
    {
        auto geom = wktreader_.read(wkt);
        auto actual = GeometryMapper::flatMap(*geom, dim, op);
        auto expected = wktreader_.read(wktExpected);
        ensure_equals_geometry(actual.get(), expected.get());
    }

};

typedef test_group<test_geometrymapper_data> group;
typedef group::object object;

group test_geometrymapper_group("geos::geom::util::GeometryMapper");


// testFlatMapInputEmpty
template<>
template<>
void object::test<1>()
{
    checkFlatMap(
        "GEOMETRYCOLLECTION(POINT EMPTY, LINESTRING EMPTY)",
        1, KEEP_LINE, "LINESTRING EMPTY");
}

// testFlatMapInputMulti
template<>
template<>
void object::test<2>()
{
    checkFlatMap(
        "GEOMETRYCOLLECTION( MULTILINESTRING((0 0, 1 1), (1 1, 2 2)), LINESTRING(2 2, 3 3))",
        1, KEEP_LINE, "MULTILINESTRING ((0 0, 1 1), (1 1, 2 2), (2 2, 3 3))");
}

// testFlatMapResultEmpty
template<>
template<>
void object::test<3>()
{
    checkFlatMap(
        "GEOMETRYCOLLECTION( LINESTRING(0 0, 1 1), LINESTRING(1 1, 2 2))",
        1, KEEP_LINE, "MULTILINESTRING((0 0, 1 1), (1 1, 2 2))");

    checkFlatMap(
        "GEOMETRYCOLLECTION( POINT(0 0), POINT(0 0), LINESTRING(0 0, 1 1))",
        1, KEEP_LINE, "LINESTRING(0 0, 1 1)");

    checkFlatMap(
        "MULTIPOINT((0 0), (1 1))",
        1, KEEP_LINE, "LINESTRING EMPTY");
}

// testFlatMapResultNull
template<>
template<>
void object::test<4>()
{
    checkFlatMap(
        "GEOMETRYCOLLECTION( POINT(0 0), LINESTRING(0 0, 1 1), POLYGON ((1 1, 1 2, 2 1, 1 1)))",
        1, KEEP_LINE, "LINESTRING(0 0, 1 1)");
}

// testFlatMapBoundary
template<>
template<>
void object::test<5>()
{
    checkFlatMap(
        "GEOMETRYCOLLECTION( POINT(0 0), LINESTRING(0 0, 1 1), POLYGON ((1 1, 1 2, 2 1, 1 1)))",
        0, BOUNDARY, "GEOMETRYCOLLECTION (POINT (0 0), POINT (1 1), LINESTRING (1 1, 1 2, 2 1, 1 1))");

    checkFlatMap(
        "LINESTRING EMPTY",
        0, BOUNDARY, "POINT EMPTY");
}


} // namespace tut
