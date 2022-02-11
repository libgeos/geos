//
// Test Suite for disagreement between Intersects and PreparedIntersects

// tut
#include <tut/tut.hpp>
#include <utility.h>
// geos
#include <geos/geom/prep/PreparedGeometryFactory.h>
#include <geos/geom/prep/PreparedGeometry.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/io/WKBReader.h>
// std

#include <memory>

using geos::geom::Geometry;
using geos::geom::GeometryFactory;
using geos::geom::prep::PreparedGeometry;
using geos::geom::prep::PreparedGeometryFactory;
using geos::io::WKBReader;

namespace tut {

//
// Test Group
//

struct test_preparedgeometryintersects_data {
    typedef geos::geom::GeometryFactory GeometryFactory;

    GeometryFactory::Ptr factory;
    WKBReader reader;
    std::unique_ptr<Geometry> g1;
    std::unique_ptr<Geometry> g2;
    std::unique_ptr<PreparedGeometry> pg1;
    std::unique_ptr<PreparedGeometry> pg2;

    test_preparedgeometryintersects_data()
        : factory(GeometryFactory::create())
        , reader(*factory)
        , g1(nullptr)
        , g2(nullptr)
        , pg1(nullptr)
        , pg2(nullptr)
    {}
};

typedef test_group<test_preparedgeometryintersects_data> group;
typedef group::object object;

group test_preparedgeometryintersects_data("geos::geom::prep::PreparedGeometry::Intersects");

//
// Test Cases
//

//
// User found geometries for which prepared and ordinary intersects
// test return disagreeing results.
//
// https://lists.osgeo.org/pipermail/postgis-users/2022-February/045264.html
// https://github.com/libgeos/geos/issues/565
//
template<>
template<>
void object::test<1>
()
{
    std::stringstream hexwkb1("01020000200912000007000000642F25DC75A24CC0E4DE5740FCB34840A7CEFE9B72A24CC09DA85B2CFBB34840B5519D0E64A24CC091FAA188FBB34840FA449E245DA24CC054C2137AFDB34840F4ACFFCE51A24CC09FEB562A03B448405328C1D144A24CC09A3DD00A0CB44840404C10C03CA24CC0EA07FE6910B44840");
    g1 = reader.readHEX(hexwkb1);

    std::stringstream hexwkb2("0102000020091200000E00000004BE47A23CA24CC098A1F14410B448409871AEBC3FA24CC078341F2114B448400858AB764DA24CC09D0546031DB448406BFD3E2D50A24CC0BEDDEDD522B4484004824AA654A24CC02DC9A60128B44840EE377FB850A24CC0FA18BD642DB44840CCAF8B474EA24CC02CCCE78134B44840D7158E7B4EA24CC01D7C17A53AB44840ACFA01B452A24CC02688BA0F40B44840DB508C8752A24CC006CDF80846B44840A1F31ABB44A24CC0C891730756B44840009AF7EE45A24CC06B7649415CB448408C2ECAC749A24CC0CE57248161B44840A74302A150A24CC07DD00E1368B44840");
    g2 = reader.readHEX(hexwkb2);

    pg1 = PreparedGeometryFactory::prepare(g1.get());
    pg2 = PreparedGeometryFactory::prepare(g2.get());

    // Prepared and non-prepared results should be same
    ensure("Prepared and non-prepared results should be same 1->2",
        pg1->intersects(g2.get()) == g1->intersects(g2.get()));
    ensure("Prepared and non-prepared results should be same 2->1",
        pg2->intersects(g1.get()) == g2->intersects(g1.get()));
    // Interects is commutative
    ensure("Interects is commutative p1->p2",
        pg1->intersects(g2.get()) == pg2->intersects(g1.get()));
    ensure("Interects is commutative g1->g2",
        g1->intersects(g2.get()) == g2->intersects(g1.get()));
}


} // namespace tut
