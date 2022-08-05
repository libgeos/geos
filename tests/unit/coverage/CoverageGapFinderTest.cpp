//
// Test Suite for geos::coverage::CoverageGapFinderTest class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/coverage/CoverageGapFinder.h>

using geos::coverage::CoverageGapFinder;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_coveragegapfinder_data {

    WKTReader r;
    WKTWriter w;

    void
    printResult(std::unique_ptr<Geometry>& actual, std::unique_ptr<Geometry>& expected)
    {
        std::cout << std::endl;
        std::cout << "--actual--" << std::endl;
        std::cout << w.write(actual.get()) << std::endl;
        std::cout << "--expect--" << std::endl;
        std::cout << w.write(expected.get()) << std::endl;
    }

    void
    checkGaps(const std::string& wktCoverage, double gapWidth, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> covGeom = r.read(wktCoverage);
        std::vector<const Geometry*> coverage = toArray(covGeom);
        std::unique_ptr<Geometry> actual = CoverageGapFinder::findGaps(coverage, gapWidth);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        // printResult(actual, expected);
        ensure_equals_geometry(actual.get(), expected.get());
    }

    std::vector<const Geometry*>
    toArray(const std::unique_ptr<Geometry>& geom)
    {
        std::vector<const Geometry*> geoms;
        for (std::size_t i = 0; i < geom->getNumGeometries(); i++) {
            geoms.push_back(geom->getGeometryN(i));
        }
        return geoms;
    }

};


typedef test_group<test_coveragegapfinder_data> group;
typedef group::object object;

group test_coveragegapfinder_data("geos::coverage::CoverageGapFinder");



// testThreePolygonGap
template<>
template<>
void object::test<1> ()
{
    checkGaps(
        "MULTIPOLYGON (((1 5, 1 9, 5 9, 5 6, 3 5, 1 5)), ((5 9, 9 9, 9 5, 7 5, 5 6, 5 9)), ((1 1, 1 5, 3 5, 7 5, 9 5, 9 1, 1 1)))",
        1,
        "LINESTRING (3 5, 7 5, 5 6, 3 5)"
        );
}






} // namespace tut
