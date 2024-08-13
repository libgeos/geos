//
// Test Suite for geos::operation::relateng::LinearBoundary class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/algorithm/BoundaryNodeRule.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Location.h>
#include <geos/io/WKTReader.h>
//#include <geos/io/WKTWriter.h>
#include <geos/operation/relateng/LinearBoundary.h>
#include <geos/operation/relateng/LineStringExtracter.h>

// std
#include <memory>

using namespace geos::geom;
using namespace geos::operation::relateng;
using geos::algorithm::BoundaryNodeRule;
using geos::io::WKTReader;
// using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_linearboundary_data {

    WKTReader r;
    // WKTWriter w;

    void
    checkLinearBoundary(const std::string& wkt, const BoundaryNodeRule& bnr, const std::string& wktBdyExpected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        auto lines = extractLines(*geom);
        LinearBoundary lb(lines, bnr);
        bool hasBoundaryExpected = wktBdyExpected.length() == 0 ? false : true;
        ensure_equals("hasBoundaryExpected == lb.hasBoundary", hasBoundaryExpected, lb.hasBoundary());

        checkBoundaryPoints(lb, *geom, wktBdyExpected);
    }

    void
    checkBoundaryPoints(LinearBoundary& lb, Geometry& geom, const std::string& wktBdyExpected)
    {
        std::set<CoordinateXY> bdySet;
        extractPoints(wktBdyExpected, bdySet);

        for (const CoordinateXY& p : bdySet) {
            ensure("checkBoundaryPoints1", lb.isBoundary(&p));
        }

        auto allPts = geom.getCoordinates();
        for (std::size_t i = 0; i < allPts->size(); i++) {
            CoordinateXY p;
            allPts->getAt(i, p);
            if (bdySet.find(p) == bdySet.end()) { // p not in bdySet
                ensure("checkBoundaryPoints2", !lb.isBoundary(&p));
            }
        }
    }

    void
    extractPoints(const std::string& wkt, std::set<CoordinateXY>& ptSet)
    {
        if (wkt.length() == 0) return;
        auto geom = r.read(wkt);
        auto pts = geom->getCoordinates();
        for (std::size_t i = 0; i < pts->size(); i++) {
            CoordinateXY p;
            pts->getAt(i, p);
            ptSet.insert(p);
        }
        return;
    }

    std::vector<const LineString*>
    extractLines(const Geometry& geom)
    {
        return LineStringExtracter::getLines(&geom);
    }

};

typedef test_group<test_linearboundary_data> group;
typedef group::object object;

group test_linearboundary_group("geos::operation::relateng::LinearBoundary");

//
// Test Cases
//

// testLineMod2
template<>
template<>
void object::test<1> ()
{
    checkLinearBoundary("LINESTRING (0 0, 9 9)",
        BoundaryNodeRule::getBoundaryRuleMod2(),
        "MULTIPOINT((0 0), (9 9))");
}

// testLines2Mod2
template<>
template<>
void object::test<2> ()
{
    checkLinearBoundary("MULTILINESTRING ((0 0, 9 9), (9 9, 5 1))",
        BoundaryNodeRule::getBoundaryRuleMod2(),
        "MULTIPOINT((0 0), (5 1))");
}

// testLines3Mod2
template<>
template<>
void object::test<3> ()
{
    checkLinearBoundary("MULTILINESTRING ((0 0, 9 9), (9 9, 5 1), (9 9, 1 5))",
        BoundaryNodeRule::getBoundaryRuleMod2(),
        "MULTIPOINT((0 0), (5 1), (1 5), (9 9))");
}

// testLines3Monvalent
template<>
template<>
void object::test<4> ()
{
    checkLinearBoundary("MULTILINESTRING ((0 0, 9 9), (9 9, 5 1), (9 9, 1 5))",
        BoundaryNodeRule::getBoundaryMonovalentEndPoint(),
        "MULTIPOINT((0 0), (5 1), (1 5))");
}







} // namespace tut
