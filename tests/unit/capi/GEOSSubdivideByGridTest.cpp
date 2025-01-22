#include <tut/tut.hpp>
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {

struct test_capigeossubdividebygrid_data : public capitest::utility {};

typedef test_group<test_capigeossubdividebygrid_data> group;
typedef group::object object;

group test_capigeossubdividebygrid_group("capi::GEOSSubdivideByGrid");

template<>
template<>
void object::test<1>()
{
    set_test_name("rectangle overlapping grid");

    input_ = fromWKT("POLYGON ((0.5 0.5, 2.5 0.5, 2.5 2.5, 0.5 2.5, 0.5 0.5))");
    expected_ = fromWKT("GEOMETRYCOLLECTION ("
                        "POLYGON ((2 2.5, 1 2.5, 1 2, 2 2, 2 2.5)),"
                        "POLYGON ((2.5 2, 2.5 2.5, 2 2.5, 2 2, 2.5 2)),"
                        "POLYGON ((1 1, 2 1, 2 2, 1 2, 1 1)),"
                        "POLYGON ((2.5 1, 2.5 2, 2 2, 2 1, 2.5 1)),"
                        "POLYGON ((1 0.5, 2 0.5, 2 1, 1 1, 1 0.5)),"
                        "POLYGON ((2 0.5, 2.5 0.5, 2.5 1, 2 1, 2 0.5)))");

    result_ = GEOSSubdivideByGrid(input_, 1, 0, 5, 3, 4, 3, false);

    ensure_geometry_equals_identical(expected_, result_);
}

template<>
template<>
void object::test<2>()
{
    set_test_name("includeExterior = true");

    input_ = fromWKT("POLYGON ((0.5 0.5, 2.5 0.5, 2.5 2.5, 0.5 2.5, 0.5 0.5))");
    expected_ = fromWKT("GEOMETRYCOLLECTION ("
        "POLYGON ((2 2.5, 1 2.5, 1 2, 2 2, 2 2.5)),"
        "POLYGON ((2.5 2, 2.5 2.5, 2 2.5, 2 2, 2.5 2)),"
        "POLYGON ((1 1, 2 1, 2 2, 1 2, 1 1)),"
        "POLYGON ((2.5 1, 2.5 2, 2 2, 2 1, 2.5 1)),"
        "POLYGON ((1 0.5, 2 0.5, 2 1, 1 1, 1 0.5)),"
        "POLYGON ((2 0.5, 2.5 0.5, 2.5 1, 2 1, 2 0.5)),"
        "POLYGON ((0.5 2.5, 1 2.5, 1 2, 1 1, 1 0.5, 0.5 0.5, 0.5 1, 0.5 2, 0.5 2.5)))");

    result_ = GEOSSubdivideByGrid(input_, 1, 0, 5, 3, 4, 3, true);

    ensure_geometry_equals_identical(expected_, result_);
}

}