// $Id$
//
// Test Suite for C-API GEOSGetCentroid

#include <tut/tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

// Common data used in test cases.
struct test_capigeometrylist_data : public capitest::utility {
    test_capigeometrylist_data()
    {
        GEOSWKTWriter_setTrim(wktw_, 1);
        GEOSWKTWriter_setRoundingPrecision(wktw_, 8);
    }
};

typedef test_group<test_capigeometrylist_data> group;
typedef group::object object;

group test_capigeometrylist_data("capi::GEOSGeometryList");

//
// Test Cases
//

// Create and destroy
template<>
template<>
void object::test<1>
()
{
    GEOSGeometryList* glist = GEOSGeometryList_create();
    ensure(nullptr != glist);
    ensure(GEOSGeometryList_size(glist) == 0);
    GEOSGeometryList_destroy(glist);
}

template<>
template<>
void object::test<2>
()
{
    GEOSGeometryList* glist = GEOSGeometryList_create();
    ensure(nullptr != glist);
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 10, 0 20)");
    ensure(nullptr != geom1_);
    ensure(GEOSGeometryList_push(glist, geom1_) == 1);
    ensure(GEOSGeometryList_size(glist) == 1);
    ensure(GEOSGeometryList_release(glist) == 1);
}

template<>
template<>
void object::test<3>
()
{
    GEOSGeometryList* glist = GEOSGeometryList_create();
    ensure(nullptr != glist);
    geom1_ = GEOSGeomFromWKT("LINESTRING(0 10, 0 20)");
    geom2_ = GEOSGeomFromWKT("POINT(5 5)");
    ensure(nullptr != geom1_ && nullptr != geom2_);
    ensure(GEOSGeometryList_push(glist, geom1_) == 1);
    ensure(GEOSGeometryList_push(glist, geom2_) == 1);
    ensure(GEOSGeometryList_size(glist) == 2);

    GEOSGeometry* geom_ptr = GEOSGeometryList_pop(glist);
    ensure(geom2_ == geom_ptr);

    geom_ptr = GEOSGeometryList_at(glist, 0);
    ensure(geom1_ == geom_ptr);

    ensure(GEOSGeometryList_release(glist) == 1);
}



} // namespace tut

