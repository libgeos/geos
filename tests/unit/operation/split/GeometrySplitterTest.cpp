#include <tut/tut.hpp>
#include <tut/tut_macros.hpp>
#include <utility.h>

#include <geos/operation/split/GeometrySplitter.h>
#include <geos/io/WKBReader.h>

using geos::geom::CircularString;
using geos::geom::LineString;
using geos::operation::split::GeometrySplitter;

namespace tut {

struct test_geometrysplitter_data {
    const geos::io::WKTReader reader_;

    void testSplit(const std::string& wktGeom, const std::string& wktEdge, const std::string& wktExpected) const
    {
        auto geom = reader_.read(wktGeom);
        auto edge = reader_.read(wktEdge);
        auto split = GeometrySplitter::split(*geom, *edge);
        auto expected = reader_.read(wktExpected);

        WKTWriter writer;
        writer.setRoundingPrecision(-1);
        //std::cout << writer.write(split.get()) << std::endl;

        ensure_equals_geometry_xyzm(split.get(), expected.get());
    }
};

typedef test_group<test_geometrysplitter_data, 255> group;
typedef group::object object;

group test_geometrysplitter_group("geos::operation::split::GeometrySplitter");

template<>
template<>
void object::test<1>()
{
    set_test_name("split square in two parts; PostGIS test #20");

    testSplit("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
              "LINESTRING (5 0, 5 10)",
              "GEOMETRYCOLLECTION (POLYGON ((0 0, 5 0, 5 10, 0 10, 0 0)), POLYGON ((5 0, 10 0, 10 10, 5 10, 5 0)))");
}

template<>
template<>
void object::test<2>()
{
    set_test_name("split multipart geometry");

    auto geom = reader_.read("MULTIPOINT (0 0, 5 0, 2.5 2.5)")->buffer(1, 1);
    auto edge = reader_.read("LINESTRING (-5 0.5, 15 0.5)");

    auto split = GeometrySplitter::split(*geom, *edge);
    ensure_equals(split->getNumGeometries(), 5u);

    // SELECT ST_AsText(ST_Split(ST_Buffer('MULTIPOINT (0 0, 5 0, 2.5 2.5)', 1, 1), 'LINESTRING (-5 .5, 15 0.5)'));
    auto expected = reader_.read(
                        "GEOMETRYCOLLECTION (POLYGON ((6 0, 5 -1, 4 0, 4.5 0.5, 5.5 0.5, 6 0)),"
                        "POLYGON ((4.5 0.5, 5 1, 5.5 0.5, 4.5 0.5)),"
                        "POLYGON ((3.5 2.5, 2.5 1.5, 1.5 2.5, 2.5 3.5, 3.5 2.5)), "
                        "POLYGON ((1 0, 0 -1, -1 0, -0.5 0.5, 0.5 0.5, 1 0)), "
                        "POLYGON ((-0.5 0.5, 0 1, 0.5 0.5, -0.5 0.5)))");

    ensure_equals_geometry(split.get(), expected.get());
}

template<>
template<>
void object::test<3>()
{
    set_test_name("split edge ends inside polygon");

    // SELECT ST_AsText(ST_Split('POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))', 'LINESTRING (-5 5, 5 5)'))
    // returned geometry is a GeometryCollection even though it only has a single component
    // node is added where split line intersects input
    testSplit("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
              "LINESTRING (5 -5, 5 5)",
              "GEOMETRYCOLLECTION( POLYGON ((0 0, 5 0, 10 0, 10 10, 0 10, 0 0)))");
}

template<>
template<>
void object::test<4>()
{
    set_test_name("split line is collinear with polygon edge");

    // SELECT ST_AsText(ST_Split('POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))', 'LINESTRING (-5 5, 0 5, 0 8, 15 8)'));
    testSplit("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))",
              "LINESTRING (-5 5, 0 5, 0 8, 15 8)",
              "GEOMETRYCOLLECTION(POLYGON((10 8,10 0,0 0,0 5,0 8,10 8)),POLYGON((0 8,0 10,10 10,10 8,0 8)))");
}

template<>
template<>
void object::test<5>()
{
    set_test_name("geometry to be split contains points");

    auto edge = reader_.read("LINESTRING (3 7, 8 2)");

    for (std::string wkt: {
                "POINT (3 7)",
                "GEOMETRYCOLLECTION (POLYGON ((0 0, 10 0, 10 10, 0 0)), POINT (3 7))"
            }) {
        auto geom = reader_.read(wkt);
        ensure_THROW(GeometrySplitter::split(*geom, *edge), geos::util::IllegalArgumentException);
    }
}

template<>
template<>
void object::test<6>()
{
    set_test_name("splitting edge does not touch geometry; PostGIS ticket #745");

    testSplit("POLYGON Z((-72 42 1,-70 43 1,-71 41 1,-72 42 1))",
              "LINESTRING(-10 40 1,-9 41 1)",
              "GEOMETRYCOLLECTION(POLYGON Z((-72 42 1,-70 43 1,-71 41 1,-72 42 1)))");
}

template<>
template<>
void object::test<7>()
{
    set_test_name("Split single-hole Polygon by line crossing both exterior and hole; PostGIS test #21");

    testSplit("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 8 2, 8 8, 2 8, 2 2))",
              "LINESTRING(5 -5, 5 15)",
              "GEOMETRYCOLLECTION(POLYGON((5 0,5 2,8 2,8 8,5 8,5 10,10 10,10 0,5 0)),POLYGON((0 0,0 10,5 10,5 8,2 8,2 2,5 2,5 0,0 0)))");
}

template<>
template<>
void object::test<8>()
{
    set_test_name("Split single-hole Polygon by line crossing only exterior; PostGIS test #22");

    testSplit("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(5 2, 8 2, 8 8, 5 8, 5 2))",
              "LINESTRING(2 -5, 2 15)",
              "GEOMETRYCOLLECTION(POLYGON((2 0,2 10,10 10,10 0,2 0),(5 2,8 2,8 8,5 8,5 2)),POLYGON((0 0,0 10,2 10,2 0,0 0)))");
}

template<>
template<>
void object::test<9>()
{
    set_test_name("Split double-hole Polygon by line crossing exterior and both holes; PostGIS test #23");

    testSplit("POLYGON((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 8 2, 8 4, 2 4, 2 2),(2 6,8 6,8 8,2 8,2 6))",
              "LINESTRING(5 -5, 5 15)",
              "GEOMETRYCOLLECTION(POLYGON((5 0,5 2,8 2,8 4,5 4,5 6,8 6,8 8,5 8,5 10,10 10,10 0,5 0)),POLYGON((0 0,0 10,5 10,5 8,2 8,2 6,5 6,5 4,2 4,2 2,5 2,5 0,0 0)))");
}

template<>
template<>
void object::test<10>()
{
    set_test_name("Split MultiPolygon by line; PostGIS test #50");

    testSplit("MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 8 2, 8 4, 2 4, 2 2),(2 6,8 6,8 8,2 8,2 6)),((20 0,20 10, 30 10, 30 0, 20 0),(25 5, 28 5, 25 8, 25 5)))",
              "LINESTRING(5 -5, 5 15)",
              "GEOMETRYCOLLECTION(POLYGON((5 0,5 2,8 2,8 4,5 4,5 6,8 6,8 8,5 8,5 10,10 10,10 0,5 0)),POLYGON((0 0,0 10,5 10,5 8,2 8,2 6,5 6,5 4,2 4,2 2,5 2,5 0,0 0)),POLYGON((20 0,20 10,30 10,30 0,20 0),(25 5,28 5,25 8,25 5)))");
}

template<>
template<>
void object::test<11>()
{
    set_test_name("Split mixed poly/line GeometryCollection by line; PostGIS test #60");

    testSplit("GEOMETRYCOLLECTION(MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 8 2, 8 4, 2 4, 2 2),(2 6,8 6,8 8,2 8,2 6)),((20 0,20 10, 30 10, 30 0, 20 0),(25 5, 28 5, 25 8, 25 5))),MULTILINESTRING((0 0, 10 0),(0 5, 10 5)))",
              "LINESTRING(5 -5, 5 15)",
              "GEOMETRYCOLLECTION(POLYGON((5 0,5 2,8 2,8 4,5 4,5 6,8 6,8 8,5 8,5 10,10 10,10 0,5 0)),POLYGON((0 0,0 10,5 10,5 8,2 8,2 6,5 6,5 4,2 4,2 2,5 2,5 0,0 0)),POLYGON((20 0,20 10,30 10,30 0,20 0),(25 5,28 5,25 8,25 5)),LINESTRING(5 5,10 5),LINESTRING(5 0,10 0),LINESTRING(0 5,5 5),LINESTRING(0 0,5 0))");
}

template<>
template<>
void object::test<12>()
{
    set_test_name("dimension preserved on empty geometry");

    // PostGIS returns GEOMETRYCOLLECTION Z EMPTY here
    // That's not currently possible in GEOS, where GeometryCollections don't have their own
    // dimension; see https://github.com/libgeos/geos/issues/888.
    testSplit("POLYGON ZM EMPTY",
              "LINESTRING EMPTY",
              "GEOMETRYCOLLECTION (POLYGON ZM EMPTY)");
}

template<>
template<>
void object::test<14>()
{
    set_test_name("split MultiLineString with Point (liblwgeom)");

    testSplit("MULTILINESTRING((-5 -2,0 0),(0 0,10 10))",
              "POINT(0 0)",
              "GEOMETRYCOLLECTION(LINESTRING(-5 -2,0 0),LINESTRING(0 0,10 10))");
}

template<>
template<>
void object::test<15>()
{
    set_test_name("split LineString with Point (liblwgeom)");

    testSplit("LINESTRING(0 0,10 0,20 4,0 3)",
              "POINT(10 0)",
              "GEOMETRYCOLLECTION(LINESTRING(0 0,10 0),LINESTRING(10 0,20 4,0 3))" );
}

template<>
template<>
void object::test<16>()
{
    set_test_name("split LineString with LineString (liblwgeom)");

    testSplit("LINESTRING(0 1,10 1)",
              "LINESTRING(7 0,7 3)",
              "GEOMETRYCOLLECTION(LINESTRING(0 1,7 1),LINESTRING(7 1,10 1))");
}

template<>
template<>
void object::test<17>()
{
    set_test_name("split LineString with MultiLineString (liblwgeom)");

    testSplit("LINESTRING(0 0, 10 0)",
              "MULTILINESTRING((1 1,1 -1),(2 1,2 -1,3 -1,3 1))",
              "GEOMETRYCOLLECTION(LINESTRING(0 0,1 0),LINESTRING(1 0,2 0),LINESTRING(2 0,3 0),LINESTRING(3 0,10 0))");
}

template<>
template<>
void object::test<18>()
{
    set_test_name("split LineString with Polygon (liblwgeom)");

    testSplit("LINESTRING(0 0, 10 0)",
              "POLYGON((1 -2,1 1,2 1,2 -1,3 -1,3 1,11 1,11 -2,1 -2))",
              "GEOMETRYCOLLECTION(LINESTRING(0 0,1 0),LINESTRING(1 0,2 0),LINESTRING(2 0,3 0),LINESTRING(3 0,10 0))");
}

template<>
template<>
void object::test<19>()
{
    set_test_name("split LineString with empty Polygon (liblwgeom)");

    testSplit("LINESTRING(0 0, 10 0)",
              "POLYGON EMPTY",
              "GEOMETRYCOLLECTION(LINESTRING(0 0, 10 0))");
}

template<>
template<>
void object::test<20>()
{
    set_test_name("split LineString with MultiPolygon (liblwgeom)");

    testSplit("LINESTRING(0 0, 10 0)",
              "MULTIPOLYGON(((1 -1,1 1,2 1,2 -1,1 -1)),((3 -1,3 1,11 1,11 -1,3 -1)))",
              "GEOMETRYCOLLECTION(LINESTRING(0 0,1 0),LINESTRING(1 0,2 0),LINESTRING(2 0,3 0),LINESTRING(3 0,10 0))");
}

template<>
template<>
void object::test<21>()
{
    set_test_name("split LineString with MultiPoint (liblwgeom)");

    testSplit("LINESTRING(0 0, 10 0)",
              "MULTIPOINT(2 0,8 0,4 0)",
              "GEOMETRYCOLLECTION(LINESTRING(8 0,10 0),LINESTRING(0 0,2 0),LINESTRING(4 0,8 0),LINESTRING(2 0,4 0))");

}

template<>
template<>
void object::test<22>()
{
    set_test_name("split LineString with Point (PostGIS ticket #3401)");

    testSplit("LINESTRING(-180 0,0 0)",
              "POINT(-20 0)",
              "GEOMETRYCOLLECTION(LINESTRING (-180 0, -20 0), LINESTRING (-20 0, 0 0))");
}

template<>
template<>
void object::test<23>()
{
    set_test_name("split LineString with Point (PostGIS ticket #5698)");

    testSplit("LINESTRING(15.796760167740288 69.05714853429149,15.796760167739626 69.05714853429157,15.795906966300288 69.05725770093837)",
              "POINT (15.796760167739626 69.05714853429157)",
              "GEOMETRYCOLLECTION (LINESTRING(15.796760167740288 69.05714853429149,15.796760167739626 69.05714853429157), LINESTRING(15.796760167739626 69.05714853429157,15.795906966300288 69.05725770093837))");
}

template<>
template<>
void object::test<24>()
{
    set_test_name("Split line by point on the line boundary; PostGIS test 2");

    testSplit("LINESTRING(0 0, 10 0)",
              "POINT(10 0)",
              "GEOMETRYCOLLECTION(LINESTRING(0 0,10 0))");
}

template<>
template<>
void object::test<25>()
{
    set_test_name("split line by point on the line exterior; PostGIS test 3");

    testSplit("LINESTRING(0 0, 10 0)",
              "POINT(5 1)", 
              "GEOMETRYCOLLECTION(LINESTRING(0 0,10 0))");
}

template<>
template<>
void object::test<26>()
{
    set_test_name("split line by disjoint line; PostGIS test 4");

    testSplit("LINESTRING(0 0, 10 0)",
              "LINESTRING (20 0, 20 20)",
              "GEOMETRYCOLLECTION(LINESTRING(0 0,10 0))");
}

template<>
template<>
void object::test<27>()
{
    set_test_name("split line by touching line; PostGIS test 5");

    testSplit("LINESTRING(0 0, 10 0)",
              "LINESTRING (10 -5, 10 5)",
              "GEOMETRYCOLLECTION(LINESTRING(0 0,10 0))");
}

template<>
template<>
void object::test<28>()
{
    set_test_name("Split line by crossing line; PostGIS test 6");

    testSplit("LINESTRING(0 0, 10 0)",
              "LINESTRING(5 -5, 5 5)",
              "GEOMETRYCOLLECTION(LINESTRING(5 0,10 0),LINESTRING(0 0,5 0))");
}

template<>
template<>
void object::test<29>()
{
    set_test_name("Split line by multiply-crossing line; PostGIS test 7");

    testSplit("LINESTRING(0 0, 10 0, 10 10, 0 10, 0 20, 10 20)",
              "LINESTRING(5 -5, 5 25)",
              "GEOMETRYCOLLECTION(LINESTRING(5 10,0 10,0 20,5 20),LINESTRING(5 0,10 0,10 10,5 10),LINESTRING(5 20,10 20),LINESTRING(0 0,5 0))");
}

template<>
template<>
void object::test<30>()
{
    set_test_name("Split line by overlapping line (1); PostGIS test 8.1");

    // Note: PostGIS produces an error here instead of a result
    testSplit("LINESTRING(0 0, 10 0)",
              "LINESTRING(5 0, 20 0)",
              "GEOMETRYCOLLECTION (LINESTRING (0 0, 5 0), LINESTRING (5 0, 10 0))");
}

template<>
template<>
void object::test<31>()
{
    set_test_name("Split line by overlapping line (1); PostGIS test 8.2");

    // Note: PostGIS produces an error here instead of a result
    testSplit("LINESTRING(0 0, 10 0)",
              "LINESTRING(5 0, 8 0)",
              "GEOMETRYCOLLECTION (LINESTRING (0 0, 5 0), LINESTRING (5 0, 8 0), LINESTRING (8 0, 10 0))");
}

template<>
template<>
void object::test<32>()
{
    set_test_name("Split multiline by line crossing both; PostGIS test 30");

    testSplit("MULTILINESTRING((0 0, 10 0),(0 5, 10 5))",
              "LINESTRING(5 -5, 5 10)",
              "GEOMETRYCOLLECTION(LINESTRING(5 5,10 5),LINESTRING(5 0,10 0),LINESTRING(0 5,5 5),LINESTRING(0 0,5 0))");
}

template<>
template<>
void object::test<33>()
{
    set_test_name("Split multiline by line crossing only one of them; PostGIS test 31");

    testSplit("MULTILINESTRING((0 0, 10 0),(0 5, 10 5))",
              "LINESTRING(5 -5, 5 2)",
              "GEOMETRYCOLLECTION(LINESTRING(5 0,10 0),LINESTRING(0 5,10 5),LINESTRING(0 0,5 0))");
}

template<>
template<>
void object::test<34>()
{
    set_test_name("Split multiline by disjoint line; PostGIS test 32");

    testSplit("MULTILINESTRING((0 0, 10 0),(0 5, 10 5))",
              "LINESTRING(5 10, 5 20)",
              "GEOMETRYCOLLECTION(LINESTRING(0 5,10 5),LINESTRING(0 0,10 0))");

}

template<>
template<>
void object::test<35>()
{
    set_test_name("Split multiline by point on one of them; PostGIS test 40");

    testSplit("MULTILINESTRING((0 0, 10 0),(0 5, 10 5))",
              "POINT(5 0)",
              "GEOMETRYCOLLECTION(LINESTRING(5 0,10 0),LINESTRING(0 5,10 5),LINESTRING(0 0,5 0))");
}

template<>
template<>
void object::test<36>()
{
    set_test_name("Split geometrycollection by line; PostGIS test 60");

    testSplit("GEOMETRYCOLLECTION(MULTIPOLYGON(((0 0, 10 0, 10 10, 0 10, 0 0),(2 2, 8 2, 8 4, 2 4, 2 2),(2 6,8 6,8 8,2 8,2 6)),((20 0,20 10, 30 10, 30 0, 20 0),(25 5, 28 5, 25 8, 25 5))),MULTILINESTRING((0 0, 10 0),(0 5, 10 5)))",
              "LINESTRING(5 -5, 5 15)",
              "GEOMETRYCOLLECTION(POLYGON((5 0,5 2,8 2,8 4,5 4,5 6,8 6,8 8,5 8,5 10,10 10,10 0,5 0)),POLYGON((0 0,0 10,5 10,5 8,2 8,2 6,5 6,5 4,2 4,2 2,5 2,5 0,0 0)),POLYGON((20 0,20 10,30 10,30 0,20 0),(25 5,28 5,25 8,25 5)),LINESTRING(5 5,10 5),LINESTRING(5 0,10 0),LINESTRING(0 5,5 5),LINESTRING(0 0,5 0))");
}

template<>
template<>
void object::test<37>()
{
    set_test_name("Split 3d line by 2d line; PostGIS test 70");

    testSplit("LINESTRING(1691983.26 4874594.81 312.24, 1691984.86 4874593.69 312.24, 1691979.54 4874586.09 312.24, 1691978.03 4874587.16 298.36)",
              "LINESTRING(1691978.0 4874589.0,1691982.0 4874588.53, 1691982.0 4874591.0)",
              "GEOMETRYCOLLECTION(LINESTRING(1691982 4874589.604285714 312.24,1691984.86 4874593.69 312.24,1691983.26 4874594.81 312.24),LINESTRING(1691978.03 4874587.16 298.36,1691979.54 4874586.09 312.24,1691981.3051513054 4874588.611644722 312.24),LINESTRING(1691981.3051513054 4874588.611644722 312.24,1691982 4874589.604285714 312.24))");

}

template<>
template<>
void object::test<38>()
{
    set_test_name("Split collapsed line by point; PostGIS test 80");

    testSplit("LINESTRING(0 1, 0 1, 0 1)",
              "POINT(0 1)",
              "GEOMETRYCOLLECTION(LINESTRING(0 1,0 1,0 1))");

}

template<>
template<>
void object::test<39>()
{
    set_test_name("Split collapsed line by point; PostGIS test 81");
    
    testSplit("LINESTRING(0 1, 0 1)",
              "POINT(0 1)",
              "GEOMETRYCOLLECTION(LINESTRING(0 1,0 1))");
}

template<>
template<>
void object::test<40>()
{
    set_test_name("Split long line by vertex point; PostGIS test 82");

    geos::io::WKBReader wkbReader;
    auto input = wkbReader.readHEX("01020000001000000034030F8FB15866C0F2311FFD3B9A53C0571C87CF1BB65BC0182DB847DB9052C0EBD57BDEEBF658C05CA18B9FA81B52C074384E71C20552C05AD308B7C38351C0A4B3920AA7914CC0ACD200FB29784FC0F8892AEE70E14040C0C8143E325651C0234604DC104E5440EF10F2807BF850C08FEE52B6CAE15F4002BF1C6676B450C0051A57A65BB061405B9E445AEC9F50C05AF3E1D5815665405E3A4A2BB6CF51C0591DE7ECD21F66400D33BFE91C7E53C0000000E0FF7F6640000000C04E9353C0000000000080664000000000008056C000000000008066C000000000008056C000000000008066C0000000E04D9353C034030F8FB15866C0F2311FFD3B9A53C0");

    auto splitPoint = geos::detail::down_cast<const LineString*>(input.get())->getPointN(14);

    auto split = GeometrySplitter::split(*input, *splitPoint);

    ensure(split->Union()->equals(input.get()));
}

template<>
template<>
void object::test<41>()
{
    set_test_name("Split line by multiline; PostGIS test 83");

    testSplit("LINESTRING(1 -1,1 1)",
              "MULTILINESTRING((10 0, 10 4),(-4 0, 4 0))",
              "GEOMETRYCOLLECTION(LINESTRING(1 -1,1 0),LINESTRING(1 0,1 1))");

}

template<>
template<>
void object::test<42>()
{
    set_test_name("Split line by polygon (boundary); PostGIS test 84");

    testSplit("LINESTRING(1 -1,1 1)",
              "POLYGON((-10 -10,-10 10,10 10,10 -10,-10 -10),(-4 2,-4 0,4 0,4 2,-4 2))",
              "GEOMETRYCOLLECTION(LINESTRING(1 -1,1 0),LINESTRING(1 0,1 1))");

}

template<>
template<>
void object::test<43>()
{
    set_test_name("Split line by multipolygon (boundary); PostGIS test 85");

    testSplit("LINESTRING(1 -2,1 1,4 1)",
              "MULTIPOLYGON(((0 -1,0 -3,2 -3,2 -1,0 -1)),((3 0,3 2,5 2,5 0,3 0)))",
              "GEOMETRYCOLLECTION(LINESTRING(1 -2,1 -1),LINESTRING(1 -1,1 1,3 1),LINESTRING(3 1,4 1))");
}

template<>
template<>
void object::test<44>()
{
    set_test_name("Split multiline by multipoint; PostGIS test 86");

    testSplit("MULTILINESTRING((0 0,10 0),(5 -5, 5 5),(0 20,10 20))",
              "MULTIPOINT(2 6,5 0,5 20,2 20,8 20,8 0,5 -2,0 0, 5 -5, 10 20)",
              "GEOMETRYCOLLECTION(LINESTRING(8 0,10 0),LINESTRING(0 0,5 0),LINESTRING(5 0,8 0),LINESTRING(5 0,5 5),LINESTRING(5 -2,5 0),LINESTRING(5 -5,5 -2),LINESTRING(8 20,10 20),LINESTRING(2 20,5 20),LINESTRING(0 20,2 20),LINESTRING(5 20,8 20))");
}

template<>
template<>
void object::test<45>()
{
    set_test_name("Split empty line by point; PostGIS test 87");

    testSplit("LINESTRING EMPTY",
              "POINT(0 1)",
              "GEOMETRYCOLLECTION(LINESTRING EMPTY)");
}

template<>
template<>
void object::test<46>()
{
    set_test_name("LineStringZ with NaN values; PostGIS ticket #5635");

    // PostGIS throws error
    testSplit("LINESTRING Z (1 2 NaN,3 4 10,5 6 NaN)",
              "MULTIPOINT( EMPTY, (2 1) , (2 4) , (4 5))",
              "GEOMETRYCOLLECTION( LINESTRING Z (1 2 NaN, 3 4 10, 4 5 10), LINESTRING Z (4 5 10, 5 6 NaN))");
}

template<>
template<>
void object::test<47>()
{
    set_test_name("LineStringZ split by edge with NaN values; PostGIS ticket #5635");

    // PostGIS throws error
    // Instead, we ignore the invalid point
    testSplit("LINESTRING Z (1 2 1,3 4 10,5 6 3)",
              "MULTIPOINT(1 NaN,2 1,2 4, 4 5)",
              "GEOMETRYCOLLECTION( LINESTRING Z (1 2 1, 3 4 10, 4 5 6.5), LINESTRING Z (4 5 6.5, 5 6 3))");
}

template<>
template<>
void object::test<48>()
{
    set_test_name("split CircularString with line");

    testSplit("CIRCULARSTRING (-5 0, -4 3, 4 3)",
              "LINESTRING (0 0, 3 6)",
              "GEOMETRYCOLLECTION (CIRCULARSTRING (-5 0, -2.6286555605956674 4.2532540417602, 2.23606797749979 4.47213595499958), CIRCULARSTRING (2.23606797749979 4.47213595499958, 3.2037371962288375 3.8387586505926348, 4 3))");
}

template<>
template<>
void object::test<49>()
{
    set_test_name("split CurvePolygon with line");

    testSplit("CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING (-5 0, -4 3, 4 3), (4 3, -5 0)))",
              "LINESTRING (0 0, 3 6)",
              "GEOMETRYCOLLECTION ("
              "CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING (-5 0, -2.6286555605956674 4.2532540417602, 2.23606797749979 4.47213595499958), (2.23606797749979 4.47213595499958, 1 2, -5 0))),"
              "CURVEPOLYGON(COMPOUNDCURVE(CIRCULARSTRING (2.23606797749979 4.47213595499958, 3.2037371962288375 3.8387586505926348, 4 3), (4 3, 1 2, 2.23606797749979 4.47213595499958))))"
             );
}

template<>
template<>
void object::test<50>()
{
    set_test_name("split CircularString with point");

    testSplit("CIRCULARSTRING (-5 0, -4 3, 4 3)",
              "POINT (0 5)",
              "GEOMETRYCOLLECTION (CIRCULARSTRING (-5 0, -3.5355339059327373 3.5355339059327378, 0 5), CIRCULARSTRING (0 5, 2.2360679774997902 4.47213595499958, 4 3))");
}

template<>
template<>
void object::test<51>()
{
    set_test_name("split CompoundCurve with point on curve");

    testSplit("COMPOUNDCURVE(CIRCULARSTRING (-5 0, -4 3, 4 3), (4 3, 0 0))",
              "POINT (0 5)",
              "GEOMETRYCOLLECTION (CIRCULARSTRING (-5 0, -3.5355339059327373 3.5355339059327378, 0 5), COMPOUNDCURVE(CIRCULARSTRING (0 5, 2.2360679774997902 4.47213595499958, 4 3), (4 3, 0 0)))");
}

template<>
template<>
void object::test<52>()
{
    set_test_name("split CompoundCurve with point on line");

    testSplit("COMPOUNDCURVE(CIRCULARSTRING (-5 0, -4 3, 4 3), (4 3, 0 0))",
              "POINT (2 1.5)",
              "GEOMETRYCOLLECTION (COMPOUNDCURVE( CIRCULARSTRING (-5 0, -4 3, 4 3), (4 3, 2 1.5)), LINESTRING (2 1.5, 0 0))");
}

#if 0
template<>
template<>
void object::test<53>()
{
    set_test_name("nearly-collapsed Polygon (QGIS test #1)");
    // void TestQgsGeometry::splitGeometry()

    // QGIS and PostGIS gives "GEOMETRYCOLLECTION EMPTY" here.
    // Currently, GEOS is producing a result that is smaller than the original polygon.
    // Neither seems ideal?
    testSplit("POLYGON ((492980.38648063864093274 7082334.45244149677455425, 493082.65415841294452548 7082319.87918917648494244, 492980.38648063858272508 7082334.45244149677455425, 492980.38648063864093274 7082334.45244149677455425))",
              "LINESTRING (493825.46541286131832749 7082214.02779923938214779, 492955.04876351181883365 7082338.06309300474822521)",
              "GEOMETRYCOLLECTION EMPTY");
}
#endif

template<>
template<>
void object::test<54>()
{
    set_test_name("Z values of split edge are not used in interpolation; QGIS test #2");
    // See https://github.com/qgis/QGIS/issues/33489

    testSplit("COMPOUNDCURVE Z ((2749546.20 1262904.45 100, 2749557.82 1262920.06 200))",
              "LINESTRING Z (2749544.19 1262914.79 0, 2749557.64 1262897.30 0 )",
              "GEOMETRYCOLLECTION Z (LINESTRING Z (2749546.2 1262904.45 100, 2749549.122464944 1262908.3759619428 125.15030072306642), LINESTRING Z (2749549.122464944 1262908.3759619428 125.15030072306642, 2749557.82 1262920.06 200))");
}

template<>
template<>
void object::test<55>()
{
    set_test_name("split CompoundCurve at an existing vertex; QGIS test #3");

    testSplit("CompoundCurve ((1 1, 2 2, 3 3))",
              "POINT (2 2)",
              "GEOMETRYCOLLECTION(LINESTRING (1 1, 2 2), LINESTRING (2 2, 3 3))");
}

template<>
template<>
void object::test<56>()
{
    set_test_name("Split self-intersecting LineString at points; adaptation of QGIS test #4");

    // Do not split on self-intersections - https://github.com/qgis/QGIS/issues/14070
    testSplit("LINESTRING (0 0, 10 0, 10 2, 6 2, 6 -2, 3 -2, 3 2, 0 2, 0 0)",
              "MULTIPOINT (0 1, 3 -1, 3 1, 6 -1, 6 1, 10 1)",
              "GEOMETRYCOLLECTION (LINESTRING (0 0, 10 0, 10 1), LINESTRING (10 1, 10 2, 6 2, 6 1), LINESTRING (6 1, 6 -1), LINESTRING (6 -1, 6 -2, 3 -2, 3 -1), LINESTRING (3 -1, 3 1), LINESTRING (3 1, 3 2, 0 2, 0 1), LINESTRING (0 1, 0 0))");
}

#if 0
template<>
template<>
void object::test<57>()
{
    set_test_name("do not split on self-intersections; QGIS test #4");

    // Need to either
    // a) do as QGIS does, and use Geometry::intersection to get intersection points, then node on those
    // b) update GeometryNoder / SegmentIntersector classes with an ignoreSelfIntersections flag that would
    //    ignore PathStrings with the same context.

    // Do not split on self-intersections - https://github.com/qgis/QGIS/issues/14070
    testSplit("LINESTRING (0 0, 10 0, 10 2, 6 2, 6 -2, 3 -2, 3 2, 0 2, 0 0)",
              "LINESTRING (0 1, 11 1, 11 -1, 0 -1)",
              "GEOMETRYCOLLECTION (LINESTRING (0 0, 10 0, 10 1), LINESTRING (10 1, 10 2, 6 2, 6 1), LINESTRING (6 1, 6 -1), LINESTRING (6 -1, 6 -2, 3 -2, 3 -1), LINESTRING (3 -1, 3 1), LINESTRING (3 1, 3 2, 0 2, 0 1), LINESTRING (0 1, 0 0))");
}
#endif

template<>
template<>
void object::test<58>()
{
    set_test_name("do not split on self-intersections; QGIS test #5");

    // Do not split on self-intersections - https://github.com/qgis/QGIS/issues/14070
    testSplit("LINESTRING (0 0, 10 0, 10 2, 6 2, 6 -2, 3 -2, 3 2, 0 2, 0 0)",
              "POINT (6 2)",
              "GEOMETRYCOLLECTION (LINESTRING (0 0, 10 0, 10 2, 6 2), LINESTRING (6 2, 6 -2, 3 -2, 3 2, 0 2, 0 0))");
}

template<>
template<>
void object::test<59>()
{
    set_test_name("split LineString Z on existing vertex; QGIS test #6");

    // https://github.com/qgis/QGIS/issues/49403
    testSplit("LINESTRING Z (0 0 0, 1 1 1, 2 2 2)",
              "POINT (1 1)",
              "GEOMETRYCOLLECTION(LINESTRING Z (0 0 0, 1 1 1), LINESTRING Z (1 1 1, 2 2 2))");
}

template<>
template<>
void object::test<60>()
{
    // Should not crash - https://github.com/qgis/QGIS/issues/50948
    testSplit("LINESTRING ( -63294.10966012725839391 -79156.27234554117603693, -63290.25259721937618451 -79162.78533450335089583, -63290.25259721936890855 -79162.78533450335089583)",
              "LINESTRING (-63290.25259721936890855  -79165.28533450335089583, -63290.25259721936890855 -79160.28533450335089583)",
              "GEOMETRYCOLLECTION(LINESTRING ( -63294.10966012725839391 -79156.27234554117603693, -63290.25259721937618451 -79162.78533450335089583, -63290.25259721936890855 -79162.78533450335089583))");
}

template<>
template<>
void object::test<61>()
{
    // Should not split the first part - https://github.com/qgis/QGIS/issues/54155
    testSplit("MULTILINESTRING((0 1, 1 0), (0 2, 2 0))",
              "LINESTRING (0.8 0.8, 1.2 1.2)",
              "GEOMETRYCOLLECTION (LINESTRING (0 1, 1 0), LINESTRING (0 2, 1 1), LINESTRING (1 1, 2 0))");
}

template<>
template<>
void object::test<62>()
{
    set_test_name("cannot split Polygon with point");

    auto geom = reader_.read("POLYGON ((0 0, 10 0, 10 10, 0 10, 0 0))");
    auto splitGeom = reader_.read("POINT (5 0)");

    ensure_THROW(GeometrySplitter::split(*geom, *splitGeom), geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<63>()
{
    set_test_name("cannot split CurvePolygon with point");

    auto geom = reader_.read("CURVEPOLYGON (COMPOUNDCURVE (CIRCULARSTRING (-5 0, 0 5, 5 0), (5 0, -5 0)))");
    auto splitGeom = reader_.read("POINT (0 0)");

    ensure_THROW(GeometrySplitter::split(*geom, *splitGeom), geos::util::IllegalArgumentException);
}

template<>
template<>
void object::test<64>()
{
    set_test_name("split LineString with disjoint point");

    testSplit("LINESTRING (0 0, 10 0)",
              "POINT (5 1)",
              "GEOMETRYCOLLECTION (LINESTRING (0 0, 10 0))");
}

template<>
template<>
void object::test<65>()
{
    set_test_name("split LineString with empty point");

    testSplit("LINESTRING (0 0, 10 0)",
              "POINT EMPTY",
              "GEOMETRYCOLLECTION (LINESTRING (0 0, 10 0))");
}

template<>
template<>
void object::test<66>()
{
    set_test_name("split LineString with invalid point");

    testSplit("LINESTRING (0 0, 10 0)",
              "POINT (6 NaN)",
              "GEOMETRYCOLLECTION (LINESTRING (0 0, 10 0))");
}

}
