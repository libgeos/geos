// Test Suite for C-API GEOSNodeCollection

#include <tut/tut.hpp>
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {

struct test_capigeosnode_collection_data : public capitest::utility {};

typedef test_group<test_capigeosnode_collection_data> group;
typedef group::object object;
group test_capigeosnode_collection_group("capi::GEOSNodeCollection");

// Basic: two crossing lines produce two noded MultiLineStrings
template<> template<> void object::test<1>()
{
    set_test_name("two crossing linestrings");
    input_ = fromWKT("GEOMETRYCOLLECTION (LINESTRING (0 0, 10 10), LINESTRING (0 10, 10 0))");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(result_ != nullptr);
    ensure_equals(GEOSGeomTypeId(result_), GEOS_GEOMETRYCOLLECTION);
    ensure_equals(GEOSGetNumGeometries(result_), 2);
    ensure_equals(GEOSGetNumGeometries(GEOSGetGeometryN(result_, 0)), 2);
    ensure_equals(GEOSGetNumGeometries(GEOSGetGeometryN(result_, 1)), 2);
}

// Issue 877: near-duplicate rings fail with IteratedNoder; snap-rounding fixes it
template<> template<> void object::test<2>()
{
    set_test_name("issue 877 near-duplicate rings with snap-rounding");
    input_ = fromWKT(
        "GEOMETRYCOLLECTION ("
        "LINESTRING (125635 6696, 131951 6376, 132163 474.0000000000043, 128381 1569.9999999999986, 125635 6696),"
        "LINESTRING (125635 6696, 131951 6376, 132163 474, 128381 1570, 125635 6696))");
    result_ = GEOSNodeCollection(input_, 1.0);
    ensure("snap-rounding succeeds", result_ != nullptr);
    ensure_equals("count preserved", GEOSGetNumGeometries(result_), 2);
}

// Non-collection input returns NULL
template<> template<> void object::test<3>()
{
    set_test_name("non-collection input returns NULL");
    input_ = fromWKT("LINESTRING (0 0, 1 1)");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure("non-collection returns NULL", result_ == nullptr);
}

// Non-intersecting: count preserved, each member stays as single-segment MultiLineString
template<> template<> void object::test<4>()
{
    set_test_name("non-intersecting: count preserved");
    input_ = fromWKT("GEOMETRYCOLLECTION (LINESTRING (0 0, 5 5), LINESTRING (10 0, 15 5))");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(result_ != nullptr);
    ensure_equals(GEOSGetNumGeometries(result_), 2);
    ensure_equals(GEOSGetNumGeometries(GEOSGetGeometryN(result_, 0)), 1);
    ensure_equals(GEOSGetNumGeometries(GEOSGetGeometryN(result_, 1)), 1);
}

// SRID is preserved on the output collection
template<> template<> void object::test<5>()
{
    set_test_name("SRID preservation");
    input_ = fromWKT("GEOMETRYCOLLECTION (LINESTRING (0 0, 10 10), LINESTRING (0 10, 10 0))");
    GEOSSetSRID(input_, 4326);
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(result_ != nullptr);
    ensure_equals("SRID preserved", GEOSGetSRID(result_), GEOSGetSRID(input_));
}

// Z interpolation: new node points get Z averaged from both intersecting lines
template<> template<> void object::test<6>()
{
    set_test_name("Z averaged at new node points from both intersecting lines");
    // Line 0: Z 0->1 at t=0.5 -> 0.5; Line 1: Z 5->10 at t=0.5 -> 7.5; avg=4.0
    input_ = fromWKT(
        "GEOMETRYCOLLECTION ("
        "LINESTRING Z (0 0 0, 1 1 1),"
        "LINESTRING Z (0 1 5, 1 0 10))");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(result_ != nullptr);
    expected_ = fromWKT(
        "GEOMETRYCOLLECTION ("
        "MULTILINESTRING Z ((0 0 0, 0.5 0.5 4), (0.5 0.5 4, 1 1 1)),"
        "MULTILINESTRING Z ((0 1 5, 0.5 0.5 4), (0.5 0.5 4, 1 0 10)))");
    GEOSNormalize(result_);
    GEOSNormalize(expected_);
    ensure_geometry_equals(result_, expected_);
}

// M interpolation: new node points get M averaged from both intersecting lines
template<> template<> void object::test<7>()
{
    set_test_name("M averaged at new node points from both intersecting lines");
    // Line 0: M 0->1 at t=0.5 -> 0.5; Line 1: M 5->10 at t=0.5 -> 7.5; avg=4.0
    input_ = fromWKT(
        "GEOMETRYCOLLECTION ("
        "LINESTRING M (0 0 0, 1 1 1),"
        "LINESTRING M (0 1 5, 1 0 10))");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(result_ != nullptr);
    expected_ = fromWKT(
        "GEOMETRYCOLLECTION ("
        "MULTILINESTRING M ((0 0 0, 0.5 0.5 4), (0.5 0.5 4, 1 1 1)),"
        "MULTILINESTRING M ((0 1 5, 0.5 0.5 4), (0.5 0.5 4, 1 0 10)))");
    GEOSNormalize(result_);
    GEOSNormalize(expected_);
    ensure_geometry_equals(result_, expected_);
}

// gridSize < 0: treated as 0, does not crash
template<> template<> void object::test<8>()
{
    set_test_name("gridSize < 0 treated as standard noding");
    input_ = fromWKT("GEOMETRYCOLLECTION (LINESTRING (0 0, 10 10), LINESTRING (0 10, 10 0))");
    result_ = GEOSNodeCollection(input_, -1.0);
    ensure("negative gridSize does not crash", result_ != nullptr);
    ensure_equals("count preserved", GEOSGetNumGeometries(result_), 2);
}

// Empty GEOMETRYCOLLECTION: returns empty collection, not NULL
template<> template<> void object::test<9>()
{
    set_test_name("empty collection");
    input_ = fromWKT("GEOMETRYCOLLECTION EMPTY");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure("empty collection returns non-NULL", result_ != nullptr);
    ensure_equals("result has 0 members", GEOSGetNumGeometries(result_), 0);
}

// Identical members: both preserved, not dissolved
template<> template<> void object::test<10>()
{
    set_test_name("identical members each preserved separately");
    input_ = fromWKT("GEOMETRYCOLLECTION (LINESTRING (0 0, 10 0), LINESTRING (0 0, 10 0))");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(result_ != nullptr);
    ensure_equals("count preserved", GEOSGetNumGeometries(result_), 2);
    ensure("member 0 non-empty", !GEOSisEmpty(GEOSGetGeometryN(result_, 0)));
    ensure("member 1 non-empty", !GEOSisEmpty(GEOSGetGeometryN(result_, 1)));
}

// Non-linear (POLYGON) member produces empty MultiLineString in its output slot
template<> template<> void object::test<11>()
{
    set_test_name("polygon member yields empty output slot");
    input_ = fromWKT(
        "GEOMETRYCOLLECTION ("
        "POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0)),"
        "LINESTRING (5 -5, 5 15))");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(result_ != nullptr);
    ensure_equals("count preserved", GEOSGetNumGeometries(result_), 2);
    ensure("polygon slot is empty", GEOSisEmpty(GEOSGetGeometryN(result_, 0)));
    ensure("line slot is non-empty", !GEOSisEmpty(GEOSGetGeometryN(result_, 1)));
}

// Nested GEOMETRYCOLLECTION member: all linear components extracted
template<> template<> void object::test<12>()
{
    set_test_name("nested GEOMETRYCOLLECTION member");
    input_ = fromWKT(
        "GEOMETRYCOLLECTION ("
        "GEOMETRYCOLLECTION (LINESTRING (0 0, 10 0), LINESTRING (0 5, 10 5)),"
        "LINESTRING (5 -1, 5 10))");
    result_ = GEOSNodeCollection(input_, 0.0);
    ensure(result_ != nullptr);
    ensure_equals("count preserved", GEOSGetNumGeometries(result_), 2);
    ensure_equals("nested member has 4 segments",
        GEOSGetNumGeometries(GEOSGetGeometryN(result_, 0)), 4);
}

} // namespace tut
