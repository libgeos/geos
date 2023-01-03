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

template<>
template<>
void object::test<2>()
{
    GEOSContextHandle_t handle = GEOS_init_r();

    ensure_equals(GEOS_getWKBOutputDims_r(handle), 2);
    ensure_equals(GEOS_getWKBByteOrder_r(handle), getMachineByteOrder());

    GEOS_finish_r(handle);
}

template<>
template<>
void object::test<3>()
{
    GEOSContextHandle_t handle = GEOS_init_r();

    ensure_equals(GEOS_setWKBOutputDims_r(handle, 4), 2);
    ensure_equals(GEOS_setWKBByteOrder_r(handle, GEOS_WKB_XDR), getMachineByteOrder());

    ensure_equals(GEOS_getWKBOutputDims_r(handle), 4);
    ensure_equals(GEOS_getWKBByteOrder_r(handle), GEOS_WKB_XDR);

    geom1_ = fromWKT("POINT ZM (3 8 2 6)");

    std::size_t size;
    unsigned char* hex = GEOSGeomToHEX_buf_r(handle, geom1_, &size);
    std::string hexStr(reinterpret_cast<char*>(hex));

    // SELECT encode(ST_AsEWKB('POINT ZM (3 8 2 6)'::geometry, 'XDR'), 'hex')
    ensure_equals(hexStr, "00C00000014008000000000000402000000000000040000000000000004018000000000000");
    GEOSFree(hex);

    GEOS_finish_r(handle);
}

} // namespace tut

