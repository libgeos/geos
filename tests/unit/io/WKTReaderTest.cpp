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
#include <geos/geom/LineString.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Point.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/util.h>
#include <geos/util/GEOSException.h>
#include <geos/util/IllegalArgumentException.h>
// std
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

    void ensure_dimension(const std::string & wkt, std::size_t dim) const {
        auto geom = wktreader.read(wkt);
        ensure_equals(wkt,
                      static_cast<std::size_t>(geom->getCoordinateDimension()),
                      dim);
    }

    void ensure_dimension(const std::string & wkt, bool has_z, bool has_m) const {
        auto geom = wktreader.read(wkt);
        ensure_equals(wkt + " hasZ", geom->hasZ(), has_z);
        ensure_equals(wkt + " hasM", geom->hasM(), has_m);
    }

    void ensure_parseexception(const std::string & wkt) const {
        try {
            auto geom = wktreader.read(wkt);
            fail();
        } catch (const geos::io::ParseException&) {}
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
    std::vector<std::string> variants{
        "POINT(-117 33 10)",
        "POINTZ(-117 33 10)",
        "POINT Z(-117 33 10)",
        "POINT Z (-117 33 10)"
    };

    for (const auto& wkt : variants) {
        GeomPtr geom(wktreader.read(wkt));
        auto coords = geom->getCoordinates();

        ensure(coords->hasZ());
        ensure(!coords->hasM());
        ensure(coords->getDimension() == 3);
        ensure(coords->getOrdinate(0, geos::geom::CoordinateSequence::Z) == 10.0);
    }
}

// 3 - Linestring dimension preserved.
template<>
template<>
void object::test<3>
()
{
    ensure_dimension("LINESTRING(-117 33, -116 34)", 2);
}

