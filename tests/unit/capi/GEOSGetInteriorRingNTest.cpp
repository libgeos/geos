#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut
{
    //
    // Test Group
    //

    struct test_geosgetinteriorringn_data : public capitest::utility
    {
    };

    typedef test_group<test_geosgetinteriorringn_data> group;
    typedef group::object object;

    group test_geosgetinteriorring("capi::GEOSGetInteriorRingN");

    template <>
    template <>
    void object::test<1>()
    {
        geom1_ = fromWKT("POLYGON ((35 10, 45 45, 15 40, 10 20, 35 10),(20 30, 35 35, 30 20, 20 30))");
        ensure(nullptr != geom1_);
        GEOSGeometry *result = const_cast<GEOSGeometry *>(GEOSGetInteriorRingN(geom1_, 0));
        ensure(nullptr != result);
        ensure_equals("LINEARRING (20 30, 35 35, 30 20, 20 30)", toWKT(result));

        ensure(GEOSGetInteriorRingN(geom1_, -1) == nullptr);
    }

    template <>
    template <>
    void object::test<2>()
    {
        geom1_ = fromWKT("LINESTRING (0 0, 1 0, 1 1, 0 0)");
        ensure(nullptr != geom1_);

        ensure(GEOSGetInteriorRingN(geom1_, 0) == nullptr);
    }


} // namespace tut
