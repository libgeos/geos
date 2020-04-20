//
// Test Suite for geos::io::WKTWriter

// tut
#include <tut/tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/Point.h>
// std
#include <sstream>
#include <string>
#include <memory>

namespace tut {
//
// Test Group
//

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
    PrecisionModel pm_e(1000);
    GeometryFactory::Ptr gf_e(GeometryFactory::create(&pm_e, 0));

    std::unique_ptr<geos::geom::Point> empty_pt(gf_e->createPoint());
    ensure(empty_pt != nullptr);

    geos::geom::Coordinate coord(1, 2);
    std::unique_ptr<Geometry> point(gf_e->createPoint(coord));
    ensure(point != nullptr);

    std::vector<const geos::geom::Geometry*> geoms{empty_pt.get(), point.get()};

    std::unique_ptr<geos::geom::Geometry> col(gf_e->createMultiPoint(geoms));

    wktwriter.setRoundingPrecision(2);
    wktwriter.setTrim(true);
    std::string result = wktwriter.write(col.get());
    ensure_equals(result, std::string("MULTIPOINT (EMPTY, 1 2)"));
}

} // namespace tut

