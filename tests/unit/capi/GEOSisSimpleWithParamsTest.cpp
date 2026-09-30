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
    input_ = GEOSGeomFromWKT("MULTILINESTRING( (0 0, -10 10,-10 -10, 0 0), (0 0,20 -10,20 10,0 0) )");
    ensure(nullptr != input_);
    ensure_equals(0, GEOSisSimpleWithParams(input_, _params, &result_));
    ensure_geometry_equals(result_, "POINT (0 0)");
}

template<>
template<>
void object::test<2>()
{
    input_ = GEOSGeomFromWKT("MULTILINESTRING( (0 0, -10 10,-10 -10, 0 0), (0 0,20 -10,20 10,0 0) )");
    ensure(nullptr != input_);
    GEOSisSimpleParams_setBoundaryNodeRule(_params, GEOSRELATE_BNR_ENDPOINT);
    ensure_equals(1, GEOSisSimpleWithParams(input_, _params, &result_));
    ensure(result_ == nullptr);
}

} // namespace tut
