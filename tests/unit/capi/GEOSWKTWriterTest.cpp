#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
//
// Test Group
//

struct test_geoswktwriter_data : public capitest::utility {

    test_geoswktwriter_data() :
        wktwriter_(GEOSWKTWriter_create()),
        wkt_(nullptr)
    {}

    ~test_geoswktwriter_data() {
        GEOSWKTWriter_destroy(wktwriter_);
        GEOSFree(wkt_);
    }

    void
    test_writer_wkt(std::string const& wkt)
    {
        geom1_ = GEOSGeomFromWKT(wkt.c_str());
        ensure(nullptr != geom1_);

        wkt_ = GEOSWKTWriter_write(wktwriter_, geom1_);
        std::string out(wkt_);

        ensure_equals(out, wkt);
    }

    GEOSWKTWriter* wktwriter_;
    char* wkt_;
};

typedef test_group<test_geoswktwriter_data> group;
typedef group::object object;

group test_geoswktwriter("capi::GEOSWKTWriter");

template<>
template<>
void object::test<1>()
{
    ensure_equals(GEOSWKTWriter_getOutputDimension(wktwriter_), 2);
}

template<>
template<>
void object::test<2>()
{
    GEOSWKTWriter_setTrim(wktwriter_, 1);
    GEOSWKTWriter_setOutputDimension(wktwriter_, 3);
    ensure_equals("getOutputDimension_2", GEOSWKTWriter_getOutputDimension(wktwriter_), 3);

    test_writer_wkt("POINT Z (10 13 3)");
}

template<>
template<>
void object::test<3>()
{
    GEOSWKTWriter_setTrim(wktwriter_, 1);
    GEOSWKTWriter_setOutputDimension(wktwriter_, 3);
    GEOSWKTWriter_setOld3D(wktwriter_, 1);

    test_writer_wkt("POINT (10 13 3)");
}

} // namespace tut

