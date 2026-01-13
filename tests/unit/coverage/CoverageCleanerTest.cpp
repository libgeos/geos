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
            ensure_equals("hasZ does not match", actual[i]->hasZ(), expected[i]->hasZ());
            ensure_equals("hasM does not match", actual[i]->hasM(), expected[i]->hasM());
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
    checkCleanSnapInt(
        std::vector<const Geometry*> cov,
        double snapDist,
        std::vector<const Geometry*> expected)
    {
        std::vector<std::unique_ptr<Geometry>> actualPtr = CoverageCleaner::clean(cov, snapDist, 0);
        std::vector<const Geometry*> actual = toArray(actualPtr);
        checkValidCoverage(actual, snapDist);
        checkEqual(expected, actual);
    }

    void
    checkCleanSnapInt(
        std::vector<const Geometry*> cov,
        double snapDist)
    {
        std::vector<std::unique_ptr<Geometry>> covClean = CoverageCleaner::clean(cov, snapDist, 0);
        checkValidCoverage(toArray(covClean), snapDist);
    }

    void
    checkCleanSnap(const std::vector<std::string>& covStrs, double snapDist)
    {
        std::vector<std::unique_ptr<Geometry>> cov = readArray(covStrs);
        std::vector<const Geometry*> covArr = toArray(cov);
        checkCleanSnapInt(covArr, snapDist);
    }

    void
    checkCleanSnap(
        const std::vector<std::string>& covStrs,
        double snapDist,
        const std::vector<std::string>& expStrs)
    {
        std::vector<std::unique_ptr<Geometry>> cov = readArray(covStrs);
        std::vector<const Geometry*> covArr = toArray(cov);
        std::vector<std::unique_ptr<Geometry>> exp = readArray(expStrs);
        std::vector<const Geometry*> expArr = toArray(exp);
        checkCleanSnapInt(covArr, snapDist, expArr);
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
    readArray(const std::vector<std::string>& wkts)
    {
        std::vector<std::unique_ptr<Geometry>> geometries;
        for (const std::string& wkt : wkts) {
            auto geom = r.read(wkt);
            if (geom != nullptr) {
                geometries.push_back(std::move(geom));
            }
        }
        return geometries;
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


// testSingleNearMatch
template<>
template<>
void object::test<2>()
{
    checkCleanSnap(
        {
            "POLYGON ((1 9, 9 9, 9 4.99, 1 5, 1 9))",
            "POLYGON ((1 1, 1 5, 9 5, 9 1, 1 1))"
        },
        0.1);
}

// testManyNearMatches
template<>
template<>
void object::test<3>()
{
    checkCleanSnap(
        {
            "POLYGON ((1 9, 9 9, 9 5, 8 5, 7 5, 4 5.5, 3 5, 2 5, 1 5, 1 9))",
            "POLYGON ((1 1, 1 4.99, 2 5.01, 3.01 4.989, 5 3, 6.99 4.99, 7.98 4.98, 9 5, 9 1, 1 1))"
        },
        0.1);
}

// testPolygonSnappedPreserved
// Tests that if interior point lies in a spike that is snapped away, polygon is still in result
template<>
template<>
void object::test<4>()
{
    checkCleanSnap(
        {"POLYGON ((90 0, 10 0, 89.99 30, 90 100, 90 0))"},
        0.1,
        {"POLYGON ((90 0, 10 0, 89.99 30, 90 0))"}
        );
}

// testPolygonsSnappedPreserved
// Tests that if interior point lies in a spike that is snapped away, polygon is still in result
template<>
template<>
void object::test<5>()
{
    checkCleanSnap(
        {
            "POLYGON ((0 0, 0 2, 5 2, 5 8, 5.01 0, 0 0))",
            "POLYGON ((0 8, 5 8, 5 2, 0 2, 0 8))"
        },
        0.02,
        {
            "POLYGON ((0 0, 0 2, 5 2, 5.01 0, 0 0))",
            "POLYGON ((0 8, 5 8, 5 2, 0 2, 0 8))"
        });
}

// testMergeGapToLongestBorder
template<>
template<>
void object::test<6>()
{
    checkCleanGapWidth(
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 9 9, 9 5, 1 5, 1 9)), POLYGON ((5 1, 5 5, 1 5, 5 1)), POLYGON ((5 1, 5.1 5, 9 5, 5 1)))",
        1,
        "GEOMETRYCOLLECTION (POLYGON ((5.1 5, 5 5, 1 5, 1 9, 9 9, 9 5, 5.1 5)), POLYGON ((5 1, 1 5, 5 5, 5 1)), POLYGON ((5 1, 5 5, 5.1 5, 9 5, 5 1)))"
        );
}

