//
// Test Suite for geos::io::WKTFileReader
//

// tut
#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>
// geos
#include <geos/io/WKTFileReader.h>
#include <geos/io/ParseException.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/util/GEOSException.h>
// std
#include <fstream>
#include <string>
#include <memory>
#include <cstdio>

namespace tut {

struct test_wktfilereader_data {
    typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;
};

typedef test_group<test_wktfilereader_data> group;
typedef group::object object;

group test_wktfilereader_group("geos::io::WKTFileReader");

// 1 - Read WKT file containing various geometries including EMPTY
template<>
template<>
void object::test<1>
()
{
    const std::string tmpfile = "test_wktfilereader_tmp1.wkt";
    {
        std::ofstream ofs(tmpfile);
        ofs << "POINT (1 1)\n";
        ofs << "POINT EMPTY\n";
        ofs << "LINESTRING (0 0, 10 10)\n";
        ofs << "POLYGON EMPTY\n";
    }

    geos::io::WKTFileReader reader;
    auto geoms = reader.read(tmpfile);
    std::remove(tmpfile.c_str());

    ensure_equals("geoms count", geoms.size(), 4u);
    ensure_equals("geom 0 type", geoms[0]->getGeometryType(), "Point");
    ensure("geom 0 not empty", !geoms[0]->isEmpty());
    ensure_equals("geom 1 type", geoms[1]->getGeometryType(), "Point");
    ensure("geom 1 empty", geoms[1]->isEmpty());
    ensure_equals("geom 2 type", geoms[2]->getGeometryType(), "LineString");
    ensure_equals("geom 3 type", geoms[3]->getGeometryType(), "Polygon");
    ensure("geom 3 empty", geoms[3]->isEmpty());
}

// 2 - Non-existent file throws GEOSException
template<>
template<>
void object::test<2>
()
{
    geos::io::WKTFileReader reader;
    try {
        reader.read("non_existent_file_xyz_12345.wkt");
        fail("Expected GEOSException on missing file");
    } catch (const geos::util::GEOSException&) {
        // Expected
    }
}

// 3 - HEXWKB in file throws ParseException
template<>
template<>
void object::test<3>
()
{
    const std::string tmpfile = "test_wktfilereader_tmp2.wkt";
    {
        std::ofstream ofs(tmpfile);
        ofs << "010100000000000000000024400000000000002440\n";
    }

    geos::io::WKTFileReader reader;
    try {
        reader.read(tmpfile);
        std::remove(tmpfile.c_str());
        fail("Expected ParseException on HEXWKB in WKT file");
    } catch (const geos::io::ParseException&) {
        std::remove(tmpfile.c_str());
        // Expected
    }
}

} // namespace tut
