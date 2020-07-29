//
// Test Suite for geos::operation::OverlayOp class for UNION

#include <tut/tut.hpp>
// geos
#include <geos/operation/overlay/OverlayOp.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/io/WKBReader.h>
#include <geos/io/WKTReader.h>
// std
#include <string>
#include <sstream>
#include <memory>

using namespace geos::geom;
using namespace geos::operation;

namespace tut {
//
// Test Group
//

struct test_overlayopunion_data {
    typedef geos::geom::Geometry::Ptr GeometryPtr;
    typedef geos::geom::GeometryFactory GeometryFactory;
    typedef geos::geom::GeometryFactory::Ptr GeometryFactoryPtr;
};

typedef test_group<test_overlayopunion_data> group;
typedef group::object object;

group test_overlayopunion_group("geos::operation::overlay::OverlayOpUnion");

//
// Test Cases
//

// 1 - Union four connected segments of a square
template<>
template<>
void object::test<1>
()
{
    GeometryFactoryPtr factory = geos::geom::GeometryFactory::create();
    geos::io::WKTReader reader(*factory);
    GeometryPtr line1(reader.read("LINESTRING(0 0, 0 5)"));
    GeometryPtr line2(reader.read("LINESTRING(0 5, 5 5)"));
    GeometryPtr line3(reader.read("LINESTRING(5 5, 5 0)"));
    GeometryPtr line4(reader.read("LINESTRING(5 0, 0 0)"));
    GeometryPtr expect(reader.read("LINESTRING(0 0,0 5,5 5,5 0,0 0)"));

    // union segments incrementally
    GeometryPtr lines12(line1->Union(line2.get()));
    GeometryPtr lines123(lines12->Union(line3.get()));
    GeometryPtr lines1234(lines123->Union(line4.get()));

    ensure(expect->contains(lines1234.get()));

    // NOTE: Since the union operation makes no effort to simplify and
    // drop nodes of degree 2 from the built topology,
    // do not expect GEOS_LINESTRING.
    // See LineMergerTest where the test triangle is generated as a single LineString.
}

// https://trac.osgeo.org/geos/ticket/523
template<>
template<>
void object::test<2>
()
{
    std::istringstream wkb1("010300000001000000080000000000000087523A41000000402C625241000"
                            "00000B0523A41000000C01E6252410000000084523A41000000C023625241"
                            "858C4F2488523A4151F8EEAC2562524111A0F52288523A414F50F3AC25625"
                            "241D59FF52288523A414F50F3AC25625241000000007F523A410000008025"
                            "6252410000000087523A41000000402C625241");

    std::istringstream wkb2("01030000000100000004000000A871502388523A4158FEF2AC2562524100A"
                            "0F52288523A414F50F3AC2562524111A0F52288523A414F50F3AC25625241"
                            "A871502388523A4158FEF2AC25625241");

    geos::io::WKBReader reader;

    auto g1 = reader.readHEX(wkb1);
    auto g2 = reader.readHEX(wkb2);

    ensure(g1->isValid());
    ensure(g2->isValid());

    auto g3 = g1->symDifference(g2.get());

    ensure(g3->isValid());
}

} // namespace tut
