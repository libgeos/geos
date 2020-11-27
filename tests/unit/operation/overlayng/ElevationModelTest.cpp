//
// Test Suite for geos::operation::overlayng::ElevationModel class.
//
// Last port:
// modules/core/src/test/java/org/locationtech/jts/operation/overlayng/ElevationModelTest.java
// 4c88fea526567b752ddb9a42aa16cfad2ee74ef1

#include <tut/tut.hpp>
#include <utility.h>

// geos
#include <geos/operation/overlayng/ElevationModel.h>
#include <geos/constants.h> // for DoubleNotANumber

// std
#include <memory>
#include <initializer_list>

using namespace geos::geom;
using namespace geos::operation::overlayng;
using geos::io::WKTReader;
using geos::DoubleNotANumber;
using geos::io::WKTWriter;

namespace tut {
//
// Test Group
//

// Common data used by all tests
struct test_overlayng_elevationmodel_data {

    WKTReader r;
    WKTWriter w;

    const double TOLERANCE = 0.00001;

    test_overlayng_elevationmodel_data()
    {
        w.setTrim(true);
        w.setOutputDimension(3);
    }

    void checkElevation(const std::string& wkt1, const std::string& wkt2, std::initializer_list<double> ords)
    {
        std::unique_ptr<Geometry> g1 = r.read(wkt1);
        std::unique_ptr<Geometry> g2 = r.read(wkt2);
        std::unique_ptr<ElevationModel> model = ElevationModel::create(*g1, *g2);
        // unsigned int numPts = ords.size() / 3;
        // assert ( 3 * numPts == ords.size());
        for ( std::initializer_list<double>::iterator i=ords.begin(), e=ords.end();
              i != e; ++i )
        {
          double x = *i++;
          double y = *i++;
          double expectedZ = *i;
          double actualZ = model->getZ(x, y);
          ensure_distance(expectedZ, actualZ, TOLERANCE);
        }
    }

    void checkElevation(const std::string& wkt1, std::initializer_list<double> ords)
    {
        checkElevation(wkt1, "POINT EMPTY", ords);
    }

    void checkElevationPopulateZ(const std::string& wkt,
                                 const std::string& wktNoZ,
                                 const std::string& wktZExpected)
    {
        std::unique_ptr<Geometry> geom = r.read(wkt);
        std::unique_ptr<ElevationModel> model = ElevationModel::create(*geom);

        std::unique_ptr<Geometry> geomNoZ = r.read(wktNoZ);
        model->populateZ(*geomNoZ);

        std::unique_ptr<Geometry> geomZExpected = r.read(wktZExpected);

        geomNoZ->normalize();
        geomZExpected->normalize();
        std::string obtainedWKT = w.write(geomNoZ.get());
        std::string expectedWKT = w.write(geomZExpected.get());
        ensure_equals(obtainedWKT, expectedWKT);
    }


};

typedef test_group<test_overlayng_elevationmodel_data> group;
typedef group::object object;

group test_overlayng_elevationmodel_group("geos::operation::overlayng::ElevationModel");

//
// Test Cases
//

// testBox
template<>
template<>
void object::test<1> ()
{
    checkElevation(
        "POLYGON Z ((1 6 50, 9 6 60, 9 4 50, 1 4 40, 1 6 50))",
        {
        0,10, 50,     5,10,  50,    10,10, 60,
        0,5,  50,     5,5, 50,      10,5, 50,
        0,4,  40,     5,4, 50,      10,4, 50,
        0,0,  40,     5,0, 50,      10,0, 50
        });
}


// testLine
template<>
template<>
void object::test<2> ()
{
    checkElevation(
        "LINESTRING Z (0 0 0, 10 10 10)",
        {
     -1,11, 5,                            11,11,  10,
        0,10, 5,    5,10,  5,   10,10,  10,
        0,5, 5,     5,5, 5,     10,5,   5,
        0,0, 0,     5,0, 5,     10,0,   5,
     -1,-1, 0,      5,-1,  5,   11,-1,  5
        });
}


// testMultiLine
template<>
template<>
void object::test<3> ()
{
    checkElevation(
        "MULTILINESTRING Z ((0 0 0, 10 10 8), (1 2 2, 9 8 6))",
        {
     -1,11, 4,                            11,11,  7,
        0,10, 4,    5,10, 4,    10,10,  7,
        0,5, 4,     5,5,  4,    10,5,   4,
        0,0, 1,     5,0,  4,    10,0,   4,
     -1,-1, 1,      5,-1, 4,    11,-1,  4
        });
  }

// testTwoLines
template<>
template<>
void object::test<4> ()
{
    checkElevation( "LINESTRING Z (0 0 0, 10 10 8)",
                    "LINESTRING Z (1 2 2, 9 8 6))",
    {
     -1,11, 4,                            11,11,  7,
        0,10, 4,    5,10, 4,    10,10,  7,
        0,5, 4,     5,5,  4,    10,5,   4,
        0,0, 1,     5,0,  4,    10,0,   4,
     -1,-1, 1,      5,-1, 4,    11,-1,  4
    });
}

/**
 * Tests that XY geometries are scanned correctly (avoiding reading Z)
 * and that they produce a model Z value of NaN
 */
// testLine2D()
template<>
template<>
void object::test<5> ()
{
    checkElevation( "LINESTRING(0 0, 10 0)",
                    { 5, 5, DoubleNotANumber }
        );
}

// testLineHorizontal
template<>
template<>
void object::test<6> ()
{
    checkElevation("LINESTRING Z (0 5 0, 10 5 10)",
        {
        0,10, 0,    5,10,  5,     10,10,  10,
        0,5,  0,    5,5,   5,     10,5,   10,
        0,0,  0,    5,0,   5,     10,0,   10
        });
}

// testLineVertical
template<>
template<>
void object::test<7> ()
{
    checkElevation("LINESTRING Z (5 0 0, 5 10 10)",
        {
        0,10, 10,    5,10, 10,    10,10, 10,
        0,5,  5,     5,5,  5,     10,5,  5,
        0,0,  0,     5,0,  0,     10,0,  0
        });
}

// tests that single point Z is used for entire grid and beyond
// testPoint()
template<>
template<>
void object::test<8> ()
{
    checkElevation("POINT Z (5 5 5)",
        {
        0,9, 5,     5,9,  5,    9,9, 5,
        0,5, 5,     5,5,  5,    9,5, 5,
        0,0, 5,     5,0,  5,    9,0, 5
        });
}

// tests that Z is average of input points with same location
// testMultiPointSame
template<>
template<>
void object::test<9> ()
{
    checkElevation("MULTIPOINT Z ((5 5 5), (5 5 9))",
        {
        0,9, 7,     5,9,  7,     9,9, 7,
        0,5, 7,     5,5,  7,     9,5, 7,
        0,0, 7,     5,0,  7,     9,0, 7
        });
}

// testPopulateZLine
template<>
template<>
void object::test<10> ()
{
  checkElevationPopulateZ("LINESTRING Z (0 0 0, 10 10 10)",
      "LINESTRING (1 1, 9 9)",
      "LINESTRING (1 1 0, 9 9 10)"
      );
}


// testPopulateZBox
template<>
template<>
void object::test<11> ()
{
  checkElevationPopulateZ("LINESTRING Z (0 0 0, 10 10 10)",
      "POLYGON ((1 9, 9 9, 9 1, 1 1, 1 9))",
      "POLYGON Z ((1 1 0, 1 9 5, 9 9 10, 9 1 5, 1 1 0))"
      );
}

} // namespace tut
