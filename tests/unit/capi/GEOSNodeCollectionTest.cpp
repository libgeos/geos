//
// Test Suite for C-API GEOSNodeCollection

#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

#include <string>

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeosnodecollection_data : public capitest::utility {

    // Clone member i of coll, normalize it, and compare (exactly) to the
    // normalized expected WKT. Verifies both the per-slot content and the
    // 1:1 positional mapping to the input.
    void check_member(GEOSGeometry* coll, int i, const char* expectedWKT)
    {
        const GEOSGeometry* m = GEOSGetGeometryN(coll, i);
        ensure(std::string("member exists ") + std::to_string(i), m != nullptr);

        GEOSGeometry* got = GEOSGeom_clone(m);
        GEOSGeometry* exp = GEOSGeomFromWKT(expectedWKT);
        GEOSNormalize(got);
        GEOSNormalize(exp);
        ensure_equals(std::string("member ") + std::to_string(i),
            static_cast<int>(GEOSEqualsExact(got, exp, 0.0)), 1);
        GEOSGeom_destroy(got);
        GEOSGeom_destroy(exp);
    }
};

typedef test_group<test_capigeosnodecollection_data> group;
typedef group::object object;

group test_capigeosnodecollection_group("capi::GEOSNodeCollection");

//
// Test Cases
//

