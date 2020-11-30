//
// Test Suite for geos::io::WKTReader

// tut
#include <tut/tut.hpp>
// geos
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/util/IllegalArgumentException.h>
// std
#include <sstream>
#include <string>
#include <memory>

namespace tut {
//
// Test Group
//

// dummy data, not used
struct test_wktreader_data {
    geos::geom::PrecisionModel pm;
    geos::geom::GeometryFactory::Ptr gf;
    geos::io::WKTReader wktreader;
    geos::io::WKTWriter wktwriter;

    typedef std::unique_ptr<geos::geom::Geometry> GeomPtr;

    test_wktreader_data()
        :
        pm(1.0),
        gf(geos::geom::GeometryFactory::create(&pm)),
        wktreader(gf.get())
    {
        wktwriter.setOutputDimension(3);
    }

};

typedef test_group<test_wktreader_data> group;
typedef group::object object;

group test_wktreader_group("geos::io::WKTReader");


//
// Test Cases
//

// 1 - Read a point, confirm 2D.
template<>
template<>
void object::test<1>
()
{
    GeomPtr geom(wktreader.read("POINT(-117 33)"));
    auto coords = geom->getCoordinates();

    ensure(coords->getDimension() == 2);
    ensure(coords->getX(0) == -117);
    ensure(coords->getY(0) == 33);
}

// 2 - Read a point, confirm 3D.
template<>
template<>
void object::test<2>
()
{
    GeomPtr geom(wktreader.read("POINT(-117 33 10)"));
    auto coords = geom->getCoordinates();

    ensure(coords->getDimension() == 3);
    ensure(coords->getOrdinate(0, geos::geom::CoordinateSequence::Z) == 10.0);
}

// 3 - Linestring dimension preserved.
template<>
template<>
void object::test<3>
()
{
    GeomPtr geom(wktreader.read("LINESTRING(-117 33, -116 34)"));
    auto coords = geom->getCoordinates();

    ensure(coords->getDimension() == 2);
}

// 4 - Ensure we can read ZM geometries, just discarding the M.
template<>
template<>
void object::test<4>
()
{
    GeomPtr geom(wktreader.read("LINESTRING ZM (-117 33 2 3, -116 34 4 5)"));
    auto coords = geom->getCoordinates();

    ensure(coords->getDimension() == 3);

    ensure_equals(wktwriter.write(geom.get()),
                  std::string("LINESTRING Z (-117 33 2, -116 34 4)"));
}

// 5 - Check support for mixed case keywords (and old style 3D)
template<>
template<>
void object::test<5>
()
{
    GeomPtr geom(wktreader.read("LineString (-117 33 2, -116 34 4)"));
    ensure_equals(wktwriter.write(geom.get()),
                  std::string("LINESTRING Z (-117 33 2, -116 34 4)"));
}

// 6 - invalid WKT (see http://trac.osgeo.org/geos/ticket/361)
template<>
template<>
void object::test<6>
()
{
    GeomPtr geom;

    try {
        geom = wktreader.read("POLYGON( EMPTY, (1 1,2 2,1 2,1 1))");
        fail("Did not get expected exception");
    }
    catch(const geos::util::IllegalArgumentException& ex) {
        ensure("Got expected exception", true);
        (void)(ex.what());
    }
    catch(...) {
        fail("Got unexpected exception");
    }
}

// POINT(0 0) http://trac.osgeo.org/geos/ticket/610
template<>
template<>
void object::test<7>
()
{
    GeomPtr geom;

    try {
        // use FLOATING model
        namespace ggm = geos::geom;
        namespace gio = geos::io;
        ggm::PrecisionModel p_pm(ggm::PrecisionModel::FLOATING);
        ggm::GeometryFactory::Ptr p_gf = ggm::GeometryFactory::create(&p_pm);
        gio::WKTReader wktReader(p_gf.get());
        const std::string str = " POINT (0 0) ";
        geom = wktReader.read(str); //HERE IT FAILS

        auto coords = geom->getCoordinates();
        ensure_equals(coords->getDimension(), 2U);
        ensure_distance(coords->getX(0), 0.0, 1e-12);
        ensure_distance(coords->getY(0), 0.0, 1e-12);
    }
    catch(const geos::util::IllegalArgumentException& ex) {
        ensure("Got expected exception", true);
        (void)(ex.what());
    }
    catch(...) {
        fail("Got unexpected exception");
    }
}

// 8 - invalid WKT (see https://trac.osgeo.org/geos/ticket/830)
template<>
template<>
void object::test<8>
()
{
    // All of these strings cause an exception during read().
    std::vector<std::string> wkt;
    wkt.push_back("MULTILINESTRING(");
    wkt.push_back("MULTIPOLYGON(");
    wkt.push_back("MULTIPOLYGON(EMPTY(");
    wkt.push_back("GEOMETRYCOLLECTION(");
    wkt.push_back("GEOMETRYCOLLECTION(LINEARRING(");
    for(size_t i = 0; i < wkt.size(); i++) {
        try {
            wktreader.read(wkt[i]);
            fail("Didn't get expected exception");
        }
        catch(...) {
            ensure("Did get expected exception", true);
        }
    }
}

// Correctly read higher dimensional empty
template<>
template<>
void object::test<9>
()
{
    auto geom1(wktreader.read("POINT EMPTY"));
    ensure("dimension(POINT EMPTY) == 2", geom1->getCoordinateDimension() == 2);

    auto geom2(wktreader.read("POINT Z EMPTY"));
    ensure("dimension(POINT Z EMPTY) == 3", geom2->getCoordinateDimension() == 3);

    auto geom3(wktreader.read("LINESTRING EMPTY"));
    ensure("dimension(LINESTRING EMPTY) == 2", geom3->getCoordinateDimension() == 2);

    auto geom4(wktreader.read("LINESTRING Z EMPTY"));
    ensure("dimension(LINESTRING Z EMPTY) == 3", geom4->getCoordinateDimension() == 3);

    auto geom5(wktreader.read("POLYGON EMPTY"));
    ensure("dimension(POLYGON EMPTY) == 2", geom5->getCoordinateDimension() == 2);

    auto geom6(wktreader.read("POLYGON Z EMPTY"));
    ensure("dimension(POLYGON Z EMPTY) == 3", geom6->getCoordinateDimension() == 3);
}


// Handle WKT with mixed dimensionality in
// coordinate sequence. This is the old behaviour, wherein
// the first coordinate of a coordinate sequence dictates the
// dimensionality of the following coordinates. This ignores
// dimensionality tags (Z/M). It also has strange behaviour
// in the multipoint case, but we leave this unchanged for now
// as this test is being written just prior to 3.9 release.
template<>
template<>
void object::test<10>
()
{
    GeomPtr geom;
    geom = wktreader.read("MULTIPOINT (1 1, 2 2)");
    ensure("dimension(MULTIPOINT (1 1, 2 2)) == 2", geom->getCoordinateDimension() == 2);

    geom = wktreader.read("LINESTRING (1 1, 2 2)");
    ensure("dimension(LINESTRING (1 1, 2 2)) == 2", geom->getCoordinateDimension() == 2);

    geom = wktreader.read("MULTIPOINT (1 1 1, 2 2)");
    ensure("dimension(MULTIPOINT (1 1 1, 2 2)) == 3", geom->getCoordinateDimension() == 3);

    geom = wktreader.read("MULTIPOINT (1 1, 2 2 2)");
    ensure("dimension(MULTIPOINT (1 1, 2 2 2)) == 3", geom->getCoordinateDimension() == 3);

    geom = wktreader.read("LINESTRING (1 1 1, 2 2)");
    ensure("dimension(LINESTRING (1 1 1, 2 2)) == 3", geom->getCoordinateDimension() == 3);

    geom = wktreader.read("LINESTRING (1 1, 2 2 2)");
    ensure("dimension(LINESTRING (1 1, 2 2 2)) == 2", geom->getCoordinateDimension() == 2);

    geom = wktreader.read("POLYGON ((0 0, 1 0, 1 1 1, 0 1, 0 0))");
    ensure("dimension(POLYGON ((0 0, 1 0, 1 1 1, 0 1, 0 0)) == 2", geom->getCoordinateDimension() == 2);
}

} // namespace tut
