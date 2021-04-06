#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosgeomtohex_buf_data : public capitest::utility {};

typedef test_group<test_geosgeomtohex_buf_data> group;
typedef group::object object;

group test_geosgeomtohex_buf("capi::GEOSGeomToHEX_buf");

template<>
template<>
void object::test<1>()
{
    
    geom1_ = fromWKT("POINT (1 2)");
    ensure(nullptr != geom1_);

    std::size_t size{};
    unsigned char* hex = GEOSGeomToHEX_buf(geom1_, &size);
    std::string hexStr(reinterpret_cast<char*>(hex));
    ensure_equals(std::string{"0101000000000000000000F03F0000000000000040"}, hexStr);

    free(hex);
}

} // namespace tut

