//
// Test Suite for geos::operation::cluster::GeometryFlattener class.

// tut
#include <tut/tut.hpp>
// geos
#include <geos/operation/cluster/GeometryFlattener.h>
#include <geos/io/WKTReader.h>

namespace tut {
//
// Test Group
//

// Common data used by tests
struct test_flattener_data {
    geos::io::WKTReader reader;

    void checkFlattener(const std::string& input_wkt, const std::string& expected_wkt) {
        const auto& flattened = geos::operation::cluster::GeometryFlattener::flatten(reader.read(input_wkt));
        const auto& expected = reader.read(expected_wkt);

        ensure_equals(flattened->getGeometryType(), expected->getGeometryType());
        ensure(flattened->equals(expected.get()));
    }

    void checkFlattenerUnchanged(const std::string& input_wkt) {
        checkFlattener(input_wkt, input_wkt);
    }
};

typedef test_group<test_flattener_data> group;
typedef group::object object;

group test_flattenerrtest_group("geos::operation::cluster::GeometryFlattener");

// empty geometry unchanged
template<>
template<>
void object::test<1>()
{
    checkFlattenerUnchanged("POINT EMPTY");
    checkFlattenerUnchanged("LINESTRING EMPTY");
    checkFlattenerUnchanged("POLYGON EMPTY");
    checkFlattenerUnchanged("MULTIPOINT EMPTY");
    checkFlattenerUnchanged("MULTILINESTRING EMPTY");
    checkFlattenerUnchanged("MULTIPOLYGON EMPTY");
    checkFlattenerUnchanged("GEOMETRYCOLLECTION EMPTY");
}

// single-part geometry unchanged
template<>
template<>
void object::test<2>()
{
    checkFlattenerUnchanged("POINT (3 8)");
    checkFlattenerUnchanged("LINESTRING (3 8, 2 2)");
    checkFlattenerUnchanged("POLYGON ((0 0, 0 1, 1 1, 0 0))");
}

// single-part collections simplified
template<>
template<>
void object::test<3>()
{
    checkFlattener("GEOMETRYCOLLECTION (POINT (1 1))", "POINT (1 1)");
    checkFlattener("MULTIPOINT ((1 1))", "POINT (1 1)");
}

// most narrow representation used
template<>
template<>
void object::test<4>()
{
    checkFlattener("GEOMETRYCOLLECTION (POINT (1 1), MULTIPOINT ((1 2), (1 3)), GEOMETRYCOLLECTION (POINT (1 4), POINT EMPTY))",
                   "MULTIPOINT ((1 1), (1 2), (1 3), (1 4), EMPTY)");
    checkFlattener("GEOMETRYCOLLECTION(MULTILINESTRING ((1 1, 2 2)), MULTIPOINT ((3 3), (4 4)))",
                   "GEOMETRYCOLLECTION(LINESTRING (1 1, 2 2), POINT (3 3), POINT (4 4))");
}

}