std::string covWithGaps = "GEOMETRYCOLLECTION (POLYGON ((1 3, 9 3, 9 1, 1 1, 1 3)), POLYGON ((1 3, 1 9, 4 9, 4 3, 3 4, 1 3)), POLYGON ((4 9, 7 9, 7 3, 6 5, 5 5, 4 3, 4 9)), POLYGON ((7 9, 9 9, 9 3, 8 3.1, 7 3, 7 9)))";

// testMergeGapWidth_0
template<>
template<>
void object::test<7>()
{
    checkCleanGapWidth(covWithGaps,
        0,
        "GEOMETRYCOLLECTION (POLYGON ((9 3, 9 1, 1 1, 1 3, 4 3, 7 3, 9 3)), POLYGON ((1 9, 4 9, 4 3, 3 4, 1 3, 1 9)), POLYGON ((6 5, 5 5, 4 3, 4 9, 7 9, 7 3, 6 5)), POLYGON ((7 9, 9 9, 9 3, 8 3.1, 7 3, 7 9)))"
        );
}

// testMergeGapWidth_1
template<>
template<>
void object::test<8>()
{
    checkCleanGapWidth(covWithGaps,
        1,
        "GEOMETRYCOLLECTION (POLYGON ((7 3, 9 3, 9 1, 1 1, 1 3, 4 3, 7 3)), POLYGON ((1 9, 4 9, 4 3, 1 3, 1 9)), POLYGON ((7 3, 6 5, 5 5, 4 3, 4 9, 7 9, 7 3)), POLYGON ((7 9, 9 9, 9 3, 7 3, 7 9)))"
        );
}

// testMergeGapWidth_2
template<>
template<>
void object::test<9>()
{
    checkCleanGapWidth(covWithGaps,
        2,
        "GEOMETRYCOLLECTION (POLYGON ((9 3, 9 1, 1 1, 1 3, 4 3, 7 3, 9 3)), POLYGON ((1 9, 4 9, 4 3, 1 3, 1 9)), POLYGON ((7 3, 4 3, 4 9, 7 9, 7 3)), POLYGON ((9 9, 9 3, 7 3, 7 9, 9 9)))"
        );
}

std::string covWithOverlap = "GEOMETRYCOLLECTION (POLYGON ((1 3, 5 3, 4 1, 1 1, 1 3)), POLYGON ((1 3, 1 9, 4 9, 4 3, 3 1.9, 1 3)))";

// testMergeOverlapMinArea
template<>
template<>
void object::test<10>()
{
    checkCleanOverlapMerge(covWithOverlap,
        CoverageCleaner::MERGE_MIN_AREA,
        "GEOMETRYCOLLECTION (POLYGON ((5 3, 4 1, 1 1, 1 3, 4 3, 5 3)), POLYGON ((1 9, 4 9, 4 3, 1 3, 1 9)))"
        );
}

// testMergeOverlapMaxArea
template<>
template<>
void object::test<11>()
{
    checkCleanOverlapMerge(covWithOverlap,
        CoverageCleaner::MERGE_MAX_AREA,
        "GEOMETRYCOLLECTION (POLYGON ((1 1, 1 3, 3 1.9, 4 3, 5 3, 4 1, 1 1)), POLYGON ((1 3, 1 9, 4 9, 4 3, 3 1.9, 1 3)))"
        );
}

// testMergeOverlapMinId
template<>
template<>
void object::test<12>()
{
    checkCleanOverlapMerge(covWithOverlap,
        CoverageCleaner::MERGE_MIN_INDEX,
        "GEOMETRYCOLLECTION (POLYGON ((5 3, 4 1, 1 1, 1 3, 4 3, 5 3)), POLYGON ((1 9, 4 9, 4 3, 1 3, 1 9)))"
        );
}

