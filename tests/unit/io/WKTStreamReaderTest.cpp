//
// Test Suite for geos::io::WKTStreamReader
//

// tut
#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>
// geos
#include <geos/io/WKTStreamReader.h>
#include <geos/io/ParseException.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
// std
#include <sstream>
#include <string>
#include <memory>

namespace tut {

struct test_wktstreamreader_data {
    typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;
};

typedef test_group<test_wktstreamreader_data> group;
typedef group::object object;

group test_wktstreamreader_group("geos::io::WKTStreamReader");

// 1 - Read single-line WKT
template<>
template<>
void object::test<1>
()
{
    std::istringstream ss("POINT(10 20)\nLINESTRING(0 0, 10 10)\n");
    geos::io::WKTStreamReader rdr(ss);

    auto g1 = rdr.next();
    ensure("g1 is not null", g1 != nullptr);
    ensure_equals("g1 type", g1->getGeometryType(), "Point");

    auto g2 = rdr.next();
    ensure("g2 is not null", g2 != nullptr);
    ensure_equals("g2 type", g2->getGeometryType(), "LineString");

    auto g3 = rdr.next();
    ensure("g3 is null at EOF", g3 == nullptr);
}

// 2 - Read empty geometries (no parens)
template<>
template<>
void object::test<2>
()
{
    std::istringstream ss("POINT EMPTY\nPOLYGON EMPTY\nLINESTRING EMPTY\n");
    geos::io::WKTStreamReader rdr(ss);

    auto g1 = rdr.next();
    ensure("g1 not null", g1 != nullptr);
    ensure("g1 isEmpty", g1->isEmpty());
    ensure_equals("g1 type", g1->getGeometryType(), "Point");

    auto g2 = rdr.next();
    ensure("g2 not null", g2 != nullptr);
    ensure("g2 isEmpty", g2->isEmpty());
    ensure_equals("g2 type", g2->getGeometryType(), "Polygon");

    auto g3 = rdr.next();
    ensure("g3 not null", g3 != nullptr);
    ensure("g3 isEmpty", g3->isEmpty());
    ensure_equals("g3 type", g3->getGeometryType(), "LineString");

    auto g4 = rdr.next();
    ensure("g4 null", g4 == nullptr);
}

// 3 - Read multi-line formatted WKT
template<>
template<>
void object::test<3>
()
{
    std::istringstream ss(
        "POLYGON (\n"
        "  (0 0, 10 0, 10 10, 0 10, 0 0),\n"
        "  (2 2, 8 2, 8 8, 2 8, 2 2)\n"
        ")\n"
        "POINT (5 5)\n"
    );
    geos::io::WKTStreamReader rdr(ss);

    auto g1 = rdr.next();
    ensure("g1 not null", g1 != nullptr);
    ensure_equals("g1 type", g1->getGeometryType(), "Polygon");

    auto g2 = rdr.next();
    ensure("g2 not null", g2 != nullptr);
    ensure_equals("g2 type", g2->getGeometryType(), "Point");

    auto g3 = rdr.next();
    ensure("g3 null", g3 == nullptr);
}

// 4 - Leading, trailing and intermediate whitespace/blank lines
template<>
template<>
void object::test<4>
()
{
    std::istringstream ss("\n\n   \nPOINT(1 1)\n\n\nPOINT(2 2)\n\n  \n");
    geos::io::WKTStreamReader rdr(ss);

    auto g1 = rdr.next();
    ensure("g1 not null", g1 != nullptr);
    ensure_equals("g1 type", g1->getGeometryType(), "Point");

    auto g2 = rdr.next();
    ensure("g2 not null", g2 != nullptr);
    ensure_equals("g2 type", g2->getGeometryType(), "Point");

    auto g3 = rdr.next();
    ensure("g3 null", g3 == nullptr);
}

// 5 - HEXWKB in stream raises ParseException
template<>
template<>
void object::test<5>
()
{
    std::istringstream ss("010100000000000000000024400000000000002440\n");
    geos::io::WKTStreamReader rdr(ss);

    try {
        rdr.next();
        fail("Expected ParseException on HEXWKB");
    } catch (const geos::io::ParseException&) {
        // Expected exception
    }
}

// 6 - Malformed / unclosed WKT raises ParseException
template<>
template<>
void object::test<6>
()
{
    std::istringstream ss("POLYGON ((0 0, 10 0, 10 10\n");
    geos::io::WKTStreamReader rdr(ss);

    try {
        rdr.next();
        fail("Expected ParseException on unclosed polygon");
    } catch (const geos::io::ParseException&) {
        // Expected exception
    }
}

// 7 - Empty stream returns nullptr
template<>
template<>
void object::test<7>
()
{
    std::istringstream ss("");
    geos::io::WKTStreamReader rdr(ss);

    auto g = rdr.next();
    ensure("null for empty stream", g == nullptr);
}

} // namespace tut
