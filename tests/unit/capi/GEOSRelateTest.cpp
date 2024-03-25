#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geosrelate_data : public capitest::utility {};

typedef test_group<test_geosrelate_data> group;
typedef group::object object;

group test_geosrelate("capi::GEOSRelate");

template<>
template<>
void object::test<1>()
{
    geom1_ = fromWKT("LINESTRING(1 2, 3 4)");
    ensure(nullptr != geom1_);
    geom2_ = fromWKT("LINESTRING(5 6, 7 8)");
    ensure(nullptr != geom2_);

    char* pattern = GEOSRelate(geom1_, geom2_);
    ensure_equals(std::string{"FF1FF0102"}, pattern);
    GEOSFree(pattern);
}

template<>
template<>
void object::test<2>()
{
    geom1_ = fromWKT("CIRCULARSTRING (0 0, 1 1, 2 0)");
    geom2_ = fromWKT("LINESTRING (1 0, 2 1)");

    ensure(geom1_);
    ensure(geom2_);

    ensure("curved geometry not supported", GEOSRelate(geom1_, geom2_) == nullptr);
    ensure("curved geometry not supported", GEOSRelate(geom2_, geom1_) == nullptr);
}

} // namespace tut

