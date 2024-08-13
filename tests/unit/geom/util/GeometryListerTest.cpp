//
// Test Suite for geos::geom::util::GeometryLister class.

// tut
#include <tut/tut.hpp>

// geos
#include <geos/geom/Geometry.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/util/GeometryLister.h>

// std
#include <vector>

namespace tut {
//
// Test Group
//

using namespace geos::geom::util;
using namespace geos::geom;

// Common data used by tests
struct test_geometrylister_data {

    geos::io::WKTReader _reader;

    test_geometrylister_data() {};

    void checkList(const std::string& wkt, std::size_t nSingletons)
    {
        std::unique_ptr<Geometry> geom(_reader.read(wkt));
        std::vector<const Geometry*> elems;
        GeometryLister::list(geom.get(), elems);
        ensure_equals(elems.size(), nSingletons);
    }
};

typedef test_group<test_geometrylister_data> group;
typedef group::object object;

group test_geometrylister_group("geos::geom::util::GeometryLister");

//
// Test Cases
//

template<>
template<>
void object::test<1> ()
{
    checkList("POINT(1 1)", 1);
}

template<>
template<>
void object::test<2> ()
{
    checkList("GEOMETRYCOLLECTION(MULTIPOINT(-117 33,-33 44),LINESTRING(0 0, 10 0),POINT(0 0),POLYGON((0 0, 10 0, 10 10, 0 10, 0 0)),GEOMETRYCOLLECTION(POINT(3 4)))", 6);
}

} // namespace tut
