//
// Test Suite for Geometry's toText() function

// tut
#include <tut/tut.hpp>
// geos
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
// std
#include <memory>
#include <string>

namespace tut {

//
// Test Group
//

struct test_totext_data {
    typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;
    geos::io::WKTReader reader;

    test_totext_data()
        : reader()
    {}
};

typedef test_group<test_totext_data> group;
typedef group::object object;

group test_totext_data("geos::geom::Geometry::toText");

//
// Test Cases
//

// Input WKT is the same as output from toText()
template<> template<> void object::test<1>
()
{

    std::vector<std::string> variants{
        "POINT (1 2)",
        "POINT Z (1 2 3)",
        "POINT M (1 2 4)",
        "POINT ZM (1 2 3 4)",
        "POINT EMPTY",
        "POINT Z EMPTY",
        "GEOMETRYCOLLECTION EMPTY",
    };
    for (const auto& wkt : variants) {
      GeomPtr g1(reader.read(wkt));
      ensure_equals(g1->toText(), wkt);
    }

}

} // namespace tut