// Crossing lines keep their per-member identity (unlike GEOSNode, which
// would flatten both into one MultiLineString).
template<>
template<>
void object::test<1>
()
{
    input_ = GEOSGeomFromWKT(
        "GEOMETRYCOLLECTION ("
        "LINESTRING (0 0, 10 10),"
        "LINESTRING (0 10, 10 0))");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(nullptr != result_);

    ensure_equals("1:1 mapping", GEOSGetNumGeometries(result_), 2);
    check_member(result_, 0, "MULTILINESTRING ((0 0, 5 5), (5 5, 10 10))");
    check_member(result_, 1, "MULTILINESTRING ((0 10, 5 5), (5 5, 10 0))");

    // Result container is a GeometryCollection of MultiLineStrings.
    ensure_equals("top-level type", GEOSGeomTypeId(result_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals("slot 0 type", GEOSGeomTypeId(GEOSGetGeometryN(result_, 0)), GEOS_MULTILINESTRING);
    ensure_equals("slot 1 type", GEOSGeomTypeId(GEOSGetGeometryN(result_, 1)), GEOS_MULTILINESTRING);
}

// Mixed input types: polygon boundary is noded as linework, point yields
// an empty slot.
template<>
template<>
void object::test<2>
()
{
    input_ = GEOSGeomFromWKT(
        "GEOMETRYCOLLECTION ("
        "LINESTRING (-5 5, 15 5),"
        "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0)),"
        "POINT (3 3))");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(nullptr != result_);

    ensure_equals("1:1 mapping", GEOSGetNumGeometries(result_), 3);
    check_member(result_, 0, "MULTILINESTRING ((-5 5, 0 5), (0 5, 10 5), (10 5, 15 5))");
    check_member(result_, 1,
        "MULTILINESTRING ((0 0, 10 0, 10 5), (10 5, 10 10, 0 10, 0 5), (0 5, 0 0))");
    check_member(result_, 2, "MULTILINESTRING EMPTY");
}

// gridSize > 0 snaps near-coincident coordinates onto a node.
template<>
template<>
void object::test<3>
()
{
    input_ = GEOSGeomFromWKT(
        "GEOMETRYCOLLECTION ("
        "LINESTRING (0 0, 10 0),"
        "LINESTRING (5 0.4, 5 10))");
    result_ = GEOSNodeCollection(input_, 1.0);
    ensure(nullptr != result_);

    ensure_equals("1:1 mapping", GEOSGetNumGeometries(result_), 2);
    check_member(result_, 0, "MULTILINESTRING ((0 0, 5 0), (5 0, 10 0))");
    check_member(result_, 1, "MULTILINESTRING ((5 0, 5 10))");
}

// A single (non-collection) input is treated as a one-member collection.
template<>
template<>
void object::test<4>
()
{
    input_ = GEOSGeomFromWKT("LINESTRING (0 0, 10 10, 10 0, 0 10)");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(nullptr != result_);

    ensure_equals("single member", GEOSGetNumGeometries(result_), 1);
    check_member(result_, 0,
        "MULTILINESTRING ((0 0, 5 5), (5 5, 10 10, 10 0, 5 5), (5 5, 0 10))");
}

// A non-collection POINT is treated as a one-member collection; its slot
// is an empty MultiLineString.
template<>
template<>
void object::test<5>
()
{
    input_ = GEOSGeomFromWKT("POINT (3 3)");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(nullptr != result_);

    ensure_equals("top-level type", GEOSGeomTypeId(result_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals("single member", GEOSGetNumGeometries(result_), 1);
    check_member(result_, 0, "MULTILINESTRING EMPTY");
}

// A non-collection POLYGON is treated as a one-member collection; its
// boundary is returned as noded linework.
template<>
template<>
void object::test<6>
()
{
    input_ = GEOSGeomFromWKT("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(nullptr != result_);

    ensure_equals("top-level type", GEOSGeomTypeId(result_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals("single member", GEOSGetNumGeometries(result_), 1);
    ensure_equals("slot type", GEOSGeomTypeId(GEOSGetGeometryN(result_, 0)), GEOS_MULTILINESTRING);
    // An uncrossed ring comes back as its single closed boundary line.
    check_member(result_, 0, "MULTILINESTRING ((0 0, 10 0, 10 10, 0 10, 0 0))");
}

// An empty atomic input still yields a one-member collection.
template<>
template<>
void object::test<7>
()
{
    input_ = GEOSGeomFromWKT("POINT EMPTY");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(nullptr != result_);

    ensure_equals("top-level type", GEOSGeomTypeId(result_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals("single member", GEOSGetNumGeometries(result_), 1);
    check_member(result_, 0, "MULTILINESTRING EMPTY");
}

// Empty members mixed with non-empty members keep their slots.
template<>
template<>
void object::test<8>
()
{
    input_ = GEOSGeomFromWKT(
        "GEOMETRYCOLLECTION ("
        "LINESTRING (0 0, 10 0),"
        "LINESTRING EMPTY)");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(nullptr != result_);

    ensure_equals("1:1 mapping", GEOSGetNumGeometries(result_), 2);
    check_member(result_, 0, "MULTILINESTRING ((0 0, 10 0))");
    check_member(result_, 1, "MULTILINESTRING EMPTY");
}

// Curved + linear with gridSize > 0: gridSize is ignored for curved input,
// and the curved member's slot is returned as a MultiCurve.
template<>
template<>
void object::test<9>
()
{
    input_ = GEOSGeomFromWKT(
        "GEOMETRYCOLLECTION ("
        "LINESTRING (0 -1, 0 2),"
        "CIRCULARSTRING (-1 0, 0 1, 1 0))");
    result_ = GEOSNodeCollection(input_, 1.0);   // gridSize > 0
    ensure(nullptr != result_);

    ensure_equals("top-level type", GEOSGeomTypeId(result_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals("1:1 mapping", GEOSGetNumGeometries(result_), 2);

    const GEOSGeometry* lineSlot = GEOSGetGeometryN(result_, 0);
    ensure_equals("line slot type", GEOSGeomTypeId(lineSlot), GEOS_MULTILINESTRING);
    ensure_equals("line slot parts", GEOSGetNumGeometries(lineSlot), 2);

    const GEOSGeometry* arcSlot = GEOSGetGeometryN(result_, 1);
    ensure_equals("arc slot type", GEOSGeomTypeId(arcSlot), GEOS_MULTICURVE);
    ensure_equals("arc slot parts", GEOSGetNumGeometries(arcSlot), 2);
    for (int i = 0; i < GEOSGetNumGeometries(arcSlot); i++) {
        ensure_equals("arc piece type",
            GEOSGeomTypeId(GEOSGetGeometryN(arcSlot, i)), GEOS_CIRCULARSTRING);
    }
}

} // namespace tut
