//
// Test Suite for geos::geom::util::GeometryCombiner class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/geom/util/GeometryCombiner.h>

#include <utility.h>
// std
#include <vector>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_geometrycombiner_data {
    using GeometryCombiner = geos::geom::util::GeometryCombiner;

    geos::io::WKTReader wktreader_;
};

typedef test_group<test_geometrycombiner_data> group;
typedef group::object object;

group test_geometrycombiner_group("geos::geom::util::GeometryCombiner");

// Two-argument version
template<>
template<>
void object::test<1>(){
    auto g1 = wktreader_.read("POINT (1 1)");
    auto g2 = wktreader_.read("POINT (2 2)");

    auto result_via_copy = GeometryCombiner::combine(g1.get(), g2.get());
    ensure_equals_geometry(result_via_copy.get(), "MULTIPOINT ((1 1), (2 2))");

    auto result_via_move = GeometryCombiner::combine(std::move(g1), std::move(g2));
    ensure_equals_geometry(result_via_move.get(), result_via_copy.get());
}

// Three-argument version
template<>
template<>
void object::test<2>(){
    auto g1 = wktreader_.read("POINT (1 1)");
    auto g2 = wktreader_.read("POINT (2 2)");
    auto g3 = wktreader_.read("LINESTRING (3 3, 4 4)");

    auto result_via_copy = GeometryCombiner::combine(g1.get(), g2.get(), g3.get());
    ensure_equals_geometry(result_via_copy.get(), "GEOMETRYCOLLECTION (POINT (1 1), POINT (2 2), LINESTRING (3 3, 4 4))");

    auto result_via_move = GeometryCombiner::combine(std::move(g1), std::move(g2), std::move(g3));
    ensure_equals_geometry(result_via_move.get(), result_via_copy.get());
}

// Vector version
template<>
template<>
void object::test<3>() {
    std::vector<std::unique_ptr<geos::geom::Geometry>> geoms;
    geoms.push_back(wktreader_.read("POINT (1 1)"));
    geoms.push_back(wktreader_.read("POLYGON EMPTY"));
    geoms.push_back(wktreader_.read("POINT (2 2)"));

    GeometryCombiner gc(std::move(geoms));
    gc.setSkipEmpty(true);

    auto result = gc.combine();

    ensure_equals_geometry(result.get(), "MULTIPOINT ((1 1), (2 2))");
}

// Outermost level of GeometryCollection is collapsed
template<>
template<>
void object::test<4>() {
    auto g1 = wktreader_.read("MULTIPOINT ((1 1), (2 2))");
    auto g2 = wktreader_.read("MULTILINESTRING ((3 3, 4 4), (5 5, 6 6))");

    auto result_via_copy = GeometryCombiner::combine(g1->clone(), g2->clone());
    ensure_equals_geometry(result_via_copy.get(), "GEOMETRYCOLLECTION("
                                                  "POINT (1 1),"
                                                  "POINT (2 2),"
                                                  "LINESTRING (3 3, 4 4),"
                                                  "LINESTRING (5 5, 6 6))");

    auto result_via_move = GeometryCombiner::combine(std::move(g1), std::move(g2));
    ensure_equals_geometry(result_via_move.get(), result_via_copy.get());
}

// Test behavior when only empty inputs provided
template<>
template<>
void object::test<5>()
{
    std::vector<std::unique_ptr<geos::geom::Geometry>> geoms;
    geoms.emplace_back(wktreader_.read("POINT EMPTY"));

    GeometryCombiner gc(std::move(geoms));
    gc.setSkipEmpty(true);

    auto result = gc.combine();

    ensure_equals_geometry(result.get(), "GEOMETRYCOLLECTION EMPTY");
}

// Test behavior when no inputs provided
template<>
template<>
void object::test<6>()
{
    std::vector<const geos::geom::Geometry*> geoms;
    GeometryCombiner gc(geoms);

    auto result = gc.combine();

    ensure_equals_geometry(result.get(), "GEOMETRYCOLLECTION EMPTY");
}


} // namespace tut
