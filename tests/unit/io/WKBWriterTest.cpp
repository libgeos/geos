//
// Test Suite for geos::io::WKBWriter

// tut
#include <tut/tut.hpp>
// geos
#include <geos/io/WKBReader.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/io/WKBConstants.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/util.h>
// std
#include <sstream>
#include <string>
#include <memory>
#include <cmath>

namespace tut {
//
// Test Group
//

using geos::geom::CoordinateSequence;
using geos::geom::CoordinateXYM;
using geos::geom::CoordinateXYZM;

// dummy data, not used
struct test_wkbwriter_data {
    geos::geom::PrecisionModel pm;
    geos::geom::GeometryFactory::Ptr gf;
    geos::io::WKTReader wktreader;
    geos::io::WKTWriter wktwriter;
    geos::io::WKBReader wkbreader;
    geos::io::WKBWriter wkbwriter;

    test_wkbwriter_data()
        :
        pm(1000.0),
        gf(geos::geom::GeometryFactory::create(&pm)),
        wktreader(gf.get()),
        wkbreader(*gf)
    {}

    void checkHexOutput(const std::string& wkt, const std::string& hex) {
        std::stringstream out;
        auto geom = wktreader.read(wkt);
        wkbwriter.setByteOrder(1); // little endian
        wkbwriter.writeHEX(*geom, out);
        ensure_equals(wkt, out.str(), hex );
    }

