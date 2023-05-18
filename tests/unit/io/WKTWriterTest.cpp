//
// Test Suite for geos::io::WKTWriter

// tut
#include <tut/tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Point.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryCollection.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/util.h>
// std
#include <sstream>
#include <string>
#include <memory>

namespace tut {
//
// Test Group
//

using geos::geom::CoordinateSequence;
using geos::geom::Coordinate;
using geos::geom::CoordinateXY;
using geos::geom::CoordinateXYM;
using geos::geom::CoordinateXYZM;

// dummy data, not used
struct test_wktwriter_data {
    typedef geos::geom::PrecisionModel PrecisionModel;
    typedef geos::geom::GeometryFactory GeometryFactory;
    typedef geos::geom::Geometry Geometry;
    typedef geos::geom::GeometryCollection GeometryCollection;
    typedef geos::io::WKTReader WKTReader;
    typedef geos::io::WKTWriter WKTWriter;
    typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;

    PrecisionModel pm;
    GeometryFactory::Ptr gf;
    WKTReader wktreader;
    WKTWriter wktwriter;

    test_wktwriter_data()
        :
        pm(1000.0),
        gf(GeometryFactory::create(&pm)),
        wktreader(gf.get())
    {}

};

typedef test_group<test_wktwriter_data> group;
typedef group::object object;

group test_wktwriter_group("geos::io::WKTWriter");


//
// Test Cases
//

// 1 - Test the trim capability.
template<>
template<>
void object::test<1>
()
{
    GeomPtr geom(wktreader.read("POINT(-117 33)"));
    std::string  result;

    wktwriter.setTrim(false);
    result = wktwriter.write(geom.get());

    ensure_equals(result, "POINT (-117.000 33.000)");

    wktwriter.setTrim(true);
    result = wktwriter.write(geom.get());

    ensure_equals(result, "POINT (-117 33)");
}

// 2 - Test the output precision capability
template<>
template<>
void object::test<2>
()
{
    GeomPtr geom(wktreader.read("POINT(-117.1234567 33.1234567)"));
    std::string  result;

    wktwriter.setTrim(false);
    result = wktwriter.write(geom.get());
    ensure_equals(result, "POINT (-117.123 33.123)");

    wktwriter.setRoundingPrecision(2);
    result = wktwriter.write(geom.get());
    ensure_equals(result, "POINT (-117.12 33.12)");

    geom = wktreader.read("POINT(-117.000001 33.000001)");
    wktwriter.setRoundingPrecision(2);
    wktwriter.setTrim(true);
    result = wktwriter.write(geom.get());
    ensure_equals(result, "POINT (-117 33)");

    geom = wktreader.read("POINT(-0.000001 -33.000001)");
    result = wktwriter.write(geom.get());
    ensure_equals(result, "POINT (0 -33)");

    geom = wktreader.read("POINT(-10000000.000001 -100000033.000001)");
    result = wktwriter.write(geom.get());
    ensure_equals(result, "POINT (-10000000 -100000033)");

}

// 3 - Test 3D generation from a 3D geometry.
template<>
template<>
void object::test<3>
()
{
    GeomPtr geom(wktreader.read("POINT Z (-117 33 120)"));
    std::string  result;

    wktwriter.setOutputDimension(3);
    wktwriter.setTrim(true);
    wktwriter.setOld3D(false);

    result = wktwriter.write(geom.get());

    ensure_equals(result, std::string("POINT Z (-117 33 120)"));

    wktwriter.setOld3D(true);
    result = wktwriter.write(geom.get());

    ensure_equals(result, std::string("POINT (-117 33 120)"));

}

// 4 - Test 2D generation from a 3D geometry.
template<>
template<>
void object::test<4>
()
{
    GeomPtr geom(wktreader.read("POINT(-117 33 120)"));
    std::string  result;

    wktwriter.setOutputDimension(2);
    wktwriter.setTrim(true);
    wktwriter.setOld3D(false);

    result = wktwriter.write(geom.get());

    ensure_equals(result, std::string("POINT (-117 33)"));
}

// 5 - Test negative number of digits in precision model
template<>
template<>
void object::test<5>
()
{
    PrecisionModel pm3(0.001);
    GeometryFactory::Ptr gf3(GeometryFactory::create(&pm3));
    WKTReader wktreader3(gf3.get());
    GeomPtr geom(wktreader3.read("POINT(123456 654321)"));

    std::string  result = wktwriter.write(geom.get());
    ensure_equals(result, std::string("POINT (123000 654000)"));
}


// 6 - Test writing out a multipoint with an empty member
template<>
template<>
void object::test<6>
()
{
    PrecisionModel pm3(PrecisionModel::FLOATING);
    GeometryFactory::Ptr gf3(GeometryFactory::create(&pm3));
    std::unique_ptr<Geometry> empty_point(gf3->createPoint());
    ensure(empty_point != nullptr);

    geos::geom::Coordinate coord(1, 2);
    std::unique_ptr<Geometry> point(gf3->createPoint(coord));
    ensure(point != nullptr);

    std::vector<const Geometry*> geoms{empty_point.get(), point.get()};
    std::unique_ptr<Geometry> col(gf3->createMultiPoint(geoms));
    ensure(col != nullptr);

    ensure(col->getCoordinate() != nullptr);
    ensure_equals(col->getCoordinate()->x, 1);
    ensure_equals(col->getCoordinate()->y, 2);

    wktwriter.setRoundingPrecision(2);
    wktwriter.setTrim(true);
    std::string result = wktwriter.write(col.get());
    ensure_equals(result, std::string("MULTIPOINT (EMPTY, (1 2))"));
}


template<>
template<>
void object::test<7>
()
{
    using geos::geom::Coordinate;
    using geos::geom::CoordinateSequence;
    using geos::geom::Point;

    PrecisionModel pmLocal;
    auto factory_ = GeometryFactory::create(&pmLocal);
    auto coords = geos::detail::make_unique<CoordinateSequence>();
    ensure(coords != nullptr);

    coords->add(Coordinate(geos::DoubleNotANumber, geos::DoubleNotANumber));
    auto point = factory_->createPoint(std::move(coords));

    std::string result = wktwriter.write(point.get());
    ensure_equals(result, std::string("POINT EMPTY"));
}


// 5 - Test negative number of digits in precision model
template<>
template<>
void object::test<8>
()
{
    const char* gctxt = "GEOMETRYCOLLECTION (LINESTRING EMPTY, POLYGON EMPTY)";
    PrecisionModel gcpm;
    GeometryFactory::Ptr gcgf(GeometryFactory::create(&gcpm));
    WKTReader gcwktreader(gcgf.get());
    GeomPtr gcgeom(gcwktreader.read(gctxt));
    std::string result = wktwriter.write(gcgeom.get());
    ensure_equals(result, std::string(gctxt));
}

// Test writing XYZM
template<>
template<>
void object::test<9>
()
{
    auto coords = geos::detail::make_unique<CoordinateSequence>(2u, true, true);
    coords->setAt(CoordinateXYZM(1, 2, 3, 4), 0);
    coords->setAt(CoordinateXYZM(5, 6, 7, 8), 1);

    auto ls = gf->createLineString(std::move(coords));

    wktwriter.setTrim(true);
    wktwriter.setOutputDimension(4);

    ensure_equals(wktwriter.write(*ls),
                  std::string("LINESTRING ZM (1 2 3 4, 5 6 7 8)"));

    wktwriter.setOld3D(true);
    ensure_equals(wktwriter.write(*ls),
                  std::string("LINESTRING (1 2 3 4, 5 6 7 8)"));


    // If only 3 dimensions are allowed we pick Z instead of M
    wktwriter.setOld3D(false);
    wktwriter.setOutputDimension(3);

    ensure_equals(wktwriter.write(*ls),
                  std::string("LINESTRING Z (1 2 3, 5 6 7)"));

    wktwriter.setOld3D(true);
    ensure_equals(wktwriter.write(*ls),
                  std::string("LINESTRING (1 2 3, 5 6 7)"));
}

// Test writing XYM
template<>
template<>
void object::test<10>
()
{
    auto coords = geos::detail::make_unique<CoordinateSequence>(2u, false, true);
    coords->setAt(CoordinateXYM(1, 2, 3), 0);
    coords->setAt(CoordinateXYM(4, 5, 6), 1);

    auto ls = gf->createLineString(std::move(coords));

    wktwriter.setTrim(true);
    wktwriter.setOutputDimension(3);

    ensure_equals(wktwriter.write(*ls),
                  std::string("LINESTRING M (1 2 3, 4 5 6)"));

    // Same output
    wktwriter.setOld3D(true);
    ensure_equals(wktwriter.write(*ls),
                  std::string("LINESTRING M (1 2 3, 4 5 6)"));
}

// Test writing XY
template<>
template<>
void object::test<11>
()
{
    auto coords = geos::detail::make_unique<CoordinateSequence>(2u, false, false);
    coords->setAt(CoordinateXY(1, 2), 0);
    coords->setAt(CoordinateXY(3, 4), 1);

    auto ls = gf->createLineString(std::move(coords));

    wktwriter.setTrim(true);

    ensure_equals(wktwriter.write(*ls),
                  std::string("LINESTRING (1 2, 3 4)"));

    // Same output
    wktwriter.setOld3D(true);
    ensure_equals(wktwriter.write(*ls),
                  std::string("LINESTRING (1 2, 3 4)"));
}

// Test writing 3D/4D EMPTY geometries
// https://trac.osgeo.org/geos/ticket/1129
template<>
template<>
void object::test<12>
()
{
    wktwriter.setOutputDimension(4);

    CoordinateSequence coords_xyz(0u, true, false);
    CoordinateSequence coords_xym(0u, false, true);
    CoordinateSequence coords_xyzm(0u, true, true);

    auto pt_xyz = gf->createPoint(coords_xyz.clone());
    auto pt_xym = gf->createPoint(coords_xym.clone());
    auto pt_xyzm = gf->createPoint(coords_xyzm.clone());

    ensure_equals(wktwriter.write(*pt_xyz), std::string("POINT Z EMPTY"));
    ensure_equals(wktwriter.write(*pt_xym), std::string("POINT M EMPTY"));
    ensure_equals(wktwriter.write(*pt_xyzm), std::string("POINT ZM EMPTY"));

    auto ls_xyz = gf->createLineString(coords_xyz.clone());
    auto ls_xym = gf->createLineString(coords_xym.clone());
    auto ls_xyzm = gf->createLineString(coords_xyzm.clone());

    ensure_equals(wktwriter.write(*ls_xyz), std::string("LINESTRING Z EMPTY"));
    ensure_equals(wktwriter.write(*ls_xym), std::string("LINESTRING M EMPTY"));
    ensure_equals(wktwriter.write(*ls_xyzm), std::string("LINESTRING ZM EMPTY"));

    auto lr_xyz = gf->createLinearRing(coords_xyz.clone());
    auto lr_xym = gf->createLinearRing(coords_xym.clone());
    auto lr_xyzm = gf->createLinearRing(coords_xyzm.clone());

    auto poly_xyz = gf->createPolygon(std::move(lr_xyz));
    auto poly_xym = gf->createPolygon(std::move(lr_xym));
    auto poly_xyzm = gf->createPolygon(std::move(lr_xyzm));

    ensure_equals(wktwriter.write(*poly_xyz), std::string("POLYGON Z EMPTY"));
    ensure_equals(wktwriter.write(*poly_xym), std::string("POLYGON M EMPTY"));
    ensure_equals(wktwriter.write(*poly_xyzm), std::string("POLYGON ZM EMPTY"));
}

// Test writing an explicitly-created XYZ geometry where Z is NaN
// https://github.com/libgeos/geos/issues/808
template<>
template<>
void object::test<13>
()
{
    wktwriter.setOutputDimension(3);
    wktwriter.setTrim(true);

    CoordinateSequence xyz(1, true, false);
    xyz.setAt(Coordinate(1, 2, std::numeric_limits<double>::quiet_NaN()), 0);
    auto pt = gf->createPoint(std::move(xyz));

    ensure_equals(wktwriter.write(*pt), std::string("POINT Z (1 2 NaN)"));

    wktwriter.setRemoveEmptyDimensions(true);

    ensure_equals(wktwriter.write(*pt), std::string("POINT (1 2)"));
}

// Test removal of empty dimensions
template<>
template<>
void object::test<14>
()
{
    wktwriter.setOutputDimension(4);
    wktwriter.setTrim(true);

    auto g = wktreader.read("LINESTRING ZM (1 2 NaN 3, 4 5 NaN NaN)");

    ensure_equals(wktwriter.write(*g), "LINESTRING ZM (1 2 NaN 3, 4 5 NaN NaN)");

    wktwriter.setRemoveEmptyDimensions(true);

    ensure_equals(wktwriter.write(*g), "LINESTRING M (1 2 3, 4 5 NaN)");
}

} // namespace tut

