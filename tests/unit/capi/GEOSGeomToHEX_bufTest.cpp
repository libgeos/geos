#include <tut/tut.hpp>
// geos
#include <geos_c.h>
#include <geos/util/Machine.h> // for getMachineByteOrder

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


    if(getMachineByteOrder() == GEOS_WKB_XDR) {
        ensure_equals(hexStr,
            std::string{"00000000013FF00000000000004000000000000000"});
    } else {
        ensure_equals(hexStr,
            std::string{"0101000000000000000000F03F0000000000000040"});
    }

    free(hex);
}

} // namespace tut

