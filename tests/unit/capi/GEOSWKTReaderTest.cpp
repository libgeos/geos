#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoswktreader_data : public capitest::utility {

    test_geoswktreader_data() :
        wktreader_(GEOSWKTReader_create())
    {}

    ~test_geoswktreader_data() {
        GEOSWKTReader_destroy(wktreader_);
    }

    GEOSWKTReader* wktreader_;
};

typedef test_group<test_geoswktreader_data> group;
typedef group::object object;

group test_geoswktreader("capi::GEOSWKTReader");

// Test setStrictMode
template<>
template<>
void object::test<1>()
{
    std::string wkt = "POINTx (1 2)";
    GEOSWKTReader_setStrictMode(wktreader_, 1);
    geom1_ = GEOSWKTReader_read(wktreader_, wkt.c_str());
    GEOSWKTReader_setStrictMode(wktreader_, 0);
    ensure(geom1_ == nullptr);
}

// Test setStrictMode_r
template<>
template<>
void object::test<2>()
{
    std::string wkt = "POINTx (1 2)";

    GEOSContextHandle_t context = GEOS_init_r();

    GEOSWKTReader_setStrictMode_r(context, wktreader_, 1);
    geom1_ = GEOSWKTReader_read_r(context, wktreader_, wkt.c_str());
    GEOSWKTReader_setStrictMode_r(context, wktreader_, 0);

    ensure(geom1_ == nullptr);

    finishGEOS_r(context);
}

} // namespace tut

