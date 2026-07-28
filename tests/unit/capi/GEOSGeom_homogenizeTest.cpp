#include <tut/tut.hpp>
// geos
#include <geos_c.h>

#include "capi_test_utils.h"

namespace tut {
struct test_capigeosgeom_homogenize : public capitest::utility {

    void testHomogenize(const std::string& wkt_in, const std::string& wkt_expected) {
        GEOSGeometry* input = fromWKT(wkt_in.c_str());
        result_ = GEOSGeom_homogenize(input);
        expected_ = fromWKT(wkt_expected.c_str());

        ensure_geometry_equals(result_, expected_);
    }

    void testHomogenizeUnchanged(const std::string& wkt) {
        testHomogenize(wkt, wkt);
    }
};

typedef test_group<test_capigeosgeom_homogenize> group;
typedef group::object object;

group test_capigeosgeom_homogenize_group("capi::GEOSGeom_homogenize");

template<>
template<>
void object::test<1>()
{
    set_test_name("non-collection type");

    testHomogenizeUnchanged("LINESTRING (0 0, 1 1)");
}

template<>
template<>
void object::test<2>()
{
    set_test_name("non-homogeneous collection");

    testHomogenizeUnchanged("GEOMETRYCOLLECTION (LINESTRING (0 0, 1 1), POINT (2 2))");
}

template<>
template<>
void object::test<3>()
{
    set_test_name("single-element collection");

    testHomogenize("GEOMETRYCOLLECTION (POINT (0 2))", "POINT (0 2)");
    testHomogenize("MULTIPOINT ((8 2))", "POINT (8 2)");
}

template<>
template<>
void object::test<4>()
{
    set_test_name("homogeneous collection");

    testHomogenize("GEOMETRYCOLLECTION (LINESTRING (0 0, 1 1), LINESTRING (4 3, 2 7))",
              "MULTILINESTRING ((0 0, 1 1), (4 3, 2 7))" );
}

template<>
template<>
void object::test<5>()
{
    set_test_name("empty collection");

    testHomogenizeUnchanged("GEOMETRYCOLLECTION EMPTY");
}

template<>
template<>
void object::test<6>()
{
    set_test_name("nested homogeneous collection");

    testHomogenize("GEOMETRYCOLLECTION (LINESTRING (0 0, 1 1), GEOMETRYCOLLECTION(LINESTRING EMPTY, MULTILINESTRING ((1 1, 2 2), (2 2, 3 3))))",
        "MULTILINESTRING ((0 0, 1 1), (1 1, 2 2), (2 2, 3 3), EMPTY)");
}

template<>
template<>
void object::test<7>()
{
    set_test_name("nested non-homogeneous collection");

    testHomogenize("GEOMETRYCOLLECTION (LINESTRING (0 0, 1 1), GEOMETRYCOLLECTION(COMPOUNDCURVE ((1 1, 2 2))))",
        "MULTICURVE (LINESTRING (0 0, 1 1), COMPOUNDCURVE ((1 1, 2 2)))");
}

} // namespace tut

