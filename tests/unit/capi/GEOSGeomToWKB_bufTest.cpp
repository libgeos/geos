#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeomtowkb_buf_data : public capitest::utility {};

typedef test_group<test_geosgeomtowkb_buf_data> group;
typedef group::object object;

group test_geosgeomtowkb_buf("capi::GEOSGeomToWKB_buf");

template<>
template<>
void object::test<1>()
{
    
    geom1_ = fromWKT("POINT (1 2)");
    ensure(nullptr != geom1_);

    std::size_t size{};
    unsigned char* wkb = GEOSGeomToWKB_buf(geom1_, &size);
    geom2_ = GEOSGeomFromWKB_buf(&wkb[0], size);
    ensure(nullptr != geom2_);
    
    ensure_equals(toWKT(geom1_), toWKT(geom2_));

    free(wkb);
}

} // namespace tut

