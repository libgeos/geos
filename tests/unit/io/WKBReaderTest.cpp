//
// Test Suite for geos::io::WKBReader
// Uses geos::io::WKTReader to check correctness.
// Uses geos::io::WKBWriter to check correctness.
// Currently only tests 2D geoms of all (7) types.
// Tests NDR and XDR input and output .

// tut
#include <tut/tut.hpp>
// geos
#include <geos/io/WKBReader.h>
#include <geos/io/WKBConstants.h>
#include <geos/io/WKBWriter.h>
#include <geos/io/WKTReader.h>
#include <geos/geom/CompoundCurve.h>
#include <geos/geom/CurvePolygon.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/util/GEOSException.h>
// std
#include <sstream>
#include <string>
#include <memory>

namespace tut {
//
// Test Group
//

using geos::geom::CoordinateXYZM;

// dummy data, not used
struct test_wkbreader_data {
    geos::geom::PrecisionModel pm;
    geos::geom::GeometryFactory::Ptr gf;
    geos::io::WKBReader wkbreader;
    geos::io::WKBWriter xdrwkbwriter;
    geos::io::WKBWriter ndrwkbwriter;
    geos::io::WKBWriter ndr3dwkbwriter;
    geos::io::WKTReader wktreader;

    typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;

    test_wkbreader_data()
        :
        pm(1.0),
        gf(geos::geom::GeometryFactory::create(&pm)),
        wkbreader(*gf),
        // 2D only, XDR (big endian)
        xdrwkbwriter(2, geos::io::WKBConstants::wkbXDR),
        // 2D only, NDR (little endian)
        ndrwkbwriter(2, geos::io::WKBConstants::wkbNDR),
        // 3D only, NDR (little endian)
        ndr3dwkbwriter(3, geos::io::WKBConstants::wkbNDR),
        wktreader(gf.get())
    {}

    void
    testParseError(const std::string& hexwkb, const std::string& errstr)
    {
        std::string exstr;
        std::stringstream hexin(hexwkb);
        try {
            GeomPtr g(wkbreader.readHEX(hexin));
            fail();
        }
        catch(const geos::util::GEOSException& ex) {
            // std::cout << e.what() << std::endl;
            exstr = ex.what();
            ensure("Missing expected error", !exstr.empty());
            ensure_equals("Parse error incorrect", exstr, errstr);
        }
    }

    GeomPtr
    readHex(const std::string& hexwkb)
    {
        std::stringstream hexin(hexwkb);
        GeomPtr g(wkbreader.readHEX(hexin));
        return g;
    }

    void
    testInput(const std::string& hexwkb,
              const std::string& expected)
    {
        std::stringstream hexin(hexwkb);
        GeomPtr g(wkbreader.readHEX(hexin));
        std::stringstream ndr_out;
        ndr3dwkbwriter.writeHEX(*g, ndr_out);
        ensure_equals("hex output",
                      ndr_out.str(), expected.c_str());
    }

    void
    testInputNdr(const std::string& WKT,
                 const std::string& ndrWKB)
    {
        GeomPtr gWKT(wktreader.read(WKT));
        // NDR input
        std::stringstream ndr_in(ndrWKB);
        GeomPtr gWKB_ndr(wkbreader.readHEX(ndr_in));
        ensure("NDR input", gWKB_ndr->equalsExact(gWKT.get()));
    }