    void checkHexOutputWithSRID(const std::string& wkt, int srid, const std::string& hex) {
        std::stringstream out;
        auto geom = wktreader.read(wkt);
        geom->setSRID(srid);
        wkbwriter.setByteOrder(1); // little endian
        wkbwriter.setIncludeSRID(true);
        wkbwriter.writeHEX(*geom, out);
        ensure_equals(wkt, out.str(), hex );
    }

};

typedef test_group<test_wkbwriter_data> group;
typedef group::object object;

group test_wkbwriter_group("geos::io::WKBWriter");


//
// Test Cases
//

// 1 - Test writing a 2D geometry with the WKBWriter in 3D output dimension.
template<>
template<>
void object::test<1>
()
{
    auto geom = wktreader.read("POINT(-117 33)");
    std::stringstream result_stream;

    wkbwriter.setOutputDimension(3);
    wkbwriter.write(*geom, result_stream);

    ensure_equals(result_stream.str().length(), 21u);

    result_stream.seekg(0);
    geom = wkbreader.read(result_stream);

    ensure(geom != nullptr);

    ensure_equals(geom->getCoordinateDimension(), 2u);
    ensure_equals(geom->getCoordinate()->x, -117.0);
    ensure_equals(geom->getCoordinate()->y, 33.0);

    auto coords = geom->getCoordinates();
    ensure(std::isnan(coords->getAt(0).z));
}

// 2 - Test writing a 3D geometry with the WKBWriter in 3D output dimension.
template<>
template<>
void object::test<2>
()
{
    auto geom = wktreader.read("POINT(-117 33 11)");
    std::stringstream result_stream;

    wkbwriter.setOutputDimension(3);
    wkbwriter.write(*geom, result_stream);

    ensure(result_stream.str().length() == 29);

    result_stream.seekg(0);
    geom = wkbreader.read(result_stream);

    ensure_equals(geom->getCoordinateDimension(), 3u);
    ensure_equals(geom->getCoordinate()->x, -117.0);
    ensure_equals(geom->getCoordinate()->y, 33.0);

    auto coords = geom->getCoordinates();
    ensure_equals(coords->getAt(0).z, 11.0);
}

// 3 - Test writing a 3D geometry with the WKBWriter in 2D output dimension.
template<>
template<>
void object::test<3>
()
{
    auto geom = wktreader.read("POINT(-117 33 11)");
    std::stringstream result_stream;

    wkbwriter.setOutputDimension(2);
    wkbwriter.write(*geom, result_stream);

    ensure_equals(result_stream.str().length(), 21u);

    result_stream.seekg(0);
    geom = wkbreader.read(result_stream);

    ensure_equals(geom->getCoordinateDimension(), 2u);
    ensure_equals(geom->getCoordinate()->x, -117.0);
    ensure_equals(geom->getCoordinate()->y, 33.0);

    auto coords = geom->getCoordinates();
    ensure(std::isnan(coords->getAt(0).z));
}

// 4 - Test that SRID is output only once
// See http://trac.osgeo.org/geos/ticket/583
template<>
template<>
void object::test<4>
()
{
    std::vector<std::unique_ptr<geos::geom::Geometry>> geoms;
    geoms.push_back(wktreader.read("POLYGON((0 0,1 0,1 1,0 1,0 0))"));
    geoms.back()->setSRID(4326);
    auto geom = gf->createGeometryCollection(std::move(geoms));
    geom->setSRID(4326);
    std::stringstream result_stream;

    wkbwriter.setOutputDimension(2);
    wkbwriter.setByteOrder(1);
    wkbwriter.setIncludeSRID(1);
    wkbwriter.writeHEX(*geom, result_stream);

    std::string actual = result_stream.str();
    ensure_equals(actual,
                  "0107000020E6100000010000000103000000010000000500000000000000000000000000000000000000000000000000F03F0000000000000000000000000000F03F000000000000F03F0000000000000000000000000000F03F00000000000000000000000000000000");

}

// 5 - Check WKB representation of empty polygon
// See http://trac.osgeo.org/geos/ticket/680
template<>
template<>
void object::test<5>
()
{
    auto geom = wktreader.read("POLYGON EMPTY");
    geom->setSRID(4326);
    std::stringstream result_stream;

    wkbwriter.setOutputDimension(2);
    wkbwriter.setByteOrder(1);
    wkbwriter.setIncludeSRID(1);
    wkbwriter.writeHEX(*geom, result_stream);

    std::string actual = result_stream.str();
    ensure_equals(actual, "0103000020E610000000000000");

    auto geom2 = wkbreader.readHEX(result_stream);
    // assert(geom->equals(geom2.get()));
}


// 5 - Check WKB representation of empty polygon
// See http://trac.osgeo.org/geos/ticket/680
template<>
template<>
void object::test<6>
()
{
    auto geom = wktreader.read("POINT EMPTY");
    geom->setSRID(4326);
    std::stringstream result_stream;

    wkbwriter.setOutputDimension(2);
    wkbwriter.setByteOrder(1);
    wkbwriter.setIncludeSRID(1);
    wkbwriter.writeHEX(*geom, result_stream);

    std::string actual = result_stream.str();
    ensure_equals(actual, "0101000020E6100000000000000000F87F000000000000F87F");

    auto geom2 = wkbreader.readHEX(result_stream);
    // assert(geom->equals(geom2.get()));
}

// https://trac.osgeo.org/geos/ticket/1048
// Higher dimension empty. Need correct support in WKT reader and in
// WKB writer.
template<>
template<>
void object::test<7>
()
{
    auto geom = wktreader.read("POINT Z EMPTY");
    geom->setSRID(4326);
    std::stringstream result_stream;

    wkbwriter.setOutputDimension(3);
    wkbwriter.setByteOrder(1);
    wkbwriter.setIncludeSRID(1);
    wkbwriter.writeHEX(*geom, result_stream);

    std::string actual = result_stream.str();
    ensure_equals(actual, "01010000A0E6100000000000000000F87F000000000000F87F000000000000F87F");

    auto geom2 = wkbreader.readHEX(result_stream);
    // assert(geom->equals(geom2.get()));
}

template<>
template<>
void object::test<8>
()
{
    auto geom = wktreader.read("LINESTRING Z EMPTY");
    geom->setSRID(4326);
    std::stringstream result_stream;

    wkbwriter.setOutputDimension(3);
    wkbwriter.setByteOrder(1);
    wkbwriter.setIncludeSRID(1);
    wkbwriter.writeHEX(*geom, result_stream);

    std::string actual = result_stream.str();
    ensure_equals(actual, "01020000A0E610000000000000");

    auto geom2 = wkbreader.readHEX(result_stream);
    // assert(geom->equals(geom2.get()));
}

template<>
template<>
void object::test<9>
()
{
    auto geom = wktreader.read("GEOMETRYCOLLECTION (POINT EMPTY)");
    geom->setSRID(4326);
    std::stringstream result_stream;

    wkbwriter.setOutputDimension(3);
    wkbwriter.setByteOrder(1);
    wkbwriter.setIncludeSRID(0);
    wkbwriter.writeHEX(*geom, result_stream);

    std::string actual = result_stream.str();
    ensure_equals(actual, "0107000000010000000101000000000000000000F87F000000000000F87F");

    auto geom2 = wkbreader.readHEX(result_stream);
    // assert(geom->equals(geom2.get()));
}

// Test writing a 3D geometry with the WKBWriter in ISO flavor
template<>
template<>
void object::test<10>
()
{
    auto geom = wktreader.read("POINT(-117 33 11)");
    std::stringstream result_stream;

    wkbwriter.setOutputDimension(3);
    wkbwriter.setFlavor(geos::io::WKBConstants::wkbIso);
    wkbwriter.write(*geom, result_stream);
    wkbwriter.setByteOrder(0);

    ensure(result_stream.str().length() == 29);

    result_stream.seekg(0);
    geom = wkbreader.read(result_stream);

    ensure(geom->getCoordinateDimension() == 3);
    ensure(geom->getCoordinate()->x == -117.0);
    ensure(geom->getCoordinate()->y == 33.0);

    auto coords = geom->getCoordinates();
    ensure_equals(coords->getAt(0).z, 11.0);

    result_stream.str("");
    wkbwriter.writeHEX(*geom, result_stream);
    std::string actual = result_stream.str();
    // std::cout << std::endl << actual << std::endl;

    // 00 == big endian
    // 000003E9 == 1001
    // C05D400000000000 == -117
    // 4040800000000000 == 33
    // 4026000000000000 == 11
    ensure_equals(actual,
                  "00000003E9C05D40000000000040408000000000004026000000000000");
}

// Test writing XYZM
template<>
template<>
void object::test<11>
()
{
    auto coords = geos::detail::make_unique<CoordinateSequence>(2u, true, true);
    coords->setAt(CoordinateXYZM{1, 2, 3, 4}, 0);
    coords->setAt(CoordinateXYZM{5, 6, 7, 8}, 1);

    auto geom = gf->createLineString(std::move(coords));

    std::stringstream iso_result_stream;
    wkbwriter.setOutputDimension(4);
    wkbwriter.setFlavor(geos::io::WKBConstants::wkbFlavour::wkbIso);
    wkbwriter.setByteOrder(1); // little endian
    wkbwriter.writeHEX(*geom, iso_result_stream);

    ensure_equals("XYZM ISO",
                  iso_result_stream.str(),
                  "01BA0B000002000000000000000000F03F000000000000004000000000000008400000000000001040000000000000144000000000000018400000000000001C400000000000002040");

    std::stringstream extended_result_stream;
    wkbwriter.setFlavor(geos::io::WKBConstants::wkbFlavour::wkbExtended);
    wkbwriter.writeHEX(*geom, extended_result_stream);

    ensure_equals("XYZM extended",
                  extended_result_stream.str(),
                  "01020000C002000000000000000000F03F000000000000004000000000000008400000000000001040000000000000144000000000000018400000000000001C400000000000002040");

}

// Test writing XYM
template<>
template<>
void object::test<12>
()
{
    auto coords = geos::detail::make_unique<CoordinateSequence>(2u, false, true);
    coords->setAt(CoordinateXYM{1, 2, 3}, 0);
    coords->setAt(CoordinateXYM{4, 5, 6}, 1);

    auto geom = gf->createLineString(std::move(coords));

    std::stringstream iso_result_stream;
    wkbwriter.setOutputDimension(4);
    wkbwriter.setFlavor(geos::io::WKBConstants::wkbFlavour::wkbIso);
    wkbwriter.setByteOrder(1); // little endian
    wkbwriter.writeHEX(*geom, iso_result_stream);

    ensure_equals("XYM ISO",
                  iso_result_stream.str(),
                  "01D207000002000000000000000000F03F00000000000000400000000000000840000000000000104000000000000014400000000000001840");

    std::stringstream extended_result_stream;
    wkbwriter.setFlavor(geos::io::WKBConstants::wkbFlavour::wkbExtended);
    wkbwriter.writeHEX(*geom, extended_result_stream);

    ensure_equals("XYM extended",
                  extended_result_stream.str(),
                  "010200004002000000000000000000F03F00000000000000400000000000000840000000000000104000000000000014400000000000001840");
}

// CircularString
template<>
template<>
void object::test<13>
()
{
    checkHexOutput("CIRCULARSTRING EMPTY",
                   "010800000000000000");

    checkHexOutput("CIRCULARSTRING (1 3, 2 4, 3 1)",
                   "010800000003000000000000000000F03F0000000000000840000000000000004000000000000010400000000000000840000000000000F03F");
}

// CompoundCurve
template<>
template<>
void object::test<14>
()
{
    checkHexOutput("COMPOUNDCURVE EMPTY",
                   "010900000000000000");

    checkHexOutput("COMPOUNDCURVE (CIRCULARSTRING (1 3, 2 4, 3 1), (3 1, 0 0))",
                   "010900000002000000010800000003000000000000000000F03F0000000000000840000000000000004000000000000010400000000000000840000000000000F03F0102000000020000000000000000000840000000000000F03F00000000000000000000000000000000");
}

// CurvePolygon
template<>
template<>
void object::test<15>
()
{
    checkHexOutput("CURVEPOLYGON EMPTY",
                   "010A00000000000000");

    checkHexOutput("CURVEPOLYGON( COMPOUNDCURVE( CIRCULARSTRING(0 0,2 0, 2 1, 2 3, 4 3), (4 3, 4 5, 1 4, 0 0)), CIRCULARSTRING(1.7 1, 1.4 0.4, 1.6 0.4, 1.6 0.5, 1.7 1) )",
                   "010A0000000200000001090000000200000001080000000500000000000000000000000000000000000000000000000000004000000000000000000000000000000040000000000000F03F00000000000000400000000000000840000000000000104000000000000008400102000000040000000000000000001040000000000000084000000000000010400000000000001440000000000000F03F000000000000104000000000000000000000000000000000010800000005000000333333333333FB3F000000000000F03F666666666666F63F9A9999999999D93F9A9999999999F93F9A9999999999D93F9A9999999999F93F000000000000E03F333333333333FB3F000000000000F03F");
}

// MultiCurve
template<>
template<>
void object::test<16>
()
{
    checkHexOutput("MULTICURVE EMPTY",
                   "010B00000000000000");

    checkHexOutput("MULTICURVE( (0 0, 5 5), COMPOUNDCURVE( (-1 -1, 0 0), CIRCULARSTRING (0 0, 1 1, 2 0)), CIRCULARSTRING(4 0, 4 4, 8 4))",
                   "010B000000030000000102000000020000000000000000000000000000000000000000000000000014400000000000001440010900000002000000010200000002000000000000000000F0BF000000000000F0BF0000000000000000000000000000000001080000000300000000000000000000000000000000000000000000000000F03F000000000000F03F00000000000000400000000000000000010800000003000000000000000000104000000000000000000000000000001040000000000000104000000000000020400000000000001040" );
}

// MultiSurface
template<>
template<>
void object::test<17>
()
{
    checkHexOutput("MULTISURFACE EMPTY",
                   "010C00000000000000");

    checkHexOutput("MULTISURFACE( CURVEPOLYGON( CIRCULARSTRING( 0 0, 4 0, 4 4, 0 4, 0 0), (1 1, 3 3, 3 1, 1 1)), POLYGON ((10 10, 14 12, 11 10, 10 10), (11 11, 11.5 11, 11 11.5, 11 11)))",
                   "010C00000002000000010A000000020000000108000000050000000000000000000000000000000000000000000000000010400000000000000000000000000000104000000000000010400000000000000000000000000000104000000000000000000000000000000000010200000004000000000000000000F03F000000000000F03F000000000000084000000000000008400000000000000840000000000000F03F000000000000F03F000000000000F03F01030000000200000004000000000000000000244000000000000024400000000000002C40000000000000284000000000000026400000000000002440000000000000244000000000000024400400000000000000000026400000000000002640000000000000274000000000000026400000000000002640000000000000274000000000000026400000000000002640");
}

// CompoundCurve with SRID
template<>
template<>
void object::test<18>
()
{
    checkHexOutputWithSRID("COMPOUNDCURVE (CIRCULARSTRING (1 3, 2 4, 3 1), (3 1, 0 0))", 5646,
                  "01090000200E16000002000000010800000003000000000000000000F03F0000000000000840000000000000004000000000000010400000000000000840000000000000F03F0102000000020000000000000000000840000000000000F03F00000000000000000000000000000000");
}

// CurvePolygon with SRID
template<>
template<>
void object::test<19>
()
{
    checkHexOutputWithSRID("CURVEPOLYGON( COMPOUNDCURVE( CIRCULARSTRING(0 0,2 0, 2 1, 2 3, 4 3), (4 3, 4 5, 1 4, 0 0)), CIRCULARSTRING(1.7 1, 1.4 0.4, 1.6 0.4, 1.6 0.5, 1.7 1) )", 5646,
                           "010A0000200E1600000200000001090000000200000001080000000500000000000000000000000000000000000000000000000000004000000000000000000000000000000040000000000000F03F00000000000000400000000000000840000000000000104000000000000008400102000000040000000000000000001040000000000000084000000000000010400000000000001440000000000000F03F000000000000104000000000000000000000000000000000010800000005000000333333333333FB3F000000000000F03F666666666666F63F9A9999999999D93F9A9999999999F93F9A9999999999D93F9A9999999999F93F000000000000E03F333333333333FB3F000000000000F03F");
}


} // namespace tut