// testMergeOverlap2
template<>
template<>
void object::test<13>()
{
    checkCleanSnap(
        {
            "POLYGON ((5 9, 9 9, 9 1, 5 1, 5 9))",
            "POLYGON ((1 5, 5 5, 5 2, 1 2, 1 5))",
            "POLYGON ((2 7, 5 7, 5 4, 2 4, 2 7))"
        },
        0.1,
        {
            "POLYGON ((5 1, 5 2, 5 4, 5 5, 5 7, 5 9, 9 9, 9 1, 5 1))",
            "POLYGON ((5 2, 1 2, 1 5, 2 5, 5 5, 5 4, 5 2))",
            "POLYGON ((5 5, 2 5, 2 7, 5 7, 5 5))"
        });
}

// testMergeOverlap
template<>
template<>
void object::test<14>()
{
    checkCleanOverlapMerge(
        "GEOMETRYCOLLECTION (POLYGON ((5 9, 9 9, 9 1, 5 1, 5 9)), POLYGON ((1 5, 5 5, 5 2, 1 2, 1 5)), POLYGON ((2 7, 5 7, 5 4, 2 4, 2 7)))",
        CoverageCleaner::MERGE_LONGEST_BORDER,
        "GEOMETRYCOLLECTION (POLYGON ((5 7, 5 9, 9 9, 9 1, 5 1, 5 2, 5 4, 5 5, 5 7)), POLYGON ((5 2, 1 2, 1 5, 2 5, 5 5, 5 4, 5 2)), POLYGON ((2 5, 2 7, 5 7, 5 5, 2 5)))"
        );
}

//-------------------------------------------

//-- a duplicate coverage element is assigned to the lowest result index
// testDuplicateItems
template<>
template<>
void object::test<15>()
{
    checkClean(
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 9 1, 1 1, 1 9)), POLYGON ((1 9, 9 1, 1 1, 1 9)))",
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 9 1, 1 1, 1 9)), POLYGON EMPTY)"
        );
}

// testCoveredItem
template<>
template<>
void object::test<16>()
{
    checkClean(
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 9 9, 9 4, 1 4, 1 9)), POLYGON ((2 5, 2 8, 8 8, 8 5, 2 5)))",
        "GEOMETRYCOLLECTION (POLYGON ((9 9, 9 4, 1 4, 1 9, 9 9)), POLYGON EMPTY)"
        );
}

// testCoveredItemMultiPolygon
template<>
template<>
void object::test<17>()
{
    checkClean(
        "GEOMETRYCOLLECTION (MULTIPOLYGON (((1 1, 1 5, 5 5, 5 1, 1 1)), ((6 5, 6 1, 9 1, 6 5))), POLYGON ((6 1, 6 5, 9 1, 6 1)))",
        "GEOMETRYCOLLECTION (MULTIPOLYGON (((1 5, 5 5, 5 1, 1 1, 1 5)), ((6 5, 9 1, 6 1, 6 5))), POLYGON EMPTY)"
        );
}

// Tests that a collapsed polygon due to snapping is returned as EMPTY
template<>
template<>
void object::test<18>()
{
    checkCleanSnap({
        "POLYGON ((1 1, 1 9, 6 5, 9 1, 1 1))",
        "POLYGON ((9 1, 6 5.1, 1 9, 9 9, 9 1))",
        "POLYGON ((9 1, 6 5, 1 9, 6 5.1, 9 1))"},
        1,
        {
        "POLYGON ((6 5, 9 1, 1 1, 1 9, 6 5))",
        "POLYGON ((9 9, 9 1, 6 5, 1 9, 9 9))",
        "POLYGON EMPTY"
        });
}

// testCoverageWithNonPolygon
template<>
template<>
void object::test<19> ()
{
    checkClean(
        "GEOMETRYCOLLECTION (LINESTRING EMPTY, POLYGON EMPTY, LINESTRING (2 1, 2 5, 8 5, 8 1, 2 1))",
        "GEOMETRYCOLLECTION (POLYGON EMPTY, POLYGON EMPTY, POLYGON EMPTY)");
}

// testCoverageWithNonPolygon
template<>
template<>
void object::test<20> ()
{
    checkClean(
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 9 9, 9 4, 1 4, 1 9)), LINESTRING EMPTY)",
        "GEOMETRYCOLLECTION (POLYGON ((1 9, 9 9, 9 4, 1 4, 1 9)), POLYGON EMPTY)");
}





} // namespace tut
