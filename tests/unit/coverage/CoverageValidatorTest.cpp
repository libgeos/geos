//
// Test Suite for geos::coverage::CoverageValidatorTest class.

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/coverage/CoverageValidator.h>
#include <geos/geom/util/PolygonExtracter.h>

using geos::geom::util::PolygonExtracter;
using geos::coverage::CoverageValidator;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_coveragevalidator_data {

    WKTReader r;
    WKTWriter w;

    void
    printResult(const Geometry& actual, const Geometry& expected)
    {
        std::cout << std::endl;
        std::cout << "--actual--" << std::endl;
        std::cout << w.write(actual) << std::endl;
        std::cout << "--expect--" << std::endl;
        std::cout << w.write(expected) << std::endl;
    }

    void
    checkInvalid(const std::vector<std::string> &wkt_geoms,
                 const std::vector<std::string> &wkt_expected)
    {
        std::vector<std::unique_ptr<Geometry>> geoms = readList(wkt_geoms);
        std::vector<const Geometry*> coverage = toCoverage(geoms);
        std::vector<std::unique_ptr<Geometry>> expectedList = readList(wkt_expected);

        std::vector<std::unique_ptr<Geometry>> resultList = CoverageValidator::validate(coverage);

        ensure_equals("Coverage and Result sizes are not equal", geoms.size(), resultList.size());
        ensure_equals("Result and expected list sizes are not equal", resultList.size(), expectedList.size());

        for (std::size_t i = 0; i < resultList.size(); i++) {
            const Geometry* actual   = resultList[i].get();
            const Geometry* expected = expectedList[i].get();
            if (actual == nullptr && expected == nullptr) {
                // ok
            }
            else if (actual != nullptr && expected != nullptr) {
                ensure_equals_geometry(expected, actual);
            }
            else {
                std::cout << actual->toString() << std::endl;
                std::cout << expected->toString() << std::endl;

                // one is null, other is not -> FAIL
                std::ostringstream msg;
                msg << "Expected does not match actual at index " << i;
                fail( msg.str() );
            }
        }
    }

    void
    checkInvalidWithGaps(const std::vector<std::string> &wkt_geoms,
                         double gapWidth,
                         const std::vector<std::string> &wkt_expected)
    {
        std::vector<std::unique_ptr<Geometry>> geoms = readList(wkt_geoms);
        std::vector<const Geometry*> coverage = toCoverage(geoms);
        std::vector<std::unique_ptr<Geometry>> expectedList = readList(wkt_expected);

        std::vector<std::unique_ptr<Geometry>> resultList = CoverageValidator::validate(coverage, gapWidth);

        ensure_equals("Coverage and Result sizes are not equal", geoms.size(), resultList.size());
        ensure_equals("Result and expected list sizes are not equal", resultList.size(), expectedList.size());

        for (std::size_t i = 0; i < resultList.size(); i++) {
            const Geometry* actual   = resultList[i].get();
            const Geometry* expected = expectedList[i].get();
            if (actual == nullptr && expected == nullptr) {
                // ok
            }
            else if (actual != nullptr && expected != nullptr) {
                ensure_equals_geometry(expected, actual);
            }
            else {
                std::cout << actual->toString() << std::endl;
                std::cout << expected->toString() << std::endl;

                // one is null, other is not -> FAIL
                std::ostringstream msg;
                msg << "Expected does not match actual at index " << i;
                fail( msg.str() );
            }
        }
    }

    void
    checkValid(const std::vector<std::string> &wkt_geoms)
    {
        std::vector<std::unique_ptr<Geometry>> geoms;
        for (const auto& wkt : wkt_geoms) {
            if (wkt.size() <= 0) {
                geoms.push_back(r.read(wkt));
            }
            else {
                geoms.push_back(r.read(wkt));
            }
        }
        std::vector<const Geometry*> coverage;
        for (const auto& geom : geoms) {
            coverage.push_back(geom.get());
        }

        bool isValid = CoverageValidator::isValid(coverage);
        ensure( isValid );
    }

    std::vector<std::unique_ptr<Geometry>>
    readList(const std::vector<std::string> &wkt_geoms)
    {
        std::vector<std::unique_ptr<Geometry>> geoms;
        for (const auto& wkt : wkt_geoms) {
            if (wkt.size() <= 0) {
                geoms.push_back(nullptr);
            }
            else {
                geoms.push_back(r.read(wkt));
            }
        }
        return geoms;
    }

    std::vector<const Geometry*>
    toCoverage(std::vector<std::unique_ptr<Geometry>> &geoms)
    {
        std::vector<const Geometry*> coverage;
        for (const auto& geom : geoms) {
            coverage.push_back( geom == nullptr ? nullptr : geom.get());
        }
        return coverage;
    }

};


typedef test_group<test_coveragevalidator_data> group;
typedef group::object object;

group test_coveragevalidator_data("geos::coverage::CoverageValidator");


//========  Invalid cases   =============================

// testCollinearUnmatchedEdge
template<>
template<>
void object::test<1> ()
{
    std::vector<std::string> coverage{
        "POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))",
        "POLYGON ((100 300, 180 300, 180 200, 100 200, 100 300))"
    };
    std::vector<std::string> expected{
        "LINESTRING (100 200, 200 200)",
        "LINESTRING (180 300, 180 200, 100 200)"
    };
    checkInvalid(coverage, expected);
}

// testOverlappingSquares
template<>
template<>
void object::test<2> ()
{
    std::vector<std::string> coverage{
        "POLYGON ((1 9, 6 9, 6 4, 1 4, 1 9))",
        "POLYGON ((9 1, 4 1, 4 6, 9 6, 9 1))"
    };
    std::vector<std::string> expected{
        "LINESTRING (6 9, 6 4, 1 4)",
        "LINESTRING (4 1, 4 6, 9 6)"
    };
    checkInvalid(coverage, expected);
}


