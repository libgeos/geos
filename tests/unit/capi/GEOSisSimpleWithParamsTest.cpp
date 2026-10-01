#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosissimplewithparams_data : public capitest::utility {
    GEOSisSimpleParams* _params;
    test_geosissimplewithparams_data()
    {
        _params = GEOSisSimpleParams_create();
    }
    ~test_geosissimplewithparams_data()
    {
        GEOSisSimpleParams_destroy( _params );
    }
};

typedef test_group<test_geosissimplewithparams_data> group;
typedef group::object object;

group test_geosissimplewithparams("capi::GEOSisSimpleWithParams");

template<>
template<>
void object::test<1>()
{
    set_test_name("MultiLineString containing two rings that touch at their endpoints, default params");

    input_ = GEOSGeomFromWKT("MULTILINESTRING( (0 0, -10 10,-10 -10, 0 0), (0 0,20 -10,20 10,0 0) )");
    ensure(nullptr != input_);
    ensure_equals((int)GEOSisSimpleWithParams(input_, _params, &result_), 0);
    ensure_geometry_equals(result_, "POINT (0 0)");
}

template<>
template<>
void object::test<2>()
{
    set_test_name("MultiLineString containing two rings that touch at their endpoints, EndPoint boundary node rule");

    input_ = GEOSGeomFromWKT("MULTILINESTRING( (0 0, -10 10,-10 -10, 0 0), (0 0,20 -10,20 10,0 0) )");
    ensure(nullptr != input_);
    GEOSisSimpleParams_setBoundaryNodeRule(_params, GEOSRELATE_BNR_ENDPOINT);
    ensure_equals((int)GEOSisSimpleWithParams(input_, _params, &result_), 1);
    ensure(result_ == nullptr);
}

template<>
template<>
void object::test<3>()
{
    set_test_name("MultiLineString containing two rings that touch at their endpoints, MultiValent boundary node rule");

    input_ = GEOSGeomFromWKT("MULTILINESTRING( (0 0, -10 10,-10 -10, 0 0), (0 0,20 -10,20 10,0 0) )");
    ensure(nullptr != input_);
    GEOSisSimpleParams_setBoundaryNodeRule(_params, GEOSRELATE_BNR_MULTIVALENT_ENDPOINT);
    ensure_equals((int)GEOSisSimpleWithParams(input_, _params, &result_), 1);
    ensure(result_ == nullptr);
}

template<>
template<>
void object::test<4>()
{
    set_test_name("MultiLineString containing two rings that touch at their endpoints, MonoValent boundary node rule");

    input_ = GEOSGeomFromWKT("MULTILINESTRING( (0 0, -10 10,-10 -10, 0 0), (0 0,20 -10,20 10,0 0) )");
    ensure(nullptr != input_);
    GEOSisSimpleParams_setBoundaryNodeRule(_params, GEOSRELATE_BNR_MONOVALENT_ENDPOINT);
    ensure_equals((int)GEOSisSimpleWithParams(input_, _params, &result_), 0);
    ensure_geometry_equals(result_, "POINT (0 0)");
}

template<>
template<>
void object::test<5>()
{
    set_test_name("MultiLineString with a line intersected by two other lines, finding all locations");

    input_ = GEOSGeomFromWKT("MULTILINESTRING( (0 0,10 0),(2 -5,2 5),(4 -5, 4 5) )");
    ensure(nullptr != input_);
    GEOSisSimpleParams_setFindAllLocations(_params, 1);
    ensure_equals((int)GEOSisSimpleWithParams(input_, _params, &result_), 0);
    ensure_equals(GEOSGetNumGeometries(result_), 2);
}

template<>
template<>
void object::test<6>()
{
    set_test_name("Non-simple line with a SRID");

    input_ = GEOSGeomFromWKT("LINESTRING(0 0,10 0,2 -5,2 5,6 -8)");
    GEOSSetSRID(input_, 3857);
    ensure(nullptr != input_);
    ensure_equals((int)GEOSisSimpleWithParams(input_, _params, &result_), 0);
    ensure_equals(GEOSGetNumGeometries(result_), 1);
    ensure_equals(GEOSGetSRID(result_), 3857);
}

template<>
template<>
void object::test<7>()
{
    set_test_name("Non-simple line with a SRID and finding all locations");

    input_ = GEOSGeomFromWKT("LINESTRING(0 0,10 0,2 -5,2 5,6 -8)");
    GEOSSetSRID(input_, 4326);
    ensure(nullptr != input_);
    GEOSisSimpleParams_setFindAllLocations(_params, 1);
    ensure_equals((int)GEOSisSimpleWithParams(input_, _params, &result_), 0);
    ensure_equals(GEOSGetNumGeometries(result_), 3);
    ensure_equals(GEOSGetSRID(result_), 4326);
}
} // namespace tut
