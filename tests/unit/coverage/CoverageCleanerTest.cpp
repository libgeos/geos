//
// Test Suite for geos::coverage::CoverageGapFinderTest class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/coverage/CoverageCleaner.h>
#include <geos/coverage/CoverageValidator.h>

using geos::coverage::CoverageCleaner;
using geos::coverage::CoverageValidator;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_coveragecleaner_data {

    WKTReader r;
    WKTWriter w;

    void
    printResult(
        const std::unique_ptr<Geometry>& expected,
        const std::unique_ptr<Geometry>& actual)
    {
        std::cout << std::endl;
        std::cout << "--expect--" << std::endl;
        std::cout << w.write(expected.get()) << std::endl;
        std::cout << "--actual--" << std::endl;
        std::cout << w.write(actual.get()) << std::endl;
    }

    void
    printResult(
        const std::vector<std::unique_ptr<Geometry>>& expected,
        const std::vector<std::unique_ptr<Geometry>>& actual)
    {
        std::cout << std::endl;
        std::cout << "--expect--" << std::endl;
        for (auto& e : expected) {
            std::cout << w.write(e.get()) << std::endl;
        }
        std::cout << "--actual--" << std::endl;
        for (auto& a : actual) {
            std::cout << w.write(a.get()) << std::endl;
        }
        std::cout << std::endl;
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

    std::vector<const Geometry*>
    toArray(const std::vector<std::unique_ptr<Geometry>>& cov)
    {
        std::vector<const Geometry*> geoms;
        for (const auto& g : cov) {
            geoms.push_back(g.get());
        }
        return geoms;
    }

    void
    checkEqual(std::vector<const Geometry*>& expected, std::vector<const Geometry*>& actual)
    {
        ensure_equals("checkEqual sizes", actual.size(), expected.size());
        for (std::size_t i = 0; i < actual.size(); i++) {
            ensure_equals_geometry(actual[i], expected[i]);
        }
    }

    void
    checkEqual(
        std::vector<std::unique_ptr<Geometry>>& expected,
        std::vector<std::unique_ptr<Geometry>>& actual)
    {
        auto actualArr = toArray(actual);
        auto expectedArr = toArray(expected);
        checkEqual(expectedArr, actualArr);
    }

    void
    checkClean(const std::string& wkt, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        std::vector<const Geometry*> cov = toArray(geom);
        std::vector<std::unique_ptr<Geometry>> actual = CoverageCleaner::cleanGapWidth(cov, 0);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        auto expectedArr = toArray(expected);
        auto actualArr = toArray(actual);
        checkEqual(expectedArr, actualArr);
    }

    void
    checkCleanGapWidth(const std::string& wkt, double gapWidth, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        std::vector<const Geometry*> cov = toArray(geom);
        std::vector<std::unique_ptr<Geometry>> actual = CoverageCleaner::cleanGapWidth(cov, gapWidth);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        auto expectedArr = toArray(expected);
        auto actualArr = toArray(actual);
        checkEqual(expectedArr, actualArr);
    }

    void
    checkCleanOverlapMerge(const std::string& wkt, int mergeStrategy, const std::string& wktExpected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        std::vector<const Geometry*> cov = toArray(geom);
        std::vector<std::unique_ptr<Geometry>> actual = CoverageCleaner::cleanOverlapGap(cov, mergeStrategy, 0);
        std::unique_ptr<Geometry> expected = r.read(wktExpected);
        auto expectedArr = toArray(expected);
        auto actualArr = toArray(actual);
        checkEqual(expectedArr, actualArr);
    }

    void
    checkCleanSnap(std::vector<const Geometry*> cov, double snapDist)
    {
        std::vector<std::unique_ptr<Geometry>> covClean = CoverageCleaner::clean(cov, snapDist, 0);
        checkValidCoverage(toArray(covClean), snapDist);
    }

    void
    checkCleanSnap(std::vector<const Geometry*> cov, double snapDist, std::vector<const Geometry*> expected)
    {
        std::vector<std::unique_ptr<Geometry>> actualPtr = CoverageCleaner::clean(cov, snapDist, 0);
        std::vector<const Geometry*> actual = toArray(actualPtr);
        checkValidCoverage(actual, snapDist);
        checkEqual(expected, actual);
    }

    void
    checkCleanSnap(const std::string& wkt, double snapDist)
    {
        std::vector<std::unique_ptr<Geometry>> cov = readArray(wkt);
        auto covArr = toArray(cov);
        checkCleanSnap(covArr, snapDist);
    }

    void
    checkValidCoverage(std::vector<const Geometry*> coverage, double tolerance)
    {
        for (const Geometry* geom : coverage) {
            ensure("checkValidCoverage geom->isValid()", geom->isValid());
        }
        bool isValid = CoverageValidator::isValid(coverage, tolerance);
        ensure("checkValidCoverage CoverageValidator", isValid);
    }

    std::vector<std::unique_ptr<Geometry>>
    readArray(std::vector<std::string>& wkts)
    {
        std::vector<std::unique_ptr<Geometry>> geometries;
        for (std::string& wkt : wkts) {
            auto geom = r.read(wkt);
            if (geom != nullptr) {
                geometries.push_back(std::move(geom));
            }
        }
        return geometries;
    }

    std::vector<std::unique_ptr<Geometry>>
    readArray(const std::string& wkt)
    {
        auto geom = r.read(wkt);
        std::vector<std::unique_ptr<Geometry>> geoms;
        for (std::size_t i = 0; i < geom->getNumGeometries(); i++) {
            geoms.emplace_back(geom->getGeometryN(i)->clone().release());
        }
        return geoms;
    }

};



typedef test_group<test_coveragecleaner_data> group;
typedef group::object object;

group test_coveragecleaner_data("geos::coverage::CoverageCleaner");



// testCoverageWithEmpty
template<>
template<>
void object::test<1> ()
{
    checkClean(
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 9 9, 9 4, 1 4, 1 9)), POLYGON EMPTY, POLYGON ((2 1, 2 5, 8 5, 8 1, 2 1)))",
        "GEOMETRYCOLLECTION (POLYGON ((1 4, 1 9, 9 9, 9 4, 8 4, 2 4, 1 4)), POLYGON EMPTY, POLYGON ((8 1, 2 1, 2 4, 8 4, 8 1)))");
}






} // namespace tut