// testFullyCoveredTriangles
template<>
template<>
void object::test<20> ()
{
    std::vector<std::string> coverage{
        "POLYGON ((1 9, 9 1, 1 1, 1 9))",
        "POLYGON ((9 9, 1 9, 9 1, 9 9))",
        "POLYGON ((9 9, 9 1, 1 1, 1 9, 9 9))"
    };
    std::vector<std::string> expected{
        "LINESTRING (9 1, 1 1, 1 9)",
        "LINESTRING (9 1, 9 9, 1 9)",
        "LINESTRING (9 9, 9 1, 1 1, 1 9, 9 9)"
    };
    checkInvalid(coverage, expected);
}


//========  Gap cases   =============================

// testGap
template<>
template<>
void object::test<3> ()
{
    std::vector<std::string> coverage{
        "POLYGON ((1 5, 9 5, 9 1, 1 1, 1 5))",
        "POLYGON ((1 9, 5 9, 5 5.1, 1 5, 1 9))",
        "POLYGON ((5 9, 9 9, 9 5, 5.5 5.1, 5 9))"
    };
    std::vector<std::string> expected{
        "LINESTRING (1 5, 9 5)",
        "LINESTRING (1 5, 5 5.1, 5 9)",
        "LINESTRING (5 9, 5.5 5.1, 9 5)"
    };
    checkInvalidWithGaps(coverage, 0.5, expected);
}

// testGapDisjoint
template<>
template<>
void object::test<4> ()
{
    std::vector<std::string> coverage{
        "POLYGON ((1 5, 9 5, 9 1, 1 1, 1 5))",
        "POLYGON ((1 9, 5 9, 5 5.1, 1 5.1, 1 9))",
        "POLYGON ((5 9, 9 9, 9 5.1, 5 5.1, 5 9))"
    };
    std::vector<std::string> expected{
        "LINESTRING (1 5, 9 5)",
        "LINESTRING (5 5.1, 1 5.1)",
        "LINESTRING (9 5.1, 5 5.1)"
    };
    checkInvalidWithGaps(coverage, 0.5, expected);
}

// testGore
template<>
template<>
void object::test<5> ()
{
    std::vector<std::string> coverage{
        "POLYGON ((1 5, 5 5, 9 5, 9 1, 1 1, 1 5))",
        "POLYGON ((1 9, 5 9, 5 5, 1 5.1, 1 9))",
        "POLYGON ((5 9, 9 9, 9 5, 5 5, 5 9))"
    };
    std::vector<std::string> expected{
        "LINESTRING (1 5, 5 5)",
        "LINESTRING (1 5.1, 5 5)",
        ""
     };
    checkInvalidWithGaps(coverage, 0.5, expected);
}

//========  Valid cases   =============================

// testGrid
template<>
template<>
void object::test<10> ()
{
    std::vector<std::string> coverage{
        "POLYGON ((1 9, 5 9, 5 5, 1 5, 1 9))",
        "POLYGON ((9 9, 9 5, 5 5, 5 9, 9 9))",
        "POLYGON ((1 1, 1 5, 5 5, 5 1, 1 1))",
        "POLYGON ((9 1, 5 1, 5 5, 9 5, 9 1))"
    };
    checkValid(coverage);
}

// testMultiPolygon
template<>
template<>
void object::test<11> ()
{
    std::vector<std::string> coverage{
        "MULTIPOLYGON (((1 9, 5 9, 5 5, 1 5, 1 9)), ((9 1, 5 1, 5 5, 9 5, 9 1)))",
        "MULTIPOLYGON (((1 1, 1 5, 5 5, 5 1, 1 1)), ((9 9, 9 5, 5 5, 5 9, 9 9)))"
    };
    checkValid(coverage);
}

// testValidDuplicatePoints
template<>
template<>
void object::test<12> ()
{
    std::vector<std::string> coverage{
        "POLYGON ((1 9, 5 9, 5 5, 1 5, 1 5, 1 5, 1 9))",
        "POLYGON ((9 9, 9 5, 5 5, 5 9, 9 9))",
        "POLYGON ((1 1, 1 5, 5 5, 5 1, 1 1))",
        "POLYGON ((9 1, 5 1, 5 5, 9 5, 9 1))"
    };
    checkValid(coverage);
}

// testRingCollapse
template<>
template<>
void object::test<13> ()
{
    std::vector<std::string> coverage{
        "POLYGON ((1 9, 5 9, 1 9))",
        "POLYGON ((9 9, 9 5, 5 5, 5 9, 9 9))",
        "POLYGON ((1 1, 1 5, 5 5, 5 1, 1 1))",
        "POLYGON ((9 1, 5 1, 5 5, 9 5, 9 1))"
    };
    checkValid(coverage);
}

  //========  Valid cases with EMPTY  =============================

// testPolygonEmpty
template<>
template<>
void object::test<14> ()
{
    std::vector<std::string> coverage{
        "POLYGON ((1 9, 5 9, 5 5, 1 5, 1 9))",
        "POLYGON ((9 9, 9 5, 5 5, 5 9, 9 9))",
        "POLYGON ((1 1, 1 5, 5 5, 5 1, 1 1))",
        "POLYGON EMPTY"
    };
    checkValid(coverage);
}

// testMultiPolygonWithEmptyRing
template<>
template<>
void object::test<15> ()
{
    std::vector<std::string> coverage{
        "MULTIPOLYGON (((9 9, 9 1, 1 1, 2 4, 7 7, 9 9)), EMPTY)"
    };
    checkValid(coverage);
}




} // namespace tut
