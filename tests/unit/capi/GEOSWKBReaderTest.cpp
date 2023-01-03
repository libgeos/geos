#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoswkbreader_data : public capitest::utility {

    test_geoswkbreader_data() :
        wkbreader_(GEOSWKBReader_create())
    {}

    ~test_geoswkbreader_data() {
        GEOSWKBReader_destroy(wkbreader_);
    }

    GEOSWKBReader* wkbreader_;
};

typedef test_group<test_geoswkbreader_data> group;
typedef group::object object;

group test_geoswkbreader("capi::GEOSWKBReader");

// Test readHEX
template<>
template<>
void object::test<1>()
{
    std::string wkb = "01010000a0917d0000000000000000084000000000000020400000000000000000";

    geom1_ = GEOSWKBReader_readHEX(wkbreader_, (unsigned char*) wkb.c_str(), wkb.size());

    ensure_equals(GEOSGeomTypeId(geom1_), GEOS_POINT);
    ensure_equals(GEOSGetSRID(geom1_), 32145);

    double x = -1;
    double y = -1;
    double z = -1;

    GEOSGeomGetX(geom1_, &x);
    GEOSGeomGetY(geom1_, &y);
    GEOSGeomGetZ(geom1_, &z);

    ensure_equals(x, 3);
    ensure_equals(y, 8);
    ensure_equals(z, 0);
}

// Test fixStructure
template<>
template<>
void object::test<2>()
{
    // WKB corresponding to POLYGON ((0 0, 1 0, 1 1, 0 1)) <-- not closed
    std::string wkb = "0103000000010000000400000000000000000000000000000000000000000000000000f03f0000000000000000000000000000f03f000000000000f03f0000000000000000000000000000f03f";

    geom1_ = GEOSWKBReader_readHEX(wkbreader_, (unsigned char*) wkb.c_str(), wkb.size());
    ensure(geom1_ == nullptr);

    GEOSWKBReader_setFixStructure(wkbreader_, 1);
    geom1_ = GEOSWKBReader_readHEX(wkbreader_, (unsigned char*) wkb.c_str(), wkb.size());

    expected_ = fromWKT("POLYGON ((0 0, 1 0, 1 1, 0 1, 0 0))");
    ensure_geometry_equals(geom1_, expected_);
}

} // namespace tut

