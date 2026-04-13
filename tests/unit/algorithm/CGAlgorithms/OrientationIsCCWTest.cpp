//
// Test Suite for Orientation::isCCW() function
// Ported from JTS junit/algorithm/IsCCWTest.java

// tut
#include <tut/tut.hpp>
// geos
#include <geos/algorithm/Orientation.h>
#include <geos/geom/Polygon.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/Coordinate.h>
#include <geos/io/WKTReader.h>
#include <geos/io/WKBReader.h>
// std
#include <string>
#include <memory>
#include <sstream>

using namespace geos::algorithm;

namespace tut {
//
// Test Group
//

struct test_isccw_data {
    typedef std::unique_ptr<geos::geom::Geometry> GeometryPtr;

    geos::io::WKTReader reader_;
    geos::io::WKBReader breader_;

    test_isccw_data()
    {
    }

    ~test_isccw_data()
    {
    }

    void
    checkCCW(bool expectedCCW, const std::string& wkt)
    {
        auto poly = reader_.read<geos::geom::Polygon>(wkt);
        ensure("WKT must be POLYGON)", poly != nullptr);
        const geos::geom::CoordinateSequence* cs = poly->getExteriorRing()->getCoordinatesRO();
        bool actualCCW = Orientation::isCCW(cs);
        ensure_equals("CoordinateSequence isCCW", actualCCW, expectedCCW);
    }

    void
    checkCCWArea(bool expectedCCWArea, const std::string& wkt)
    {
        auto poly = reader_.read<geos::geom::Polygon>(wkt);
        ensure("WKT must be POLYGON)", poly != nullptr);
        const geos::geom::CoordinateSequence* cs = poly->getExteriorRing()->getCoordinatesRO();
        bool actualCCWArea = Orientation::isCCWArea(cs);
        ensure_equals("CoordinateSequence isCCWArea", actualCCWArea, expectedCCWArea);
    }