// 4 - Ensure we can read ZM geometries
template<>
template<>
void object::test<4>
()
{
    auto geom = wktreader.read<geos::geom::LineString>("LINESTRING ZM (-117 33 2 3, -116 34 4 5)");
    auto coords = geom->getCoordinatesRO();

    ensure_equals(coords->getDimension(), 4u);

    auto c0 = coords->getAt<geos::geom::CoordinateXYZM>(0);
    ensure_equals(c0.x, -117);
    ensure_equals(c0.y, 33);
    ensure_equals(c0.z, 2);
    ensure_equals(c0.m, 3);

    auto c1 = coords->getAt<geos::geom::CoordinateXYZM>(1);
    ensure_equals(c1.x, -116);
    ensure_equals(c1.y, 34);
    ensure_equals(c1.z, 4);
    ensure_equals(c1.m, 5);
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
    try {
        wktreader.read("POLYGON( EMPTY, (1 1,2 2,1 2,1 1))");
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
    wkt.emplace_back("MULTILINESTRING(");
    wkt.emplace_back("MULTIPOLYGON(");
    wkt.emplace_back("MULTIPOLYGON(EMPTY(");
    wkt.emplace_back("GEOMETRYCOLLECTION(");
    wkt.emplace_back("GEOMETRYCOLLECTION(LINEARRING(");
    for(const auto& i : wkt) {
        try {
            wktreader.read(i);
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
    ensure_dimension("POINT EMPTY", 2);
    ensure_dimension("POINTM EMPTY", 3);
    ensure_dimension("POINT M EMPTY", 3);
    ensure_dimension("POINTZ EMPTY", 3);
    ensure_dimension("POINT Z EMPTY", 3);
    ensure_dimension("POINTZM EMPTY", 4);
    ensure_dimension("POINT ZM EMPTY", 4);
    ensure_dimension("POINT Z M EMPTY", 4);

    ensure_dimension("LINESTRING EMPTY", 2);
    ensure_dimension("LINESTRINGM EMPTY", 3);
    ensure_dimension("LINESTRING M EMPTY", 3);
    ensure_dimension("LINESTRINGZ EMPTY", 3);
    ensure_dimension("LINESTRING Z EMPTY", 3);
    ensure_dimension("LINESTRINGZM EMPTY", 4);
    ensure_dimension("LINESTRING ZM EMPTY", 4);
    ensure_dimension("LINESTRING Z M EMPTY", 4);

    ensure_dimension("POLYGON EMPTY", 2);
    ensure_dimension("POLYGONM EMPTY", 3);
    ensure_dimension("POLYGON M EMPTY", 3);
    ensure_dimension("POLYGONZ EMPTY", 3);
    ensure_dimension("POLYGON Z EMPTY", 3);
    ensure_dimension("POLYGONZM EMPTY", 4);
    ensure_dimension("POLYGON ZM EMPTY", 4);
    ensure_dimension("POLYGON Z M EMPTY", 4);
}


// Raise an exception on WKT with mixed dimensionality
template<>
template<>
void object::test<10>
()
{
    ensure_parseexception("MULTIPOINT (1 1 1, 2 2)");
    ensure_parseexception("MULTIPOINT ((1 1 1), 2 2)");
    ensure_parseexception("MULTIPOINT (1 1, 2 2 2)");
    ensure_parseexception("MULTIPOINT ((1 1), (2 2 2))");
    ensure_parseexception("LINESTRING (1 1, 2 2 2)");
}

// Test typed variant of WKTReader::read
template<>
template<>
void object::test<11>
()
{
    // Correct type
    auto ls = wktreader.read<geos::geom::LineString>("LINESTRING (5 8, 5 7)");
    ensure(ls != nullptr);

    // Exception thrown on incorrect type
    try {
        auto poly = wktreader.read<geos::geom::LineString>("POINT (2 8)");
        fail();
    } catch (geos::util::GEOSException & e) {
        ensure_equals(std::string(e.what()), "ParseException: Unexpected WKT type");
    }

    // Malformed
    try {
        auto ps = wktreader.read<geos::geom::Point>("POINT (2, 8)");
        fail();
    } catch (geos::util::GEOSException & e) {
        ensure_equals(std::string(e.what()), "ParseException: Expected number but encountered ','");
    }

}

// Collection of empty
template<>
template<>
void object::test<12>
()
{
    auto geom1(wktreader.read("MULTIPOINT(EMPTY,  EMPTY)"));
    ensure("MULTIPOINT(EMPTY,  EMPTY)", geom1->getNumGeometries() == 2);
    ensure("MULTIPOINT(EMPTY,  EMPTY)", geom1->getGeometryN(1)->isEmpty());

    auto geom2(wktreader.read("MULTIPOINT((1 1), EMPTY)"));
    ensure("MULTIPOINT((1 1), EMPTY)", geom2->getNumGeometries() == 2);
    ensure("MULTIPOINT((1 1), EMPTY)", geom2->getGeometryN(1)->isEmpty());

    auto geom3(wktreader.read("MULTIPOINT(EMPTY, (1 1))"));
    ensure("MULTIPOINT( EMPTY, (1 1))", geom3->getNumGeometries() == 2);
    ensure("MULTIPOINT( EMPTY, (1 1))", geom3->getGeometryN(0)->isEmpty());
}

template<>
template<>
void object::test<13>
()
{
    wktreader.setFixStructure(true);
    auto geom = wktreader.read("POLYGON((0 0, 0 1, 1 1, 1 0))");
    std::unique_ptr<geos::geom::Polygon> p(geos::detail::down_cast<geos::geom::Polygon*>(geom.release()));
    ensure("setFixStructure", p->getExteriorRing()->getNumPoints() == 5);
}

// https://trac.osgeo.org/geos/ticket/676
template<>
template<>
void object::test<14>
()
{
    auto geom = wktreader.read("POINT M(1 2 3)");
    ensure_equals(geom->getCoordinateDimension(), 3u);

    geom = wktreader.read("POINTM(1 2 3)");
    ensure_equals(geom->getCoordinateDimension(), 3u);
}

// https://github.com/libgeos/geos/issues/669
template<>
template<>
void object::test<15>
()
{
    auto geom = wktreader.read("LINESTRINGZ(0 0 1, 1 1 1)");

    ensure_equals(geom->getCoordinateDimension(), 3u);
}

// Raise exception on dimensionality inconsistent with declared
template<>
template<>
void object::test<16>
()
{
    ensure_parseexception("POINTM(1 1)");
    ensure_parseexception("GEOMETRYCOLLECTION Z(POINT Z(0 0 0), LINESTRING M(1 1 1, 2 2 2))");
    ensure_parseexception("GEOMETRYCOLLECTION (POINT (0, 0, 0), POINT (0, 0, 0, 0)");
}

// Consistent mix of implicit and explicit dimensionality
template<>
template<>
void object::test<17>
()
{
    auto geom = wktreader.read("GEOMETRYCOLLECTION (POINT (3 3 3), POINTZ (4 4 9), POINT Z (2 8 2), POINT EMPTY)");

    ensure_equals(geom->getNumGeometries(), 4u);
}

// Inconsistent mix of implicit and explicit dimensionality
template<>
template<>
void object::test<18>
()
{
    // Implicit third dimension is always Z, so this geometry is inconsistent
    ensure_parseexception("GEOMETRYCOLLECTION M (POINT (2 0 8), POINTM (1 1 1), POINT M (3 2 7), POINT EMPTY)");
}

// Incorrect number of coordinates
template<>
template<>
void object::test<19>
()
{
    ensure_parseexception("POINT (3 8, 2 7");
}

// Mixed dimensionality within single-part geometry
template<>
template<>
void object::test<20>
()
{
    ensure_parseexception("POLYGON Z ((0 0,0 10,10 10,10 0,0 0),(1 1 1,1 2 1,2 2 1,2 1 1,1 1 1))");
    ensure_parseexception("POLYGON Z ((0 0,0 10,10 10,10 0,0 0),(1 1 1,1 2 1,2 2 1,2 1 1,1 1 1))");

    ensure_parseexception("LINESTRING Z (0 0 0 1, 0 1 0 1)");
}

// https://trac.osgeo.org/geos/ticket/1095
template<>
template<>
void object::test<21>
()
{
    try {
        auto geom = wktreader.read("GEOMETRYCOLLECTION(POINT (0 1)), POINT (1 1)");
        fail();
    } catch (geos::io::ParseException &e) {
        std::string msg(e.what());
        ensure_equals(msg, "ParseException: Unexpected text after end of geometry");
    }
}

// https://github.com/libgeos/geos/issues/886
template<>
template<>
void object::test<22>
()
{

    ensure_dimension("MULTIPOINT (0 0, 1 2)", false, false);
    ensure_dimension("MULTIPOINT Z (0 0 4, 1 2 4)", true, false);
    ensure_dimension("MULTIPOINT M (0 0 3, 1 2 5)", false, true);
    ensure_dimension("MULTIPOINT ZM (0 0 4 3, 1 2 4 5)", true, true);

    ensure_dimension("MULTIPOINT ((0 0), (1 2))", false, false);
    ensure_dimension("MULTIPOINT Z ((0 0 4), (1 2 4))", true, false);
    ensure_dimension("MULTIPOINT M ((0 0 3), (1 2 5))", false, true);
    ensure_dimension("MULTIPOINT ZM ((0 0 4 3), (1 2 4 5))", true, true);
}

// EMPTY token with some white space
template<>
template<>
void object::test<23>
()
{
    GeomPtr geom(wktreader.read("MULTIPOINT( EMPTY, (10 10), (20 20))"));

    ensure_equals(geom->getNumGeometries(), 3u);
}

} // namespace tut