    void
    testInputOutput(const std::string& WKT,
                    const std::string& ndrWKB,
                    const std::string& xdrWKB)
    {
        GeomPtr gWKT(wktreader.read(WKT));

        // NDR input
        std::stringstream ndr_in(ndrWKB);
        GeomPtr gWKB_ndr(wkbreader.readHEX(ndr_in));
        ensure("NDR input",
               gWKB_ndr->equalsExact(gWKT.get()));

        // XDR input
        std::stringstream xdr_in(xdrWKB);
        GeomPtr gWKB_xdr(wkbreader.readHEX(xdr_in));
        ensure("XDR input",
               gWKB_xdr->equalsExact(gWKT.get()));

        // Compare geoms read from NDR and XDR
        ensure(gWKB_xdr->equalsExact(gWKB_ndr.get()));

        // NDR output
        std::stringstream ndr_out;
        ndrwkbwriter.writeHEX(*gWKT, ndr_out);
        ensure_equals("NDR output",
                      ndr_out.str(), ndr_in.str());

        // XDR output
        std::stringstream xdr_out;
        xdrwkbwriter.writeHEX(*gWKT, xdr_out);
        ensure_equals("XDR output",
                      xdr_out.str(), xdr_in.str());

    }

};

typedef test_group<test_wkbreader_data> group;
typedef group::object object;

group test_wkbreader_group("geos::io::WKBReader");


//
// Test Cases
//

// 1 - Read/write a point in XDR and NDR format
template<>
template<>
void object::test<1>
()
{
    testInputOutput(

        // WKT
        "POINT(0 0)",

        // NDR HEXWKB
        "010100000000000000000000000000000000000000",

        // XDR HEXWKB
        "000000000100000000000000000000000000000000"

    );
}

// 2 - Read a linestring
template<>
template<>
void object::test<2>
()
{

    testInputOutput(

        // WKT
        "LINESTRING(1 2, 3 4)",

        // NDR HEXWKB
        "010200000002000000000000000000F03F000000000000004000000000000008400000000000001040",

        // XDR HEXWKB
        "0000000002000000023FF0000000000000400000000000000040080000000000004010000000000000"

    );

}

// 3 - Read a polygon
template<>
template<>
void object::test<3>
()
{
    testInputOutput(

        // WKT
        "POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 2 6, 6 4, 2 2))",

        // NDR HEXWKB
        "0103000000020000000500000000000000000000000000000000000000000000000000244000000000000000000000000000002440000000000000244000000000000000000000000000002440000000000000000000000000000000000400000000000000000000400000000000000040000000000000004000000000000018400000000000001840000000000000104000000000000000400000000000000040",

        // XDR HEXWKB
        "0000000003000000020000000500000000000000000000000000000000402400000000000000000000000000004024000000000000402400000000000000000000000000004024000000000000000000000000000000000000000000000000000440000000000000004000000000000000400000000000000040180000000000004018000000000000401000000000000040000000000000004000000000000000"

    );

}

// 4 - Read a multipoint
template<>
template<>
void object::test<4>
()
{

    testInputOutput(

        // WKT
        "MULTIPOINT((0 0), (10 0), (10 10), (0 10), (0 0))",

        // NDR HEXWKB
        "010400000005000000010100000000000000000000000000000000000000010100000000000000000024400000000000000000010100000000000000000024400000000000002440010100000000000000000000000000000000002440010100000000000000000000000000000000000000",

        // XDR HEXWKB
        "000000000400000005000000000100000000000000000000000000000000000000000140240000000000000000000000000000000000000140240000000000004024000000000000000000000100000000000000004024000000000000000000000100000000000000000000000000000000"

    );

}

// 5 - Read a multilinestring
template<>
template<>
void object::test<5>
()
{

    testInputOutput(

        // WKT
        "MULTILINESTRING((0 0, 10 0, 10 10, 0 10, 10 20),(2 2, 2 6, 6 4, 20 2))",

        // NDR HEXWKB
        "010500000002000000010200000005000000000000000000000000000000000000000000000000002440000000000000000000000000000024400000000000002440000000000000000000000000000024400000000000002440000000000000344001020000000400000000000000000000400000000000000040000000000000004000000000000018400000000000001840000000000000104000000000000034400000000000000040",

        // XDR HEXWKB
        "000000000500000002000000000200000005000000000000000000000000000000004024000000000000000000000000000040240000000000004024000000000000000000000000000040240000000000004024000000000000403400000000000000000000020000000440000000000000004000000000000000400000000000000040180000000000004018000000000000401000000000000040340000000000004000000000000000"

    );

}

// 6 - Read a multipolygon
template<>
template<>
void object::test<6>
()
{

    testInputOutput(

        // WKT
        "MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 2 6, 6 4, 2 2)),((60 60, 60 50, 70 40, 60 60)))",

        // NDR HEXWKB
        "0106000000020000000103000000020000000500000000000000000000000000000000000000000000000000244000000000000000000000000000002440000000000000244000000000000000000000000000002440000000000000000000000000000000000400000000000000000000400000000000000040000000000000004000000000000018400000000000001840000000000000104000000000000000400000000000000040010300000001000000040000000000000000004E400000000000004E400000000000004E400000000000004940000000000080514000000000000044400000000000004E400000000000004E40",

        // XDR HEXWKB
        "000000000600000002000000000300000002000000050000000000000000000000000000000040240000000000000000000000000000402400000000000040240000000000000000000000000000402400000000000000000000000000000000000000000000000000044000000000000000400000000000000040000000000000004018000000000000401800000000000040100000000000004000000000000000400000000000000000000000030000000100000004404E000000000000404E000000000000404E000000000000404900000000000040518000000000004044000000000000404E000000000000404E000000000000"

    );

}

// 7 - Read a collection
template<>
template<>
void object::test<7>
()
{

    testInputOutput(

        // WKT
        "GEOMETRYCOLLECTION(POINT(0 0),LINESTRING(1 2,3 4),POLYGON((0 0,10 0,10 10,0 10,0 0),(2 2,2 6,6 4,2 2)),MULTIPOINT((0 0),(10 0),(10 10),(0 10),(0 0)),MULTILINESTRING((0 0,10 0,10 10,0 10,10 20),(2 2,2 6,6 4,20 2)),MULTIPOLYGON(((0 0,10 0,10 10,0 10,0 0),(2 2,2 6,6 4,2 2)),((60 60,60 50,70 40,60 60))))",

        // NDR HEXWKB
        "010700000006000000010100000000000000000000000000000000000000010200000002000000000000000000F03F00000000000000400000000000000840000000000000104001030000000200000005000000000000000000000000000000000000000000000000002440000000000000000000000000000024400000000000002440000000000000000000000000000024400000000000000000000000000000000004000000000000000000004000000000000000400000000000000040000000000000184000000000000018400000000000001040000000000000004000000000000000400104000000050000000101000000000000000000000000000000000000000101000000000000000000244000000000000000000101000000000000000000244000000000000024400101000000000000000000000000000000000024400101000000000000000000000000000000000000000105000000020000000102000000050000000000000000000000000000000000000000000000000024400000000000000000000000000000244000000000000024400000000000000000000000000000244000000000000024400000000000003440010200000004000000000000000000004000000000000000400000000000000040000000000000184000000000000018400000000000001040000000000000344000000000000000400106000000020000000103000000020000000500000000000000000000000000000000000000000000000000244000000000000000000000000000002440000000000000244000000000000000000000000000002440000000000000000000000000000000000400000000000000000000400000000000000040000000000000004000000000000018400000000000001840000000000000104000000000000000400000000000000040010300000001000000040000000000000000004E400000000000004E400000000000004E400000000000004940000000000080514000000000000044400000000000004E400000000000004E40",

        // XDR HEXWKB
        "0000000007000000060000000001000000000000000000000000000000000000000002000000023FF00000000000004000000000000000400800000000000040100000000000000000000003000000020000000500000000000000000000000000000000402400000000000000000000000000004024000000000000402400000000000000000000000000004024000000000000000000000000000000000000000000000000000440000000000000004000000000000000400000000000000040180000000000004018000000000000401000000000000040000000000000004000000000000000000000000400000005000000000100000000000000000000000000000000000000000140240000000000000000000000000000000000000140240000000000004024000000000000000000000100000000000000004024000000000000000000000100000000000000000000000000000000000000000500000002000000000200000005000000000000000000000000000000004024000000000000000000000000000040240000000000004024000000000000000000000000000040240000000000004024000000000000403400000000000000000000020000000440000000000000004000000000000000400000000000000040180000000000004018000000000000401000000000000040340000000000004000000000000000000000000600000002000000000300000002000000050000000000000000000000000000000040240000000000000000000000000000402400000000000040240000000000000000000000000000402400000000000000000000000000000000000000000000000000044000000000000000400000000000000040000000000000004018000000000000401800000000000040100000000000004000000000000000400000000000000000000000030000000100000004404E000000000000404E000000000000404E000000000000404900000000000040518000000000004044000000000000404E000000000000404E000000000000"

    );

}

// 8 - Invalid HEXWKB for missing HEX char (#675)
template<>
template<>
void object::test<8>
()
{
    std::stringstream hexwkb;
    // NOTE: add a 0 to make valid
    hexwkb << "01010000000000000000000000000000000000000";
    //hexwkb << "0";
    std::string err;
    try {
        GeomPtr gWKB_ndr(wkbreader.readHEX(hexwkb));
    }
    catch(const geos::util::GEOSException& ex) {
        err = ex.what();
    }
    ensure("Missing expected error", !err.empty());
    ensure_equals(err, "ParseException: Premature end of HEX string");
}

// 9 - Extended HEXWKB (3dZ + srid)
template<>
template<>
void object::test<9>
()
{
    std::stringstream hexwkb;
    hexwkb <<
// SRID=4326;POINT(1 2 3)
// NDR HEXEWKB
           "01010000A0E6100000000000000000F03F00000000000000400000000000000840";
    std::string err;
    GeomPtr gWKB(wkbreader.readHEX(hexwkb));
    ensure_equals(gWKB->getSRID(), 4326);
    ensure_equals(gWKB->getCoordinateDimension(), 3);
}

// 10 - Extended WKB with Z
template<>
template<>
void object::test<10>
()
{
    testInputNdr(
        // WKT
        "POINT Z(1 2 3)",
        // NDR HEXWKB
        "01010000A0E6100000000000000000F03F00000000000000400000000000000840"
    );
}

// 11 - Extended WKB with Z
template<>
template<>
void object::test<11>
()
{
    testInputNdr(
        // WKT
        "LINESTRING Z(1 2 3, 4 5 6)",
        // NDR HEXWKB
        "01020000A0E610000002000000000000000000F03F00000000000000400000000000000840000000000000104000000000000014400000000000001840"
    );
}

// 12 - Extended WKB with Z
template<>
template<>
void object::test<12>
()
{
    testInputNdr(
        // WKT
        "POLYGON Z((0 0 100,0 10 100,10 10 100,10 0 100,0 0 100),(1 1 100,1 9 100,9 9 100,9 1 100,1 1 100))",
        // NDR HEXWKB
        "01030000A0E6100000020000000500000000000000000000000000000000000000000000000000594000000000000000000000000000002440000000000000594000000000000024400000000000002440000000000000594000000000000024400000000000000000000000000000594000000000000000000000000000000000000000000000594005000000000000000000F03F000000000000F03F0000000000005940000000000000F03F000000000000224000000000000059400000000000002240000000000000224000000000000059400000000000002240000000000000F03F0000000000005940000000000000F03F000000000000F03F0000000000005940"
    );
}

// 13 - Extended WKB with Z
template<>
template<>
void object::test<13>
()
{
    testInputNdr(
        // WKT
        "MULTIPOINT Z((0 1 2), (3 4 5))",
        // NDR HEXWKB
        "01040000A0E61000000200000001010000800000000000000000000000000000F03F00000000000000400101000080000000000000084000000000000010400000000000001440"
    );
}

// 14 - Extended WKB with Z
template<>
template<>
void object::test<14>
()
{
    testInputNdr(
        // WKT
        "MULTILINESTRING Z((0 1 2,3 4 5),(6 7 8,9 10 11))",
        // NDR HEXWKB
        "01050000A0E6100000020000000102000080020000000000000000000000000000000000F03F000000000000004000000000000008400000000000001040000000000000144001020000800200000000000000000018400000000000001C400000000000002040000000000000224000000000000024400000000000002640"
    );
}

// 15 - Extended WKB with Z
template<>
template<>
void object::test<15>
()
{
    testInputNdr(
        // WKT
        "MULTIPOLYGON Z(((0 0 100,0 10 100,10 10 100,10 0 100,0 0 100),(1 1 100,1 9 100,9 9 100,9 1 100,1 1 100)),((-9 0 50,-9 10 50,-1 10 50,-1 0 50,-9 0 50)))",
        // NDR HEXWKB
        "01060000A0E6100000020000000103000080020000000500000000000000000000000000000000000000000000000000594000000000000000000000000000002440000000000000594000000000000024400000000000002440000000000000594000000000000024400000000000000000000000000000594000000000000000000000000000000000000000000000594005000000000000000000F03F000000000000F03F0000000000005940000000000000F03F000000000000224000000000000059400000000000002240000000000000224000000000000059400000000000002240000000000000F03F0000000000005940000000000000F03F000000000000F03F00000000000059400103000080010000000500000000000000000022C00000000000000000000000000000494000000000000022C000000000000024400000000000004940000000000000F0BF00000000000024400000000000004940000000000000F0BF0000000000000000000000000000494000000000000022C000000000000000000000000000004940"
    );
}


// 16 - Read a empty multipoint
template<>
template<>
void object::test<16>
()
{

    testInputOutput(
        // WKT
        "MULTIPOINT EMPTY",
        // NDR HEXWKB
        "010400000000000000",
        // XDR HEXWKB
        "000000000400000000"
    );

}


// 17 - Read a empty linestring
template<>
template<>
void object::test<17>
()
{

    testInputOutput(
        // WKT
        "LINESTRING EMPTY",
        // NDR HEXWKB
        "010200000000000000",
        // XDR HEXWKB
        "000000000200000000"
    );

}


// 18 - Read a empty polygon
template<>
template<>
void object::test<18>
()
{

    testInputOutput(
        // WKT
        "POLYGON EMPTY",
        // NDR HEXWKB
        "010300000000000000",
        // XDR HEXWKB
        "000000000300000000"
    );

}

// 19 - Read a empty collection
template<>
template<>
void object::test<19>
()
{

    testInputOutput(
        // WKT
        "GEOMETRYCOLLECTION EMPTY",
        // NDR HEXWKB
        "010700000000000000",
        // XDR HEXWKB
        "000000000700000000"
    );

}


// POINT M (1 2 3)
template<>
template<>
void object::test<20>
()
{
    auto g = readHex("01D1070000000000000000F03F00000000000000400000000000000840");

    auto coords = g->getCoordinates();

    ensure_equals("size", coords->size(), 1u);
    ensure("!hasZ", !coords->hasZ());
    ensure("hasM", coords->hasM());

    CoordinateXYZM coord;
    coords->getAt(0, coord);
    ensure_equals(coord.x, 1);
    ensure_equals(coord.y, 2);
    ensure_equals(coord.m, 3);
    ensure(std::isnan(coord.z));
}

// POINT ZM (1 2 3 4)
template<>
template<>
void object::test<21>
()
{
    auto g = readHex("01B90B0000000000000000F03F000000000000004000000000000008400000000000001040");

    auto coords = g->getCoordinates();

    ensure_equals("size", coords->size(), 1u);
    ensure("hasZ", coords->hasZ());
    ensure("hasM", coords->hasM());

    CoordinateXYZM coord;
    coords->getAt(0, coord);
    ensure_equals(coord.x, 1);
    ensure_equals(coord.y, 2);
    ensure_equals(coord.z, 3);
    ensure_equals(coord.m, 4);
}

// LINESTRING M (1 2 3, 4 5 6)
template<>
template<>
void object::test<22>
()
{
    auto g = readHex("01D207000002000000000000000000F03F00000000000000400000000000000840000000000000104000000000000014400000000000001840");

    auto coords = g->getCoordinates();

    ensure_equals("size", coords->size(), 2u);
    ensure("!hasZ", !coords->hasZ());
    ensure("hasM", coords->hasM());

    CoordinateXYZM c0, c1;
    coords->getAt(0, c0);
    ensure_equals(c0.x, 1);
    ensure_equals(c0.y, 2);
    ensure_equals(c0.m, 3);
    ensure(std::isnan(c0.z));

    coords->getAt(1, c1);
    ensure_equals(c1.x, 4);
    ensure_equals(c1.y, 5);
    ensure_equals(c1.m, 6);
    ensure(std::isnan(c1.z));
}

// LINESTRING ZM (1 2 3 4, 5 6 7 8)
template<>
template<>
void object::test<23>
()
{
    auto g = readHex("01BA0B000002000000000000000000F03F000000000000004000000000000008400000000000001040000000000000144000000000000018400000000000001C400000000000002040");

    auto coords = g->getCoordinates();

    ensure_equals("size", coords->size(), 2u);
    ensure("hasZ", coords->hasZ());
    ensure("hasM", coords->hasM());

    CoordinateXYZM c0, c1;
    coords->getAt(0, c0);
    ensure_equals(c0.x, 1);
    ensure_equals(c0.y, 2);
    ensure_equals(c0.z, 3);
    ensure_equals(c0.m, 4);

    coords->getAt(1, c1);
    ensure_equals(c1.x, 5);
    ensure_equals(c1.y, 6);
    ensure_equals(c1.z, 7);
    ensure_equals(c1.m, 8);
}

// EMPTY WKB TESTS
template<>
template<>
void object::test<24>
()
{
    GeomPtr g;

    // POINT EMPTY
    g = readHex(std::string("0101000000000000000000F87F000000000000F87F"));
    ensure("POINT EMPTY isEmpty", g->isEmpty());
    ensure_equals("POINT EMPTY getCoordinateDimension", g->getCoordinateDimension(), 2);

    // POINT Z EMPTY
    g = readHex(std::string("0101000080000000000000F87F000000000000F87F000000000000F87F"));
    ensure("POINT Z EMPTY isEmpty", g->isEmpty());
    ensure_equals("POINT Z EMPTY getCoordinateDimension", g->getCoordinateDimension(), 3);

    // POINT M EMPTY
    g = readHex(std::string("01D1070000000000000000F87F000000000000F87F000000000000F87F"));
    ensure("POINT M EMPTY isEmpty", g->isEmpty());
    ensure_equals("POINT M EMPTY getCoordinateDimension", g->getCoordinateDimension(), 3);

    // POINT ZM EMPTY
    g = readHex(std::string("01B90B0000000000000000F87F000000000000F87F000000000000F87F000000000000F87F"));
    ensure("POINT ZM EMPTY isEmpty", g->isEmpty());
    ensure_equals("POINT ZM EMPTY getCoordinateDimension", g->getCoordinateDimension(), 4);

    // LINESTRING EMPTY
    g = readHex(std::string("010200000000000000"));
    ensure("LINESTRING EMPTY isEmpty", g->isEmpty());
    ensure_equals("LINESTRING EMPTY getCoordinateDimension", g->getCoordinateDimension(), 2);

    // LINESTRING Z EMPTY
    g = readHex(std::string("010200008000000000"));
    ensure("LINESTRING Z EMPTY isEmpty", g->isEmpty());
    ensure_equals("LINESTRING Z EMPTY getCoordinateDimension", g->getCoordinateDimension(), 3);

    // LINESTRING M EMPTY
    g = readHex(std::string("01D207000000000000"));
    ensure("LINESTRING M EMPTY isEmpty", g->isEmpty());
    ensure_equals("LINESTRING M EMPTY getCoordinateDimension", g->getCoordinateDimension(), 3);

    // LINESTRING ZM EMPTY
    g = readHex(std::string("01BA0B000000000000"));
    ensure("LINESTRING ZM EMPTY isEmpty", g->isEmpty());
    ensure_equals("LINESTRING ZM EMPTY getCoordinateDimension", g->getCoordinateDimension(), 4);

    // POLYGON EMPTY
    g = readHex(std::string("010300000000000000"));
    ensure("POLYGON EMPTY isEmpty", g->isEmpty());
    ensure_equals("POLYGON EMPTY getCoordinateDimension", g->getCoordinateDimension(), 2);

    // POLYGON Z EMPTY
    g = readHex(std::string("010300008000000000"));
    ensure("POLYGON Z EMPTY isEmpty", g->isEmpty());
    ensure_equals("POLYGON Z EMPTY getCoordinateDimension", g->getCoordinateDimension(), 3);

    // POLYGON M EMPTY
    g = readHex(std::string("01D307000000000000"));
    ensure("POLYGON M EMPTY isEmpty", g->isEmpty());
    ensure_equals("POLYGON M EMPTY getCoordinateDimension", g->getCoordinateDimension(), 3);

    // POLYGON ZM EMPTY
    g = readHex(std::string("01BB0B000000000000"));
    ensure("POLYGON ZM EMPTY isEmpty", g->isEmpty());
    ensure_equals("POLYGON ZM EMPTY getCoordinateDimension", g->getCoordinateDimension(), 4);
}

// Malformed WKB wrong coordinate count
template<>
template<>
void object::test<25>
()
{
    testParseError(
        "010200000003000000000000000000F03F000000000000004000000000000008400000000000001040",
        "ParseException: Input buffer is smaller than requested object size"
    );
}

// Malformed WKB with very large coordinate count
template<>
template<>
void object::test<26>
()
{
    testParseError(
        "010200000000000080000000000000F03F000000000000004000000000000008400000000000001040",
        "ParseException: Input buffer is smaller than requested object size"
    );
}


// Malformed WKB polygon with very large ring count
template<>
template<>
void object::test<27>
()
{
    testParseError(
        "01030000000000008001000000000000000000F03F000000000000004000000000000008400000000000001040",
        "ParseException: Input buffer is smaller than requested object size"
    );
}

// Malformed WKB polygon with slightly large ring count
template<>
template<>
void object::test<28>
()
{
    testParseError(
        "01030000000200000000000000",
        "ParseException: Input buffer is smaller than requested object size"
    );
}

// Malformed WKB polygon with more buffer than data
template<>
template<>
void object::test<29>
()
{
    testInput(
        "01030000000100000000000000000000000000F03F000000000000004000000000000008400000000000001040",
        "010300000000000000"
    );
}

// Malformed WKB collection with overly large geom count
template<>
template<>
void object::test<30>
()
{
    testParseError(
        "010700000009000000010100000000000000000010400000000000001040",
        "ParseException: Input buffer is smaller than requested object size"
    );
}

// CircularString
template<>
template<>
void object::test<31>
()
{
    // CIRCULARSTRING(1 3,2 4,3 1)
    auto g = readHex("010800000003000000000000000000F03F0000000000000840000000000000004000000000000010400000000000000840000000000000F03F");

    ensure_equals(g->getGeometryTypeId(), geos::geom::GEOS_CIRCULARSTRING);
    ensure_equals(g->getNumPoints(), 3u);
}

// CompoundCurve
template<>
template<>
void object::test<32>
()
{
    //  SRID=5646;COMPOUNDCURVE(CIRCULARSTRING(1 3,2 4,3 1),(3 1,0 0))
    auto g = readHex("01090000200E16000002000000010800000003000000000000000000F03F0000000000000840000000000000004000000000000010400000000000000840000000000000F03F0102000000020000000000000000000840000000000000F03F00000000000000000000000000000000");

    ensure_equals(g->getGeometryTypeId(), geos::geom::GEOS_COMPOUNDCURVE);
    ensure_equals(g->getSRID(), 5646);

    const auto* cc = static_cast<const geos::geom::CompoundCurve*>(g.get());
    ensure_equals(cc->getNumCurves(), 2u);
    ensure_equals(cc->getCurveN(0)->getGeometryTypeId(), geos::geom::GEOS_CIRCULARSTRING);
    ensure_equals(cc->getCurveN(0)->getNumPoints(), 3u);

    ensure_equals(cc->getCurveN(1)->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals(cc->getCurveN(1)->getNumPoints(), 2u);
}

// CurvePolygon
template<>
template<>
void object::test<33>
()
{
    // SRID=5646;CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING(0 0,2 0,2 1,2 3,4 3),(4 3,4 5,1 4,0 0)),
    //                        CIRCULARSTRING(1.7 1,1.4 0.4,1.6 0.4,1.6 0.5,1.7 1))
    auto g = readHex("010A0000200E1600000200000001090000000200000001080000000500000000000000000000000000000000000000000000000000004000000000000000000000000000000040000000000000F03F00000000000000400000000000000840000000000000104000000000000008400102000000040000000000000000001040000000000000084000000000000010400000000000001440000000000000F03F000000000000104000000000000000000000000000000000010800000005000000333333333333FB3F000000000000F03F666666666666F63F9A9999999999D93F9A9999999999F93F9A9999999999D93F9A9999999999F93F000000000000E03F333333333333FB3F000000000000F03F");

    ensure_equals(g->getGeometryTypeId(), geos::geom::GEOS_CURVEPOLYGON);
    ensure_equals(g->getSRID(), 5646);

    const auto* cp = static_cast<const geos::geom::CurvePolygon*>(g.get());
    ensure_equals(cp->getExteriorRing()->getGeometryTypeId(), geos::geom::GEOS_COMPOUNDCURVE);
    ensure_equals(cp->getNumInteriorRing(), 1u);
    ensure_equals(cp->getInteriorRingN(0)->getGeometryTypeId(), geos::geom::GEOS_CIRCULARSTRING);
}

// MultiCurve
template<>
template<>
void object::test<34>
()
{
    // MULTICURVE((0 0,5 5),
    //            COMPOUNDCURVE((-1 -1,0 0),CIRCULARSTRING(0 0,1 1,2 0)),
    //            CIRCULARSTRING(4 0,4 4,8 4))
    auto g = readHex("010B000000030000000102000000020000000000000000000000000000000000000000000000000014400000000000001440010900000002000000010200000002000000000000000000F0BF000000000000F0BF0000000000000000000000000000000001080000000300000000000000000000000000000000000000000000000000F03F000000000000F03F00000000000000400000000000000000010800000003000000000000000000104000000000000000000000000000001040000000000000104000000000000020400000000000001040");
    ensure_equals(g->getGeometryTypeId(), geos::geom::GEOS_MULTICURVE);
    ensure_equals(g->getNumGeometries(), 3u);
    ensure_equals(g->getGeometryN(0)->getGeometryTypeId(), geos::geom::GEOS_LINESTRING);
    ensure_equals(g->getGeometryN(1)->getGeometryTypeId(), geos::geom::GEOS_COMPOUNDCURVE);
    ensure_equals(g->getGeometryN(2)->getGeometryTypeId(), geos::geom::GEOS_CIRCULARSTRING);
}

// MultiSurface
template<>
template<>
void object::test<35>
()
{
    // MULTISURFACE(CURVEPOLYGON(CIRCULARSTRING(0 0,4 0,4 4,0 4,0 0),(1 1,3 3,3 1,1 1)),
    //             ((10 10,14 12,11 10,10 10),(11 11,11.5 11,11 11.5,11 11))
    auto g = readHex("010C00000002000000010A000000020000000108000000050000000000000000000000000000000000000000000000000010400000000000000000000000000000104000000000000010400000000000000000000000000000104000000000000000000000000000000000010200000004000000000000000000F03F000000000000F03F000000000000084000000000000008400000000000000840000000000000F03F000000000000F03F000000000000F03F01030000000200000004000000000000000000244000000000000024400000000000002C40000000000000284000000000000026400000000000002440000000000000244000000000000024400400000000000000000026400000000000002640000000000000274000000000000026400000000000002640000000000000274000000000000026400000000000002640");
    ensure_equals(g->getGeometryTypeId(), geos::geom::GEOS_MULTISURFACE);
    ensure_equals(g->getNumGeometries(), 2u);
    ensure_equals(g->getGeometryN(0)->getGeometryTypeId(), geos::geom::GEOS_CURVEPOLYGON);
    ensure_equals(g->getGeometryN(1)->getGeometryTypeId(), geos::geom::GEOS_POLYGON);
}

// Invalid CompoundCurve with Point as a member
template<>
template<>
void object::test<36>
()
{
    testParseError("01090000200E160000010000000101000000000000000000F03F000000000000F03F",
                   "ParseException: Expected SimpleCurve but got Point");
}

} // namespace tut