    void
    checkHexOrientationCCW(bool expectedCCW, std::istringstream& wkt)
    {
        GeometryPtr geom(breader_.readHEX(wkt));
        auto cs = geom->getCoordinates();
        bool actualCCW = Orientation::isCCW(cs.get());
        ensure_equals("CoordinateSequence isCCW", actualCCW, expectedCCW);
    }

};

typedef test_group<test_isccw_data> group;
typedef group::object object;

group test_isccw_group("geos::algorithm::CGAlgorithms::OrientationIsCCW");

//
// Test Cases
//

// 1 - Test if coordinates of polygon are counter-clockwise oriented
template<>
template<>
void object::test<1>
()
{
    const std::string wkt("POLYGON ((60 180, 140 240, 140 240, 140 240, 200 180, 120 120, 60 180))");
    checkCCW(false, wkt);
}

// 2 - Test if coordinates of polygon are counter-clockwise oriented
template<>
template<>
void object::test<2>
()
{
    const std::string wkt("POLYGON ((60 180, 140 120, 100 180, 140 240, 60 180))");
    checkCCW(true, wkt);
}

// 3 - Test the same polygon as in test No 2 but with duplicated top point
template<>
template<>
void object::test<3>
()
{
    const std::string wkt("POLYGON ((60 180, 140 120, 100 180, 140 240, 140 240, 60 180))");
    checkCCW(true, wkt);
}

// 4 - Test orientation the narrow (almost collapsed) ring
//     resulting in GEOS during execution of the union described
//     in http://trac.osgeo.org/geos/ticket/398
template<>
template<>
void object::test<4>
()
{
    std::istringstream
    wkt("0102000000040000000000000000000000841D588465963540F56BFB214F0341408F26B714B2971B40F66BFB214F0341408C26B714B2971B400000000000000000841D588465963540");
    checkHexOrientationCCW(true, wkt);
}

// 5 - Test orientation the narrow (almost collapsed) ring
//     resulting in JTS during execution of the union described
//     in http://trac.osgeo.org/geos/ticket/398
template<>
template<>
void object::test<5>
()
{
    std::istringstream
    wkt("0102000000040000000000000000000000841D588465963540F56BFB214F0341408F26B714B2971B40F66BFB214F0341408E26B714B2971B400000000000000000841D588465963540");
    checkHexOrientationCCW(true, wkt);
}

// testCCWSmall
template<>
template<>
void object::test<6>
()
{
    const std::string wkt("POLYGON ((1 1, 9 1, 5 9, 1 1))");
    checkCCW(true, wkt);
}

// testFlatTopSegment
template<>
template<>
void object::test<7>
()
{
    const std::string wkt("POLYGON ((100 200, 200 200, 200 100, 100 100, 100 200))");
    checkCCW(false, wkt);
}

// testFlatMultipleTopSegment
template<>
template<>
void object::test<8>
()
{
    const std::string wkt("POLYGON ((100 200, 127 200, 151 200, 173 200, 200 200, 100 100, 100 200))");
    checkCCW(false, wkt);
}

// testDegenerateRingHorizontal
template<>
template<>
void object::test<9>
()
{
    const std::string wkt("POLYGON ((100 200, 100 200, 200 200, 100 200))");
    checkCCW(false, wkt);
}

// testDegenerateRingAngled
template<>
template<>
void object::test<10>
()
{
    const std::string wkt("POLYGON ((100 100, 100 100, 200 200, 100 100))");
    checkCCW(false, wkt);
}

// testDegenerateRingVertical
template<>
template<>
void object::test<11>
()
{
    const std::string wkt("POLYGON ((200 100, 200 100, 200 200, 200 100))");
    checkCCW(false, wkt);
}

/**
* This case is an invalid ring, so answer is a default value
*/
// testTopAngledSegmentCollapse
template<>
template<>
void object::test<12>
()
{
    const std::string wkt("POLYGON ((10 20, 61 20, 20 30, 50 60, 10 20))");
    checkCCW(false, wkt);
}

// testABATopFlatSegmentCollapse
template<>
template<>
void object::test<13>
()
{
    const std::string wkt("POLYGON ((71 0, 40 40, 70 40, 40 40, 20 0, 71 0))");
    checkCCW(true, wkt);
}

// testABATopFlatSegmentCollapseMiddleStart
template<>
template<>
void object::test<14>
()
{
    const std::string wkt("POLYGON ((90 90, 50 90, 10 10, 90 10, 50 90, 90 90))");
    checkCCW(true, wkt);
}

// testMultipleTopFlatSegmentCollapseSinglePoint
template<>
template<>
void object::test<15>
()
{
    const std::string wkt("POLYGON ((100 100, 200 100, 150 200, 170 200, 200 200, 100 200, 150 200, 100 100))");
    checkCCW(true, wkt);
}

// testMultipleTopFlatSegmentCollapseFlatTop
template<>
template<>
void object::test<16>
()
{
    const std::string wkt("POLYGON ((10 10, 90 10, 70 70, 90 70, 10 70, 30 70, 50 70, 10 10))");
    checkCCW(true, wkt);
}


// See https://trac.osgeo.org/postgis/ticket/6065
template<>
template<>
void object::test<17>
()
{
    const std::string wkt("POLYGON((17.42207545158684 69.11091383590066,17.422075702665087 69.11091383235977,17.579930758184094 69.12294910230447,17.622976580401446 69.12848944101118,17.579930758184094 69.12294910230447,17.422075702665087 69.11091383235977,17.42207570266477 69.11091383235974,17.42207545158684 69.11091383590066,17.42207545158653 69.11091383590062,17.42207545158684 69.11091383590066))");
    checkCCW(false, wkt);

    std::istringstream wkb("0103000000010000000A000000890205230D6C31408177583619475140FD613B270D6C314031AA5436194751405D58985776943140195EE965DE47514085FFA6647B9F31408FC1C62B394851405D58985776943140195EE965DE475140FD613B270D6C314031AA543619475140A4613B270D6C31402FAA543619475140890205230D6C31408177583619475140320205230D6C31407E77583619475140890205230D6C31408177583619475140");
    checkHexOrientationCCW(false, wkb);
}


} // namespace tut

