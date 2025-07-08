//
// Test Suite for geos::operation::overlay::snap::GeometrySnapper class.

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/operation/overlay/snap/GeometrySnapper.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>
#include <string>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_geometrysnapper_data {

    geos::io::WKTReader reader;

    test_geometrysnapper_data() {};

};

using geos::operation::overlay::snap::GeometrySnapper;

typedef test_group<test_geometrysnapper_data> group;
typedef group::object object;

group test_geometrysnapper_group("geos::operation::overlay::snap::GeometrySnapper");

//
// Test Cases
//

// Test vertices snapping
template<>
template<>
void object::test<1> ()
{
    auto src = reader.read("POLYGON ((0 0, 0 100, 100 100, 100 0, 0 0))");
    GeometrySnapper snapper(*src);
    auto snap = reader.read("MULTIPOINT ((0 0), (0 100.0000001), (100 100), (100 0))");
    auto expected = reader.read("POLYGON ((0 0, 0 100.0000001, 100 100, 100 0, 0 0))");
    auto result = snapper.snapTo(*snap, 0.000001);
    ensure_equals_geometry(expected.get(), result.get());
}

// Test vertices snapping
template<>
template<>
void object::test<2> ()
{
    auto src = reader.read("POLYGON ((0 0, 0 100, 100 100, 100 0, 0 0))");
    GeometrySnapper snapper(*src);
    auto snap = reader.read("MULTIPOINT ((0.0000001 50))");
    auto expected = reader.read("POLYGON ((0 0, 0.0000001 50, 0 100, 100 100, 100 0, 0 0))");
    auto result = snapper.snapTo(*snap, 0.000001);
    ensure_equals_geometry(expected.get(), result.get());
}


// Test self snapping
template<>
template<>
void object::test<3> ()
{
    auto src = reader.read("POLYGON ((0 0, 0 100, 0 100, 100 100, 100 0, 0 0))");
    auto result = GeometrySnapper::snapToSelf(*src, 0.1, true);
    auto expected = reader.read("POLYGON ((0 0, 0 100, 100 100, 100 0, 0 0))");
    ensure_equals_geometry(expected.get(), result.get());
}


} // namespace tut
