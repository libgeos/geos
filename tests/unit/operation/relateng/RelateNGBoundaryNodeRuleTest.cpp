//
// Test Suite for geos::operation::relateng Boundary Node Rules

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/algorithm/BoundaryNodeRule.h>
#include <geos/operation/relateng/RelateNG.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKTWriter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/IntersectionMatrix.h>
#include <geos/geom/Location.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::relateng;
using geos::io::WKTReader;
using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_relatengbnr_data {

    WKTReader r;
    WKTWriter w;

    void runRelate(
        const std::string& wkt1,
        const std::string& wkt2,
        const BoundaryNodeRule& bnRule,
        const std::string& expectedIM)
    {
        std::unique_ptr<Geometry> g1 = r.read(wkt1);
        std::unique_ptr<Geometry> g2 = r.read(wkt2);
        auto im = RelateNG::relate(g1.get(), g2.get(), bnRule);
        std::string imStr = im->toString();
        //System.out.println(imStr);
        if (imStr != expectedIM) {
            std::cerr << std::endl << w.write(*g1) << " relate " << bnRule.toString() << " " << w.write(*g2) << " = " << imStr << std::endl;
        }
        ensure_equals("runRelate", imStr, expectedIM);
    }


};

typedef test_group<test_relatengbnr_data> group;
typedef group::object object;

group test_relatengbnr_group("geos::operation::relateng::RelateNGBoundarNodeRule");

// BoundaryNodeRule::getBoundaryRuleMod2()
// BoundaryNodeRule::getBoundaryEndPoint()
// BoundaryNodeRule::getBoundaryMultivalentEndPoint()
// BoundaryNodeRule::getBoundaryMonovalentEndPoint()
// BoundaryNodeRule::getBoundaryOGCSFS()


// testMultiLineStringSelfIntTouchAtEndpoint
template<>
template<>
void object::test<1> ()
{
    std::string a = "MULTILINESTRING ((20 20, 100 100, 100 20, 20 100), (60 60, 60 140))";
    std::string b = "LINESTRING (60 60, 20 60)";

    // under EndPoint, A has a boundary node - A.bdy / B.bdy = 0
    runRelate(a, b, BoundaryNodeRule::getBoundaryEndPoint(),  "FF1F00102");
}

// testLineStringSelfIntTouchAtEndpoint
template<>
template<>
void object::test<2> ()
{
    std::string a = "LINESTRING (20 20, 100 100, 100 20, 20 100)";
    std::string b = "LINESTRING (60 60, 20 60)";

    // results for both rules are the same
    runRelate(a, b,  BoundaryNodeRule::getBoundaryOGCSFS(),   "F01FF0102");
    runRelate(a, b,  BoundaryNodeRule::getBoundaryEndPoint(),  "F01FF0102");
}

// testMultiLineStringTouchAtEndpoint
template<>
template<>
void object::test<3> ()
{
    std::string a = "MULTILINESTRING ((0 0, 10 10), (10 10, 20 20))";
    std::string b = "LINESTRING (10 10, 20 0)";

    // under Mod2, A has no boundary - A.int / B.bdy = 0
    // runRelateTest(a, b,  BoundaryNodeRule.OGC_SFS_BOUNDARY_RULE,   "F01FFF102");
    // under EndPoint, A has a boundary node - A.bdy / B.bdy = 0
    runRelate(a, b,  BoundaryNodeRule::getBoundaryEndPoint(),  "FF1F00102");
    // under MultiValent, A has a boundary node but B does not - A.bdy / B.bdy = F and A.int
    // runRelateTest(a, b,  BoundaryNodeRule.MULTIVALENT_ENDPOINT_BOUNDARY_RULE,  "0F1FFF1F2");
}

// testLineRingTouchAtEndpoints
// template<>
// template<>
// void object::test<4> ()
// {
//     std::string a = "LINESTRING (20 100, 20 220, 120 100, 20 100)";
//     std::string b = "LINESTRING (20 20, 20 100)";

//     // under Mod2, A has no boundary - A.int / B.bdy = 0
//     runRelate(a, b,  BoundaryNodeRule::getBoundaryOGCSFS(),   "F01FFF102");
//     // under EndPoint, A has a boundary node - A.bdy / B.bdy = 0
//     runRelate(a, b,  BoundaryNodeRule::getBoundaryEndPoint(),  "FF1F0F102");
//     // under MultiValent, A has a boundary node but B does not - A.bdy / B.bdy = F and A.int
//     runRelate(a, b,  BoundaryNodeRule::getBoundaryMonovalentEndPoint(),  "FF10FF1F2");
// }

// testLineRingTouchAtEndpointAndInterior
template<>
template<>
void object::test<5> ()
{
    std::string a = "LINESTRING (20 100, 20 220, 120 100, 20 100)";
    std::string b = "LINESTRING (20 20, 40 100)";

    // this is the same result as for the above test
    runRelate(a, b,  BoundaryNodeRule::getBoundaryOGCSFS(),   "F01FFF102");
    // this result is different - the A node is now on the boundary, so A.bdy/B.ext = 0
    runRelate(a, b,  BoundaryNodeRule::getBoundaryEndPoint(),  "F01FF0102");
}

// testPolygonEmptyRing
template<>
template<>
void object::test<6> ()
{
    std::string a = "POLYGON EMPTY";
    std::string b = "LINESTRING (20 100, 20 220, 120 100, 20 100)";

    // closed line has no boundary under SFS rule
    runRelate(a, b,  BoundaryNodeRule::getBoundaryOGCSFS(),   "FFFFFF1F2");

    // closed line has boundary under ENDPOINT rule
    runRelate(a, b,  BoundaryNodeRule::getBoundaryEndPoint(),  "FFFFFF102");
}

// testPolygonEmptyMultiLineStringClosed
template<>
template<>
void object::test<7> ()
{
    std::string a = "POLYGON EMPTY";
    std::string b = "MULTILINESTRING ((0 0, 0 1), (0 1, 1 1, 1 0, 0 0))";

    // closed line has no boundary under SFS rule
    runRelate(a, b,  BoundaryNodeRule::getBoundaryOGCSFS(),   "FFFFFF1F2");

    // closed line has boundary under ENDPOINT rule
    runRelate(a, b,  BoundaryNodeRule::getBoundaryEndPoint(),  "FFFFFF102");
}

// testLineStringInteriorTouchMultivalent
template<>
template<>
void object::test<8> ()
{
    std::string a = "POLYGON EMPTY";
    std::string b = "MULTILINESTRING ((0 0, 0 1), (0 1, 1 1, 1 0, 0 0))";

    // closed line has no boundary under SFS rule
    runRelate(a, b,  BoundaryNodeRule::getBoundaryOGCSFS(),   "FFFFFF1F2");

    // closed line has boundary under ENDPOINT rule
    runRelate(a, b,  BoundaryNodeRule::getBoundaryEndPoint(),  "FFFFFF102");
}




} // namespace tut
